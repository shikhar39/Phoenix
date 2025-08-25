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
                vkGetPhysicalDeviceSurfaceSupportKHR(device, j, mSurface, &supportsPresentation);
                if (supportsPresentation)
                {
                    indices.presentFamily = j;
                    indices.hasPresentFamily = true;
                    spdlog::info("QueueFamily: {} supports presentation", indices.presentFamily);
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
        

        bool Device::checkExtensionSupport (const VkPhysicalDevice& device) const {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
			spdlog::info("Comparing available device extensions with the required ones ...");
            std::set<std::string> requiredExtensionsSet(mRequiredDeviceExtensions.begin(), mRequiredDeviceExtensions.end());
            for (const auto& extension : availableExtensions) {
                requiredExtensionsSet.erase(extension.extensionName);
            }
            if (requiredExtensionsSet.empty()) {
                spdlog::info("All required device extensions are supported.");
                return true;
			}
            else {
                spdlog::error("Missing required device extensions:");
                return false;
            }
		}

        bool Device::isDeviceSuitable(const VkPhysicalDevice& device) {
            QueueFamilyIndices indices = findQueueFamilies(device);

            return indices.isComplete() && checkExtensionSupport(device);
        }
        
        void Device::choosePhysicalDevice()
        {
            uint32_t deviceCount;

            // Get number of physical devices available
            vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

            spdlog::info("Found {} physical devices", deviceCount);
            
            std::vector<VkPhysicalDevice> devices(deviceCount);

            // Assign the devices to device vector
            vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

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
                        
						mPhysicalDevice = devices[i];
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
				mPhysicalDevice = integratedSuitableDevice;
			}
            else if (nonDiscreteSuitableDevice != VK_NULL_HANDLE) {
				mPhysicalDevice = nonDiscreteSuitableDevice;
            }
            else {
				throw std::runtime_error("failed to find a suitable GPU!"); 
            }
        }
            
        
        void Device::createSurface (const Vulkan::Window& window) {
            window.createSurface(mInstance, mSurface);
        }

        void Device::createLogicalDevice()
        {
            QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
            float queuePriority[] = {1.0f, 1.0f};
            
            VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
            graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            graphicsQueueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
            graphicsQueueCreateInfo.queueCount = 2;
            graphicsQueueCreateInfo.pQueuePriorities = queuePriority;
            graphicsQueueCreateInfo.pNext = nullptr;
			// TODO: Maybe use a set to manage duplicate queue family indices
            spdlog::warn("Maybe use a set to manage duplicate queue family indices");
            // VkDeviceQueueCreateInfo presentQueueCreateInfo = {};
            // presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            // presentQueueCreateInfo.queueFamilyIndex = indices.presentFamily;
            // presentQueueCreateInfo.queueCount = 1;
            // presentQueueCreateInfo.pQueuePriorities = &queuePriority;
            // presentQueueCreateInfo.pNext = nullptr;

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {graphicsQueueCreateInfo};

            spdlog::info("Queues requested from {} families", queueCreateInfos.size());
            
            VkDeviceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			// TODO: Add extensions and features when stuff starts breaking
            spdlog::warn("Add extensions and features when stuff starts breaking");

            createInfo.enabledExtensionCount = mRequiredDeviceExtensions.size();
            createInfo.ppEnabledExtensionNames = mRequiredDeviceExtensions.data();
            createInfo.pEnabledFeatures = nullptr;
            createInfo.queueCreateInfoCount = 1;
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.pNext = nullptr;

            if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create logical device!");
            }

			spdlog::info("Logical device created");

			vkGetDeviceQueue(mDevice, indices.graphicsFamily, 0, &mGraphicsQueue);
            vkGetDeviceQueue(mDevice, indices.presentFamily, 0, &mPresentQueue);
			spdlog::info("Graphics and present queue handles created.");
        }

        void Device::createInstance() {
            if (enableValidationLayers && !checkValidationLayerSupport()) {
				spdlog::error("Validation layers requested, but not available!");
                throw std::runtime_error("Validation layers requested, but not available!");
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
                
                createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
                createInfo.ppEnabledLayerNames = mValidationLayers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);
                createInfo.pNext = &debugCreateInfo;
            } else {
                createInfo.enabledLayerCount = 0;
                createInfo.ppEnabledLayerNames = nullptr;
            }
            if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance!");
            }
        }

        void Device::setupDebugMessenger() {
            if (!enableValidationLayers) return;
            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);
            if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
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
            
            spdlog::info("Validation Layer Required Extensions:");
            for (const auto& validationExtension : mValidationLayers) {
                spdlog::info("{}", validationExtension);

                //std::cout << validationExtension << '\n';

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
                DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
            }

            vkDestroyDevice(mDevice, nullptr);
            vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
            vkDestroyInstance(mInstance, nullptr);
        }
    }
}