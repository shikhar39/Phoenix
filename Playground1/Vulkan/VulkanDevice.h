#pragma once

#include <vector>

#include "VulkanWindow.h"
#include "glfw3.h"

namespace PhoenixEngine {
    namespace Vulkan {
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
            void setupDebugMessenger();
			void createSurface(Vulkan::Window&);
            bool isDeviceSuitable(VkPhysicalDevice&);

            std::vector<const char *> getRequiredExtensions();
            bool checkValidationLayerSupport() const;
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkSurfaceKHR surface;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			

            const std::vector<const char *> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        };
    }
}

