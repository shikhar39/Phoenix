#pragma once

#include "stdafx.hpp"

#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "SwapChain.hpp"
#include "Model.hpp"

#include <memory>
#include <cstdint>
#include <filesystem>

#include "Pipeline.hpp"

namespace PhoenixEngine {
namespace Vulkan {
class Renderer {
public:
	Renderer(Window&, Device&);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer &operator=(const Renderer&) = delete;

	void drawFrame();
	void loadModels();

	VkCommandBuffer getCurrentCommandBuffer() const { return mCommandBuffers[currentFrameIndex]; }
	VkRenderPass getSwapChainRenderPass() const { return mSwapChain->getRenderPass(); }

private:
	void createCommandBuffers();
	void freeCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer&, uint32_t imageIndex) const;
	void recreateSwapChain();

	//Temporary
	void createPipeline();
	void createPipelineLayout();
	VkPipelineLayout mPipelineLayout;
	std::unique_ptr<Pipeline> mPipeline;
	
	Window& mWindow;
	Device& mDevice;
	std::unique_ptr<SwapChain> mSwapChain;

	std::unique_ptr<Model> mModel;

	std::vector<VkCommandBuffer> mCommandBuffers;

	uint32_t mCurrentImageIndex;
	uint32_t currentFrameIndex{0};
};
}
}
