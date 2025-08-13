#include "stdafx.h"
#include "VulkanDevice.h"

#include <vector>
#include <spdlog/fmt/ranges.h>

namespace PhoenixEngine {
    namespace Vulkan {
        // local callback functions
        static VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData)
        {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

            return VK_FALSE;
        }

        static VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
            const VkAllocationCallbacks *pAllocator,
            VkDebugUtilsMessengerEXT *pDebugMessenger) {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                instance,
                "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr) {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            } else {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }

        static void DestroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks *pAllocator) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                instance,
                "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(instance, debugMessenger, pAllocator);
            }
        }
        
        Device::Device(const Vulkan::Window& window) {
            createInstance();
            setupDebugMessenger();
            createSurface(window);
            choosePhysicalDevice();
            createLogicalDevice();
        }

        QueueFamilyIndices Device::findQueueFamilies(const VkPhysicalDevice& device) const
        {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

            // Assign the queue family properties to the vector
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            QueueFamilyIndices indices;

            for (int j = 0; j < queueFamilyCount; j++)
            {
                VkBool32 supportsPresentation;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, j, surface, &supportsPresentation);
                if (supportsPresentation)
                {
                    indices.presentFamily = j;
                    indices.hasPresentFamily = true;
                    spdlog::info("QueueFamily: {} supports presentation", indices.presentFamily);
                    // std::cout << "Queue Family " << j << " supports presentation" << std::endl;
                }
                if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    indices.graphicsFamily = j;
                    indices.hasGraphicsFamily = true;
                    spdlog::info("QueueFamily: {} supports graphics", indices.graphicsFamily);
                }
                if (indices.isComplete())
                {
                    break;
                }
            }
            
            return indices;
        }
        
        bool Device::isDeviceSuitable(const VkPhysicalDevice& device) {
            QueueFamilyIndices indices = findQueueFamilies(device);
            
            return indices.isComplete();
        }
        
        void Device::choosePhysicalDevice()
        {
            uint32_t deviceCount;

            // Get number of physical devices available
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            spdlog::info("Found {} physical devices", deviceCount);
            
            std::vector<VkPhysicalDevice> devices(deviceCount);

            // Assign the devices to device vector
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            std::vector<VkPhysicalDeviceProperties> deviceProperties(deviceCount);

            VkPhysicalDevice integratedSuitableDevice = VK_NULL_HANDLE;
            VkPhysicalDevice nonDiscreteSuitableDevice = VK_NULL_HANDLE;

            for (uint32_t i = 0; i < deviceCount; i++)
            {
                VkPhysicalDeviceProperties currentDeviceProperties;
                vkGetPhysicalDeviceProperties(devices[i], &currentDeviceProperties);

                spdlog::info("Checking current device: {}", currentDeviceProperties.deviceName);
                bool isSuitable = isDeviceSuitable(devices[i]);

                if (isSuitable) {
                    spdlog::info("{} has required queue families", currentDeviceProperties.deviceName);

                    if (currentDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        spdlog::info("{} is discrete. Selecting...", currentDeviceProperties.deviceName);
                        
						physicalDevice = devices[i];
                        return; // We found a suitable discrete GPU, no need to continue searching
                    }
                    if (currentDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
						integratedSuitableDevice = devices[i];
                    }
                    else {
                        if (!nonDiscreteSuitableDevice) {
                            nonDiscreteSuitableDevice = devices[i];
                        }
                    }
                }
                //Loop ends here.
            }

            if (integratedSuitableDevice != VK_NULL_HANDLE) {
				physicalDevice = integratedSuitableDevice;
			}
            else if (nonDiscreteSuitableDevice != VK_NULL_HANDLE) {
				physicalDevice = nonDiscreteSuitableDevice;
            }
            else {
				throw std::runtime_error("failed to find a suitable GPU!"); 
            }
        }
            
        
        void Device::createSurface (const Vulkan::Window& window) {
            window.createSurface(instance, surface);
        }

        void Device::createLogicalDevice()
        {
            VkDeviceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        }

        void Device::createInstance() {
            if (enableValidationLayers && !checkValidationLayerSupport()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            VkApplicationInfo appInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Phoenix Engine";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            auto extensions = getRequiredExtensions();

            VkInstanceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
            if (enableValidationLayers) {
                
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);
                createInfo.pNext = &debugCreateInfo;
            } else {
                createInfo.enabledLayerCount = 0;
                createInfo.ppEnabledLayerNames = nullptr;
            }
            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance!");
            }
        }

        void Device::setupDebugMessenger() {
            if (!enableValidationLayers) return;
            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);
            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }
        
        std::vector<const char*> Device::getRequiredExtensions() const
        {
            uint32_t glfwExtensionCount;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            uint32_t availableExtensionCount;
            vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());
            
            spdlog::info("GLFW required extensions: {}", extensions);
            for (const auto& glfwExtension : extensions) {
                bool found = false;

                for (const auto& availableExtension : availableExtensions) {
                    if (strcmp(availableExtension.extensionName, glfwExtension) == 0) {
                        spdlog::info("Found extension: {}", availableExtension.extensionName);
                        found = true;
                    }
                }
                if (!found) {
                    throw std::runtime_error("failed to find required extension!" + std::string(glfwExtension));
                }
            }

            if (enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            
            return extensions;
        }

        bool Device::checkValidationLayerSupport() const {
            uint32_t availableLayerCount;
            vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
            std::vector<VkLayerProperties> availableExtensions(availableLayerCount);
            vkEnumerateInstanceLayerProperties(&availableLayerCount, availableExtensions.data());
            
            std::cout << "Validation Layer Required Extensions:\n";
            for (const auto& validationExtension : validationLayers) {
                std::cout << validationExtension << '\n';

                bool found = false;
                for (const auto& availableLayer : availableExtensions) {
                    if (strcmp(availableLayer.layerName, validationExtension) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            }

            return true;
        }

        void Device::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
            createInfo.pUserData = nullptr;  // Optional
        }

        
        Device::~Device() {
            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
            }

            vkDestroySurfaceKHR(instance, surface, nullptr);
            vkDestroyInstance(instance, nullptr);
        }
    }
}