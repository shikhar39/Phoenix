#pragma once

#include "VulkanWindow.hpp"

#include <cstdint>
#include <vector>

namespace PhoenixEngine {
namespace Vulkan {
const int MAX_FRAMES_IN_FLIGHT = 2;

struct QueueFamilyIndices

{
	uint32_t graphicsFamily;

	uint32_t presentFamily;

	bool hasGraphicsFamily = false;

	bool hasPresentFamily = false;

	bool isComplete() {
		return hasGraphicsFamily && hasPresentFamily;
	}
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
	Device(Window& window);
	~Device();

	const VkDevice& get() const { return mDevice; }

	const VkSurfaceKHR& getSurface() const { return mSurface; }

	const SwapchainSupportDetails getSwapchainSupport() { return checkSwapchainSupport(mPhysicalDevice); }
	const QueueFamilyIndices getQueueFamilies() { return findQueueFamilies(mPhysicalDevice); }

	const VkCommandPool& getCommandPool() { return mCommandPool; }

	VkQueue getGraphicsQueue() const {
		return mGraphicsQueue;
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer,
							 uint32_t imageIndex) const;

	VkQueue getPresentQueue() const {
		return mPresentQueue;
	}

private:
	void choosePhysicalDevice();
	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void createLogicalDevice();

	void createCommandPool();

	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice&) const;

	SwapchainSupportDetails checkSwapchainSupport(const VkPhysicalDevice&);

	bool isDeviceSuitable(const VkPhysicalDevice&);

	bool checkExtensionSupport(const VkPhysicalDevice& device) const;

	std::vector<const char*> getRequiredExtensions() const;

	bool checkValidationLayerSupport() const;

	static void populateDebugMessengerCreateInfo(
		VkDebugUtilsMessengerCreateInfoEXT& createInfo);


	Window& mWindow;

	VkInstance mInstance;

	VkDebugUtilsMessengerEXT mDebugMessenger;

	VkSurfaceKHR mSurface;

	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

	VkDevice mDevice;

	VkQueue mGraphicsQueue;

	VkQueue mPresentQueue;

	SwapchainSupportDetails mSwapchainSupportDetails;

	VkCommandPool mCommandPool;

	const std::vector<const char*> mValidationLayers = {
		"VK_LAYER_KHRONOS_validation"};

	const std::vector<const char*> mRequiredDeviceExtensions = {

		VK_KHR_SWAPCHAIN_EXTENSION_NAME

	};
};
}  // namespace Vulkan
}  // namespace PhoenixEngine
