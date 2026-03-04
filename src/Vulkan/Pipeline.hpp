#pragma once

#include "SwapChain.hpp"

#include <vector>
#include <filesystem>

namespace PhoenixEngine {
namespace Vulkan {
struct PipelineConfigInfo
{
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    std::vector<VkVertexInputBindingDescription> mVertexInputBindingDescriptions{};
    std::vector<VkVertexInputAttributeDescription> mVertexInputAttributeDescriptions{};

    VkPipelineViewportStateCreateInfo mViewportInfo;
    VkPipelineInputAssemblyStateCreateInfo mInputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo mRasterizationInfo;
    VkPipelineMultisampleStateCreateInfo mMultisamplingInfo;
    VkPipelineColorBlendAttachmentState mColorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo mColorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo mDepthStencilInfo;

    std::vector<VkDynamicState> mDynamicStateEnables;
    VkPipelineDynamicStateCreateInfo mDynamicStateInfo;

    VkPipelineLayout mPipelineLayout = nullptr;
    VkRenderPass mRenderPass = nullptr;

    uint32_t mSubpass = 0;
};
    
class Pipeline
{
public:
    Pipeline(
        Device& device,
        const std::string& vertFilePath,
        const std::string& fragFilePath,
        const PipelineConfigInfo& configInfo
        );
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void bind(VkCommandBuffer commandBuffer) const;

    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
private:
    void createGraphicsPipeline(
        const std::string& vertFilePath,
        const std::string& fragFilePath,
        const PipelineConfigInfo& configInfo
        );

    void createShaderModule(
        const std::vector<char>& code,
        VkShaderModule* shaderModule
    ) const;
    static std::vector<char> readFile(const std::filesystem::path& relative);

    Device& mDevice;
    VkPipeline mGraphicsPipeline;
    VkShaderModule mVertexShaderModule;
    VkShaderModule mFragmentShaderModule;
};
}    
}


