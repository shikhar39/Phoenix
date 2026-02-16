#include "VulkanRenderer.hpp"

#include "VulkanDevice.hpp"
#include "Model.hpp"
#include "../Utils.hpp"

#include <cstdint>
#include <fstream>
#include <filesystem>

namespace PhoenixEngine {
namespace Vulkan {
Renderer::Renderer(Window& window, Device& device)
: mWindow{window}, mDevice{device} {
	recreateSwapChain();
	createCommandBuffers();
	createGraphicsPipeline();
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::drawFrame() {
	VkResult result = mSwapChain->acquireNextImage(&mCurrentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		spdlog::warn("trying to recreate swapchain");
		recreateSwapChain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swapchain image");
	}

	VkCommandBuffer currentCommandBuffer = getCurrentCommandBuffer();

	vkResetCommandBuffer(currentCommandBuffer, 0);
	recordCommandBuffer(currentCommandBuffer, mCurrentImageIndex);

	mSwapChain->submitCommandBuffer(&currentCommandBuffer, &mCurrentImageIndex);
}


void Renderer::recreateSwapChain() {
	VkExtent2D extent = mWindow.getExtent();
	while (extent.width == 0 ||extent.height == 0) {
		extent = mWindow.getExtent();
		glfwWaitEvents();
	}

	spdlog::warn("{}, {}", extent.width, extent.height);
	vkDeviceWaitIdle(mDevice.get());

	if (mSwapChain == nullptr) {
		mSwapChain = std::make_unique<SwapChain>(mDevice, extent);
	} else {
		std::shared_ptr<SwapChain> oldSwapChain = std::move(mSwapChain);
		mSwapChain = std::make_unique<SwapChain>(mDevice, extent, oldSwapChain);
	}
}

void Renderer::createCommandBuffers() {
	mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mDevice.getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

	if (vkAllocateCommandBuffers(mDevice.get(), &allocInfo, mCommandBuffers.data()) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
	spdlog::error("Command buffer allocated");
}

void Renderer::freeCommandBuffers() {
	vkFreeCommandBuffers(
		mDevice.get(),
		mDevice.getCommandPool(),
		static_cast<uint32_t>(mCommandBuffers.size()),
		mCommandBuffers.data());
  mCommandBuffers.clear();
}

void Renderer::recordCommandBuffer(VkCommandBuffer& commandBuffer,
								 uint32_t imageIndex) const {
	spdlog::info("recording command buffer");
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;				   // Optional
	beginInfo.pInheritanceInfo = nullptr;  // Optional
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	auto swapChainExtent = mSwapChain->getSwapChainExtent();

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mSwapChain->getRenderPass();
	renderPassInfo.framebuffer = mSwapChain->getFrameBuffer(imageIndex);
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = mSwapChain->getSwapChainExtent();
	VkClearValue clearColor = {{{0.5f, 0.5f, 0.5f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
						 VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					  mGraphicsPipeline);
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	spdlog::info("drawing model");
	mModel->bind(commandBuffer);
	mModel->draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void Renderer::loadModels() {
	spdlog::info("attempting model loading");
	std::vector<Model::Vertex> vertices {
		{{0.0, -0.5}, {1.0, 0.0, 0.0}},
		{{0.5, 0.5}, {0.0, 1.0, 0.0}},
		{{-0.5, 0.5}, {0.0, 0.0, 1.0}}
	};

	spdlog::info("successfully created vertices");

	mModel = std::make_unique<Model>(mDevice, vertices);
	spdlog::info("successfully created model");
}

void Renderer::createGraphicsPipeline() {
	VkExtent2D swapChainExtent = mSwapChain->getSwapChainExtent();
#pragma region READ_SHADER_FILES
	spdlog::info(Utils::getProjectRoot().string());
	std::filesystem::path dirPath("./"); // your directory

	if (std::filesystem::exists(dirPath) &&
	    std::filesystem::is_directory(dirPath)) {
	  std::cout << "Files in " << dirPath << ":\n";

	  for (const auto &entry : std::filesystem::directory_iterator(dirPath)) {
	    if (entry.is_regular_file()) { // skip subdirectories
	      std::cout << "  " << entry.path().filename() << "\n";
	    }
	  }
	} else {
	  std::cout << dirPath << " does not exist or is not a directory.\n";
	}
	auto vertShaderFile = readFile("shaders/out/shader.vert.spv");
	auto fragShaderFile = readFile("shaders/out/shader.frag.spv");

	spdlog::info("Reading vertex shader file: {} bytes", vertShaderFile.size());
	spdlog::info("Reading fragment shader file: {} bytes", fragShaderFile.size());

	VkShaderModule vertShaderModule = createShaderModule(vertShaderFile);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderFile);
#pragma endregion

#pragma region SHADER_STAGE_CREATION
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
													  fragShaderStageInfo};
	spdlog::info("Shader stages created");
#pragma endregion

#pragma region CREATE_DYNAMIC_STAGE
	std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
												 VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	// dynamicState.pNext;
	dynamicState.flags;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
#pragma endregion

#pragma region VERTEX_INPUT_INFO
	auto bindingDescriptions = Model::Vertex::getBindingDescriptions();
	auto attributeDescriptions = Model::Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();  // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();	 // Optional
#pragma endregion

#pragma region INPUT_ASSEMBLY
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
#pragma endregion

#pragma region VIEWPORT_AND_SCISSOR
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;
#pragma endregion

#pragma region VIEWPORT_STATE_CREATE_INFO
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
#pragma endregion
	spdlog::warn(
		"Some parameters here require GPU feature enabling. Look into that!!");

#pragma region RASTERIZER
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
#pragma endregion

#pragma region MULTISAMPLING
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#pragma endregion

#pragma region COLOR_BLEND_ATTACHMENT
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
#pragma endregion

#pragma region COLOR_BLENDING
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
#pragma endregion

#pragma region PIPELINE_LAYOUT
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;			// Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;	// Optional
#pragma endregion

	if (vkCreatePipelineLayout(mDevice.get(), &pipelineLayoutInfo, nullptr,
							   &mPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
	spdlog::info("Pipeline layout created");

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext;
	pipelineInfo.flags;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pTessellationState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mSwapChain->getRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(mDevice.get(), VK_NULL_HANDLE, 1, &pipelineInfo,
								  nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	spdlog::info("Graphics pipeline created");
	// ########### SHADER MODULE CLEANUP ###########

	vkDestroyShaderModule(mDevice.get(), vertShaderModule, nullptr);
	vkDestroyShaderModule(mDevice.get(), fragShaderModule, nullptr);
	spdlog::info("Shader modules cleaned up");
}

std::vector<char> Renderer::readFile(const std::filesystem::path& relative) {
	std::ifstream file(Utils::assetPath(relative), std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	return buffer;
}

VkShaderModule Renderer::createShaderModule(const std::vector<char>& code) const {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(mDevice.get(), &createInfo, nullptr, &shaderModule) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}
} // namespace Vulkan
} // namespace PhoenixEngine
