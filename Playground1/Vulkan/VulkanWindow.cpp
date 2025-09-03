#include "stdafx.h"
#include "VulkanWindow.h"

namespace PhoenixEngine {
    namespace Vulkan {    
        Window::Window(int inWidth, int inHeight, std::string inName) : PhoenixEngine::Window(inWidth, inHeight, inName) {}
        
        
        void Window::createSurface(VkInstance& instance, VkSurfaceKHR& surface) const
        {
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create window surface!");
			}
        }

        VkExtent2D Window::getSwapchainExtent(VkSurfaceCapabilitiesKHR& capabilities)
        {
            if (capabilities.currentExtent.width != UINT32_MAX)
            {
                return capabilities.currentExtent;
            }

            spdlog::info("Window manager requested manual setting of swapchain extent");

            int width;
            int height;
            
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D newExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

            newExtent.width = std::clamp(newExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            newExtent.height = std::clamp(newExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return newExtent;
        }
    }
}