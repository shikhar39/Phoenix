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
            Device();
            ~Device();

        private:
            void choosePhysicalDevice();
            void createInstance();
            void setupDebugMessenger();

            std::vector<const char *> getRequiredExtensions();
            bool checkValidationLayerSupport() const;
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;

            const std::vector<const char *> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        };
    }
}

