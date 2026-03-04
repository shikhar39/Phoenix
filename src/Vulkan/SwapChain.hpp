#pragma once

#include "VulkanDevice.hpp"

#include <cstdint>
#include <memory>

namespace PhoenixEngine {
namespace  Vulkan {
class SwapChain {
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	SwapChain(Device&, VkExtent2D);
	SwapChain(Device&, VkExtent2D, std::shared_ptr<SwapChain> oldSwapchain);

	~SwapChain();

	SwapChain(const SwapChain &) = delete;
	SwapChain &operator=(const SwapChain &) = delete;

	VkRenderPass& getRenderPass() { return mRenderPass; }
	VkFramebuffer& getFrameBuffer(size_t index) { return mFrameBuffers[index]; }
	VkExtent2D& getSwapChainExtent() { return mSwapChainExtent; }

	const VkResult acquireNextImage(uint32_t *) const;
	const VkResult submitCommandBuffer(const VkCommandBuffer*, const uint32_t*);
private:
	void init();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	// void createDepthResources();
	void createFrameBuffers();
	void createSyncObjects();

	const VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&) const;
	const VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>&) const;
	const VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR&) const;


	VkSwapchainKHR mSwapChain;
	std::shared_ptr<SwapChain> mOldSwapChain;

	Device& mDevice;
	VkExtent2D mWindowExtent;

	VkExtent2D mSwapChainExtent;
	VkFormat mImageFormat;
	VkPresentModeKHR mPresentMode;

	std::vector<VkImage> mSwapchainImages;
	std::vector<VkImageView> mSwapchainImageViews;
	VkRenderPass mRenderPass;

	std::vector<VkFramebuffer> mFrameBuffers;


	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mInFlightFences;

	uint32_t mCurrentFrame = 0;
};

}
}
