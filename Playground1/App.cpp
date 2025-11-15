#include "stdafx.h"
#include "App.h"

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

    void App::drawFrame()
    {
        // vk wait for fence 
		vkWaitForFences(*device.get(), 1, device.getInFlightFence(), VK_TRUE, UINT64_MAX);
		
		// reset fence
		vkResetFences(*device.get(), 1, device.getInFlightFence());
        
        uint32_t imageIndex;
		vkAcquireNextImageKHR(*device.get(), *device.getSwapchain(), UINT64_MAX , *device.getImageAvailableSemaphore(),nullptr,  &imageIndex);

		vkResetCommandBuffer(device.getCommandBuffer(), 0);
		device.recordCommandBuffer(device.getCommandBuffer(), imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { *device.getImageAvailableSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &device.getCommandBuffer();
		VkSemaphore signalSemaphores[] = { *device.getRenderFinishedSemaphore() };

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, *device.getInFlightFence()) != VK_SUCCESS) {
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

    	vkQueuePresentKHR(device.getPresentQueue(),  &presentInfo);
    }
}