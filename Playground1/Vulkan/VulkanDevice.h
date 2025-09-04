#pragma once

#include <vector>

#include "VulkanWindow.h"
#include "glfw3.h"

namespace PhoenixEngine {
    namespace Vulkan {

        struct QueueFamilyIndices
        {
            uint32_t graphicsFamily;
            uint32_t presentFamily;
            bool hasGraphicsFamily = false;
            bool hasPresentFamily = false;

            bool isComplete() { return hasGraphicsFamily && hasPresentFamily; }
        };

        struct SwapchainSupportDetails
        {
            VkSurfaceCapabilitiesKHR surfaceCapabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };
        
        class Device {
#ifdef NDEBUG
            const bool enableValidationLayers = false;
#else
            const bool enableValidationLayers = true;
#endif
            
            public:
             Device(Vulkan::Window &window);
            ~Device();

        private:
            void choosePhysicalDevice();
            void createInstance();
            void createSwapchain();
            const VkExtent2D setSwapchainExtent(VkSurfaceCapabilitiesKHR& capabilities);
            const VkSurfaceFormatKHR& chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
            const VkPresentModeKHR chooseSwapchainPresentMode(
                const std::vector<VkPresentModeKHR>& availablePresentModes) const;
            void setupDebugMessenger();
			void createSurface();
            void createLogicalDevice();
            void createSwapchainImageViews();
            void createGraphicsPipeline();

            
            QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice&) const;
            SwapchainSupportDetails checkSwapchainSupport(const VkPhysicalDevice&);
            bool isDeviceSuitable(const VkPhysicalDevice&);
            bool checkExtensionSupport(const VkPhysicalDevice& device) const ;
            std::vector<const char *> getRequiredExtensions() const;
            bool checkValidationLayerSupport() const;
            static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

            static std::vector<char> readFile(const std::string& path);
            
            Window& mWindow;

            VkInstance mInstance;
            VkDebugUtilsMessengerEXT mDebugMessenger;
            VkSurfaceKHR mSurface;
			VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
            VkDevice mDevice;

			VkQueue mGraphicsQueue;
			VkQueue mPresentQueue;

            SwapchainSupportDetails mSwapchainSupportDetails;
            VkSwapchainKHR mSwapchain;
			VkFormat mSwapchainImageFormat;
            VkExtent2D mSwapchainExtent;

			std::vector<VkImage> mSwapchainImages;
            std::vector<VkImageView> mSwapchainImageViews;

            const std::vector<const char *> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };

            const std::vector<const char*> mRequiredDeviceExtensions = { 
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
        };
    }
}

