#include "App.h"

#include <cstdint>
#include <stdexcept>

#include "stdafx.h"
#include "vulkan/vulkan_core.h"

namespace PhoenixEngine {

App::App() {
}

App::~App() {
}

void App::run() {
	spdlog::info("Starting Phoenix Engine app");
	while (!window.shouldClose()) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(*device.get());
}

void App::drawFrame() {
	uint32_t currentFrame = device.getCurrentFrame();

	// vk wait for fence
	vkWaitForFences(*device.get(), 1, &device.getInFlightFence(currentFrame),
					VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;

	VkResult result =
		vkAcquireNextImageKHR(*device.get(), *device.getSwapchain(), UINT64_MAX,
							  device.getImageAvailableSemaphore(currentFrame),
							  VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		spdlog::warn("trying to recreate swapchain");
		device.recreateSwapchain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swapchain image");
	}

	// reset fence
	vkResetFences(*device.get(), 1, &device.getInFlightFence(currentFrame));

	vkResetCommandBuffer(device.getCommandBuffer(currentFrame), 0);
	device.recordCommandBuffer(device.getCommandBuffer(currentFrame), imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = {
		device.getImageAvailableSemaphore(currentFrame)};
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &device.getCommandBuffer(currentFrame);
	VkSemaphore signalSemaphores[] = {
		device.getRenderFinishedSemaphore(imageIndex)};

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo,
					  device.getInFlightFence(currentFrame)) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {*device.getSwapchain()};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
		window.wasframebufferResized()) {
		spdlog::warn("queue has issue too!");
		window.resetFramebufferResized();
		device.recreateSwapchain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swapchain image!");
	}

	spdlog::warn("I drew");
	device.advanceFrame();
}
}  // namespace PhoenixEngine
