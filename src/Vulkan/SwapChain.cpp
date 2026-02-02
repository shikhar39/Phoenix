#include "stdafx.hpp"

#include "SwapChain.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace  PhoenixEngine {
namespace Vulkan {
SwapChain::SwapChain(Device& device, VkExtent2D extent) : mDevice{device}, mWindowExtent{extent} {
	init();
}

SwapChain::SwapChain(Device& device, VkExtent2D extent, std::shared_ptr<SwapChain> oldSwapChain) : mDevice{device}, mWindowExtent{extent}, mOldSwapChain{oldSwapChain} {
	init();
	mOldSwapChain = nullptr;
}

SwapChain::~SwapChain() {
	for (auto imageView : mSwapchainImageViews) {
		vkDestroyImageView(mDevice.get(), imageView, nullptr);
	}
	mSwapchainImageViews.clear();

	if (mSwapChain != nullptr) {
		vkDestroySwapchainKHR(mDevice.get(), mSwapChain, nullptr);
		mSwapChain = nullptr;
	}
}

void SwapChain::init() {
	createSwapChain();
	createImageViews();
	createRenderPass();
	// createDepthResources();
	createFrameBuffers();
	createSyncObjects();
}


void SwapChain::createSwapChain() {
	SwapchainSupportDetails swapchainSupportDetails = mDevice.getSwapchainSupport();

	VkSurfaceFormatKHR chosenFormat =
		chooseSurfaceFormat(swapchainSupportDetails.formats);
	VkPresentModeKHR chosenPresentMode =

		choosePresentMode(swapchainSupportDetails.presentModes);

	mImageFormat = chosenFormat.format;
	mSwapChainExtent = chooseExtent(swapchainSupportDetails.surfaceCapabilities);

	QueueFamilyIndices indices = mDevice.getQueueFamilies();
	uint32_t indicesList[] = {indices.graphicsFamily, indices.presentFamily};

	uint32_t imageCount =
		swapchainSupportDetails.surfaceCapabilities.minImageCount + 1;

	if (swapchainSupportDetails.surfaceCapabilities.maxImageCount > 0 &&
		imageCount > swapchainSupportDetails.surfaceCapabilities.maxImageCount) {
		imageCount = swapchainSupportDetails.surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mDevice.getSurface();

	createInfo.imageFormat = chosenFormat.format;
	createInfo.imageColorSpace = chosenFormat.colorSpace;
	createInfo.presentMode = chosenPresentMode;
	createInfo.imageArrayLayers = 1;
	createInfo.imageExtent = mSwapChainExtent;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.minImageCount = imageCount;

	createInfo.oldSwapchain = mOldSwapChain == nullptr ? VK_NULL_HANDLE : mOldSwapChain->mSwapChain;

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = indicesList;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform =
		swapchainSupportDetails.surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE;

	VkResult result =
	vkCreateSwapchainKHR(mDevice.get(), &createInfo, nullptr, &mSwapChain);
	if (result != VK_SUCCESS) {
		std::cout << "Error: " << result << "\n";
		throw std::runtime_error("Failed to create swapchain!");
	}
	// spdlog::warn("Look at the difference between the swapchain buffers and the
	// " "frame buffers");

	uint32_t swapchainImageCount;
	spdlog::info("tried something here!");
	vkGetSwapchainImagesKHR(mDevice.get(), mSwapChain, &swapchainImageCount, nullptr);
	mSwapchainImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(mDevice.get(), mSwapChain, &swapchainImageCount,
							mSwapchainImages.data());
}

void SwapChain::createImageViews() {
	spdlog::info("Readying image view buffer");
	mSwapchainImageViews.resize(mSwapchainImages.size());

	spdlog::info("Creating swapchain image views");
	spdlog::info("Total images: {}", mSwapchainImages.size());
	spdlog::info("Total images views: {}", mSwapchainImageViews.size());

	for (int i = 0; i < mSwapchainImages.size(); i++) {
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = mSwapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = mImageFormat;

		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(mDevice.get(), &imageViewCreateInfo, nullptr,
						  &mSwapchainImageViews[i]);
	}
}

void SwapChain::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = mImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(mDevice.get(), &renderPassInfo, nullptr, &mRenderPass) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	spdlog::info("Render pass created");
}

void SwapChain::createFrameBuffers() {
	mFrameBuffers.resize(mSwapchainImageViews.size());
	for (size_t i = 0; i < mSwapchainImageViews.size(); i++) {
		VkImageView attachments[] = {mSwapchainImageViews[i]};
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = mSwapChainExtent.width;
		framebufferInfo.height = mSwapChainExtent.height;
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(mDevice.get(), &framebufferInfo, nullptr,
								&mFrameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
	spdlog::info("FrameBuffers created");
}

void SwapChain::createSyncObjects() {
	mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mRenderFinishedSemaphores.resize(mSwapchainImages.size());
	mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < mSwapchainImages.size(); i++) {
		if (vkCreateSemaphore(mDevice.get(), &semaphoreCreateInfo, nullptr,
							  &mRenderFinishedSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create Semaphore");
		}
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(mDevice.get(), &semaphoreCreateInfo, nullptr,
							  &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(mDevice.get(), &fenceCreateInfo, nullptr,
						  &mInFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error(
				"failed to create fences and image available semaphores");
		}
	}
	spdlog::info("Semaphores created");
	spdlog::info("Fence created");
}

const VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
	for (auto& format : availableFormats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return availableFormats[0];
}

const VkPresentModeKHR SwapChain::choosePresentMode(
	const std::vector<VkPresentModeKHR>& availableModes) const {
	for (auto& presentMode : availableModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

const VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actualExtent = mWindowExtent;
		actualExtent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actualExtent.width)
		);
		actualExtent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height)
		);
		return actualExtent;
	}
}

const VkResult SwapChain::acquireNextImage(uint32_t *imageIndex) const {
	vkWaitForFences(
		mDevice.get(),
		1,
		&mInFlightFences[mCurrentFrame],
		VK_TRUE,
		std::numeric_limits<uint64_t>::max()
	);

	VkResult result = vkAcquireNextImageKHR(
		mDevice.get(),
		mSwapChain,
		std::numeric_limits<uint64_t>::max(),
		mImageAvailableSemaphores[mCurrentFrame],  // must be a not signaled semaphore
		VK_NULL_HANDLE,
		imageIndex
	);

	return result;
}

const VkResult SwapChain::submitCommandBuffer(const VkCommandBuffer* buffers, const uint32_t *imageIndex) {
	vkResetFences(mDevice.get(), 1, &mInFlightFences[mCurrentFrame]);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = buffers;
	VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[*imageIndex] };

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if (vkQueueSubmit(mDevice.getGraphicsQueue(), 1, &submitInfo,
		mInFlightFences[mCurrentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { mSwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = imageIndex;
	presentInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(mDevice.getPresentQueue(), &presentInfo);


	spdlog::warn("I drew");

	mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return result;
}


} // namespace Vulkan
} // namespace PhoenixEngine
