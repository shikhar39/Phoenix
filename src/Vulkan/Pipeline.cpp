#include "Pipeline.hpp"

#include <fstream>

#include "Model.hpp"
#include "Utils.hpp"

namespace PhoenixEngine {
namespace Vulkan {
Pipeline::Pipeline(Device& device, const std::string& vertFilePath, const std::string& fragFilePath,
				   const PipelineConfigInfo& configInfo) : mDevice{device} {
	createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
}

Pipeline::~Pipeline() {
	vkDestroyShaderModule(mDevice.get(), mVertexShaderModule, nullptr);
	vkDestroyShaderModule(mDevice.get(), mFragmentShaderModule, nullptr);
	spdlog::info("Shader modules cleaned up");
	vkDestroyPipeline(mDevice.get(), mGraphicsPipeline, nullptr);
}

void Pipeline::bind(VkCommandBuffer commandBuffer) const {
vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				  mGraphicsPipeline);
}

	void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
	configInfo.mInputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.mInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.mInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.mViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.mViewportInfo.viewportCount = 1;
	configInfo.mViewportInfo.pViewports = nullptr;
	configInfo.mViewportInfo.scissorCount = 1;
	configInfo.mViewportInfo.pScissors = nullptr;
	
	configInfo.mRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.mRasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.mRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.mRasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.mRasterizationInfo.lineWidth = 1.0f;
	configInfo.mRasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	configInfo.mRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.mRasterizationInfo.depthBiasEnable = VK_FALSE;

	configInfo.mMultisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.mMultisamplingInfo.sampleShadingEnable = VK_FALSE;
	configInfo.mMultisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	configInfo.mColorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	configInfo.mColorBlendAttachment.blendEnable = VK_FALSE;

	configInfo.mColorBlendInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.mColorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.mColorBlendInfo.attachmentCount = 1;
	configInfo.mColorBlendInfo.pAttachments = &configInfo.mColorBlendAttachment;

	configInfo.mDynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
												 VK_DYNAMIC_STATE_SCISSOR};

	configInfo.mDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.mDynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.mDynamicStateEnables.size());
	configInfo.mDynamicStateInfo.pDynamicStates = configInfo.mDynamicStateEnables.data();

	configInfo.mVertexInputBindingDescriptions = Model::Vertex::getBindingDescriptions();
	configInfo.mVertexInputAttributeDescriptions = Model::Vertex::getAttributeDescriptions();
}

void Pipeline::createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo) {
#pragma region READ_SHADER_FILES
	auto vertShaderFile = readFile(vertFilePath);
	auto fragShaderFile = readFile(fragFilePath);

	spdlog::info("Reading vertex shader file: {} bytes", vertShaderFile.size());
	spdlog::info("Reading fragment shader file: {} bytes", fragShaderFile.size());

	createShaderModule(vertShaderFile, &mVertexShaderModule);
	createShaderModule(fragShaderFile,  &mFragmentShaderModule);
#pragma endregion

#pragma region SHADER_STAGE_CREATION
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = mVertexShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = mFragmentShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
													  fragShaderStageInfo};
	spdlog::info("Shader stages created");
#pragma endregion

#pragma region VERTEX_INPUT_INFO
	auto bindingDescriptions = Model::Vertex::getBindingDescriptions();
	auto attributeDescriptions = Model::Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(configInfo.mVertexInputBindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = configInfo.mVertexInputBindingDescriptions.data();  // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configInfo.mVertexInputAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = configInfo.mVertexInputAttributeDescriptions.data();	 // Optional
#pragma endregion

#pragma region INPUT_ASSEMBLY
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
#pragma endregion

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &configInfo.mViewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.mRasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.mMultisamplingInfo;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &configInfo.mColorBlendInfo;
	pipelineInfo.pDynamicState = &configInfo.mDynamicStateInfo;
	pipelineInfo.layout = configInfo.mPipelineLayout;
	pipelineInfo.renderPass = configInfo.mRenderPass;
	pipelineInfo.subpass = configInfo.mSubpass;
	
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(mDevice.get(), VK_NULL_HANDLE, 1, &pipelineInfo,
								  nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	spdlog::info("Graphics pipeline created");
	// ########### SHADER MODULE CLEANUP ###########

}

void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) const {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	if (vkCreateShaderModule(mDevice.get(), &createInfo, nullptr, shaderModule) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
		}
}

std::vector<char> Pipeline::readFile(const std::filesystem::path& relative) {
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
}
}
