#include "VulkanRenderer.hpp"

#include "VulkanDevice.hpp"
#include "Model.hpp"
#include "../Utils.hpp"

#include <cstdint>
#include <fstream>
#include <filesystem>

#include "Pipeline.hpp"

namespace PhoenixEngine {
namespace Vulkan {
Renderer::Renderer(Window& window, Device& device)
: mWindow{window}, mDevice{device} {
	recreateSwapChain();
	createCommandBuffers();
	createPipelineLayout();
	createPipeline();
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
	mPipeline->bind(commandBuffer);
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
	
void Renderer::createPipeline() {
	PipelineConfigInfo pipelineConfigInfo{};

	Pipeline::defaultPipelineConfigInfo(pipelineConfigInfo);
	pipelineConfigInfo.mRenderPass = getSwapChainRenderPass();
	pipelineConfigInfo.mPipelineLayout = mPipelineLayout;

	mPipeline =  std::make_unique<Pipeline>(
		mDevice,
		"shaders/out/shader.vert.spv",
		"shaders/out/shader.frag.spv",
		pipelineConfigInfo
		);
}
	
void Renderer::createPipelineLayout() {
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;			// Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;	// Optional

	if (vkCreatePipelineLayout(mDevice.get(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}
} // namespace Vulkan
} // namespace PhoenixEngine
