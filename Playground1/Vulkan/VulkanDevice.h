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
        
        class Device {
#ifdef NDEBUG
            const bool enableValidationLayers = false;
#else
            const bool enableValidationLayers = true;
#endif
            
            public:
             Device(const Vulkan::Window &window);
            ~Device();

        private:
            void choosePhysicalDevice();
            void createInstance();
            void setupDebugMessenger();
			void createSurface(const Vulkan::Window&);
            void createLogicalDevice();

            
            QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice&) const;
            bool isDeviceSuitable(const VkPhysicalDevice&);

            std::vector<const char *> getRequiredExtensions() const;
            bool checkValidationLayerSupport() const;
            static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkSurfaceKHR surface;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            
            const std::vector<const char *> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        };
    }
}

