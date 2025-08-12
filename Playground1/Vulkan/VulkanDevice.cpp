#include "stdafx.h"
#include "VulkanDevice.h"

#include <vector>

namespace PhoenixEngine {
    namespace Vulkan {
        // local callback functions
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

            return VK_FALSE;
        }

        VkResult CreateDebugUtilsMessengerEXT(
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

        void DestroyDebugUtilsMessengerEXT(
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
        
        Device::Device(Vulkan::Window& window) {
            createInstance();
            createSurface(window);
            choosePhysicalDevice();
            setupDebugMessenger();
        }

        bool Device::isDeviceSuitable(VkPhysicalDevice& device) {
            uint32_t queueFamilyCount;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
            // Assign the queue family properties to the vector
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());
            bool deviceSupportsPresentation = false;
            bool deviceHasGraphcisAndComputeQueue = false;
            for (int j = 0; j < queueFamilyCount; j++)
            {
                VkBool32 supportsPresentation;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, j, surface, &supportsPresentation);
                if (supportsPresentation == VK_TRUE)
                {
                    deviceSupportsPresentation = true;
                    std::cout << "Queue Family " << j << " supports presentation" << std::endl;
                }
                if ((queueFamilyProperties[j].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)))
                {
                    deviceHasGraphcisAndComputeQueue = true;
                    std::cout << "Queue found that has Graphics, Compute queue family and supports presentation" << std::endl;
                }
            }
			return (deviceSupportsPresentation && deviceHasGraphcisAndComputeQueue);
        }
        void Device::choosePhysicalDevice()
        {
            uint32_t deviceCount;
			// Get number of physical devices available
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            std::cout << "Found " << deviceCount << " devices" << std::endl;

            std::vector<VkPhysicalDevice> devices(deviceCount);
			// Assign the devices to devices vector
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            std::vector<VkPhysicalDeviceProperties> deviceProperties(deviceCount);
            // std::vector<std::vector<VkQueueFamilyProperties>> queueFamilyProperties(deviceCount);
			VkPhysicalDevice integratedSuitableDevice = VK_NULL_HANDLE;
            VkPhysicalDevice nonDiscreteSuitableDevice = VK_NULL_HANDLE;
            for (int i = 0; i < deviceCount; i++)
            {
                VkPhysicalDeviceProperties currentDeviceProperties;
                vkGetPhysicalDeviceProperties(devices[i], &currentDeviceProperties);
                bool isSuitable = isDeviceSuitable(devices[i]);
                if (isSuitable) {
                    std::cout << "Device Found!: " << currentDeviceProperties.deviceName << std::endl;
                    if (currentDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        std::cout << " Discrete GPU Found!: " << currentDeviceProperties.deviceName << std::endl;
						physicalDevice = devices[i];
                        return; // We found a suitable discrete GPU, no need to continue searching
                    }
                    else if (currentDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
						integratedSuitableDevice = devices[i];
                    }
                    else {
                        if (!nonDiscreteSuitableDevice) {
                            nonDiscreteSuitableDevice = devices[i];
                        }
                    }
                }
                else {
                    continue; // Skip unsuitable devices
                }
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
            
        
        void Device::createSurface (Vulkan::Window& window) {
            
            window.createSurface(instance, surface);
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
        
        std::vector<const char*> Device::getRequiredExtensions() {
            uint32_t glfwExtensionCount;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            uint32_t availableExtensionCount;
            vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());
            
            std::cout << "GLFW Required Instance Extensions:\n";
            for (const auto& glfwExtension : extensions) {
                std::cout << glfwExtension << '\n';

                bool found = false;
                for (const auto& availableExtension : availableExtensions) {
                    if (strcmp(availableExtension.extensionName, glfwExtension) == 0) {
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
            
            vkDestroyInstance(instance, nullptr);
        }
    }
}