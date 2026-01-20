#include "VulkanDevice.h"

#include <GLFW/glfw3.h>
#include <spdlog/fmt/ranges.h>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <vector>
#include<filesystem>

#include "stdafx.h"
#include "vulkan/vulkan_core.h"

namespace PhoenixEngine {
namespace Vulkan {
// local callback functions
static VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			  VkDebugUtilsMessageTypeFlagsEXT messageType,
			  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			  void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void
DestroyDebugUtilsMessengerEXT(VkInstance instance,
							  VkDebugUtilsMessengerEXT debugMessenger,
							  const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

Device::Device(Vulkan::Window& window) : mWindow{window} {
	createInstance();
	setupDebugMessenger();
	createSurface();
	choosePhysicalDevice();
	createLogicalDevice();
	createSwapchain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
}

QueueFamilyIndices Device::findQueueFamilies(const VkPhysicalDevice& device) const {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

	// Assign the queue family properties to the vector
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
											 queueFamilies.data());

	QueueFamilyIndices indices;

	for (int j = 0; j < queueFamilyCount; j++) {
		VkBool32 supportsPresentation;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, j, mSurface,
											 &supportsPresentation);
		if (supportsPresentation) {
			indices.presentFamily = j;
			indices.hasPresentFamily = true;
			spdlog::info("QueueFamily: {} supports presentation",
						 indices.presentFamily);
		}
		if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = j;
			indices.hasGraphicsFamily = true;
			spdlog::info("QueueFamily: {} supports graphics", indices.graphicsFamily);
		}
		if (indices.isComplete()) {
			break;
		}
	}

	return indices;
}

SwapchainSupportDetails Device::checkSwapchainSupport(const VkPhysicalDevice& device) {
	SwapchainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface,
											  &details.surfaceCapabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);

	details.formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount,
										 details.formats.data());
	spdlog::info("{} formats are supported by this surface",
				 details.formats.size());

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount,
											  nullptr);

	details.presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount,
											  details.presentModes.data());
	spdlog::info("{} present modes are supported by this surface",
				 details.presentModes.size());

	return details;
}

bool Device::checkExtensionSupport(const VkPhysicalDevice& device) const {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
										 nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);

	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
										 availableExtensions.data());
	spdlog::info(
		"Comparing available device extensions with the required ones ...");
	std::set<std::string> requiredExtensionsSet(mRequiredDeviceExtensions.begin(),
												mRequiredDeviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensionsSet.erase(extension.extensionName);
	}
	if (requiredExtensionsSet.empty()) {
		spdlog::info("All required device extensions are supported.");
		return true;
	} else {
		spdlog::error("Missing required device extensions:");
		return false;
	}
}

bool Device::isDeviceSuitable(const VkPhysicalDevice& device) {
	QueueFamilyIndices indices = findQueueFamilies(device);

	SwapchainSupportDetails swapchainDetails = checkSwapchainSupport(device);

	bool swapchainSuitable = !swapchainDetails.formats.empty() &&
							 !swapchainDetails.presentModes.empty();

	return indices.isComplete() && checkExtensionSupport(device) &&
		   swapchainSuitable;
}

void Device::choosePhysicalDevice() {
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

	for (uint32_t i = 0; i < deviceCount; i++) {
		VkPhysicalDeviceProperties currentDeviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &currentDeviceProperties);

		spdlog::info("Checking current device: {}",
					 currentDeviceProperties.deviceName);
		bool isSuitable = isDeviceSuitable(devices[i]);

		if (isSuitable) {
			spdlog::info("{} has required queue families",
						 currentDeviceProperties.deviceName);

			if (currentDeviceProperties.deviceType ==
				VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				spdlog::info("{} is discrete. Selecting...",
							 currentDeviceProperties.deviceName);

				mPhysicalDevice = devices[i];
				return;	 // We found a suitable discrete GPU, no need to continue
						 // searching
			}
			if (currentDeviceProperties.deviceType ==
				VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
				integratedSuitableDevice = devices[i];
			} else {
				if (!nonDiscreteSuitableDevice) {
					nonDiscreteSuitableDevice = devices[i];
				}
			}
		}
		// Loop ends here.
	}

	if (integratedSuitableDevice != VK_NULL_HANDLE) {
		mPhysicalDevice = integratedSuitableDevice;
	} else if (nonDiscreteSuitableDevice != VK_NULL_HANDLE) {
		mPhysicalDevice = nonDiscreteSuitableDevice;
	} else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void Device::createSurface() {
	mWindow.createSurface(mInstance, mSurface);
}

void Device::createLogicalDevice() {
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

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {
		graphicsQueueCreateInfo};

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

	if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) !=
		VK_SUCCESS) {
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
		createInfo.enabledLayerCount =
			static_cast<uint32_t>(mValidationLayers.size());
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

void Device::createSwapchain() {
	SwapchainSupportDetails swapchainSupportDetails =
		checkSwapchainSupport(mPhysicalDevice);

	VkSurfaceFormatKHR chosenFormat =
		chooseSwapchainFormat(swapchainSupportDetails.formats);
	VkPresentModeKHR chosenPresentMode =
		chooseSwapchainPresentMode(swapchainSupportDetails.presentModes);

	mSwapchainImageFormat = chosenFormat.format;
	mSwapchainExtent =
		mWindow.getSwapchainExtent(swapchainSupportDetails.surfaceCapabilities);

	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
	uint32_t indicesList[] = {indices.graphicsFamily, indices.presentFamily};

	uint32_t imageCount =
		swapchainSupportDetails.surfaceCapabilities.minImageCount + 1;

	if (swapchainSupportDetails.surfaceCapabilities.maxImageCount > 0 &&
		imageCount > swapchainSupportDetails.surfaceCapabilities.maxImageCount) {
		imageCount = swapchainSupportDetails.surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.imageFormat = chosenFormat.format;
	createInfo.imageColorSpace = chosenFormat.colorSpace;
	createInfo.presentMode = chosenPresentMode;
	createInfo.imageArrayLayers = 1;
	createInfo.imageExtent = mSwapchainExtent;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.minImageCount = imageCount;
	createInfo.surface = mSurface;

	if (mSwapchain != VK_NULL_HANDLE) {
		createInfo.oldSwapchain = mSwapchain;
	}

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = indicesList;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform =
		swapchainSupportDetails.surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE;

	VkSwapchainKHR newSwapchain;
	VkResult result =
		vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &newSwapchain);
	if (result != VK_SUCCESS) {
		std::cout << "Error: " << result << "\n";
		throw std::runtime_error("Failed to create swapchain!");
	}

	if (mSwapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
	}

	mSwapchain = newSwapchain;

	// spdlog::warn("Look at the difference between the swapchain buffers and the
	// " "frame buffers");

	uint32_t swapchainImageCount;
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &swapchainImageCount, nullptr);
	mSwapchainImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &swapchainImageCount,
							mSwapchainImages.data());
}

const VkSurfaceFormatKHR& Device::chooseSwapchainFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
	for (auto& format : availableFormats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return availableFormats[0];
}

const VkPresentModeKHR Device::chooseSwapchainPresentMode(
	const std::vector<VkPresentModeKHR>& availablePresentModes) const {
	for (auto& presentMode : availablePresentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

void Device::createImageViews() {
	spdlog::info("Readying image view buffer");
	mSwapchainImageViews.resize(mSwapchainImages.size());

	spdlog::info("Creating swapchain image views");
	spdlog::info("Total images: {}", mSwapchainImages.size());
	spdlog::info("Total images views: {}", mSwapchainImageViews.size());

	for (int i = 0; i < mSwapchainImages.size(); i++) {
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = mSwapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = mSwapchainImageFormat;

		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(mDevice, &imageViewCreateInfo, nullptr,
						  &mSwapchainImageViews[i]);
	}
}

void Device::createGraphicsPipeline() {
#pragma region READ_SHADER_FILES
	std::filesystem::path dirPath("./"); // your directory

	if (std::filesystem::exists(dirPath) &&
	    std::filesystem::is_directory(dirPath)) {
	  std::cout << "Files in " << dirPath << ":\n";

	  for (const auto &entry : std::filesystem::directory_iterator(dirPath)) {
	    if (entry.is_regular_file()) { // skip subdirectories
	      std::cout << "  " << entry.path().filename() << "\n";
	    }
	  }
	} else {
	  std::cout << dirPath << " does not exist or is not a directory.\n";
	}
	auto vertShaderFile = readFile("./shaders/out/shader.vert.spv");
	auto fragShaderFile = readFile("./shaders/out/shader.frag.spv");

	spdlog::info("Reading vertex shader file: {} bytes", vertShaderFile.size());
	spdlog::info("Reading fragment shader file: {} bytes", fragShaderFile.size());

	VkShaderModule vertShaderModule = createShaderModule(vertShaderFile);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderFile);
#pragma endregion

#pragma region SHADER_STAGE_CREATION
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
													  fragShaderStageInfo};
	spdlog::info("Shader stages created");
#pragma endregion

#pragma region CREATE_DYNAMIC_STAGE
	std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
												 VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	// dynamicState.pNext;
	dynamicState.flags;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
#pragma endregion

#pragma region VERTEX_INPUT_INFO
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;  // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;	 // Optional
#pragma endregion

#pragma region INPUT_ASSEMBLY
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
#pragma endregion

#pragma region VIEWPORT_AND_SCISSOR
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)mSwapchainExtent.width;
	viewport.height = (float)mSwapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = mSwapchainExtent;
#pragma endregion

#pragma region VIEWPORT_STATE_CREATE_INFO
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
#pragma endregion
	spdlog::warn(
		"Some parameters here require GPU feature enabling. Look into that!!");

#pragma region RASTERIZER
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
#pragma endregion

#pragma region MULTISAMPLING
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
#pragma endregion

#pragma region COLOR_BLEND_ATTACHMENT
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
#pragma endregion

#pragma region COLOR_BLENDING
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
#pragma endregion

#pragma region PIPELINE_LAYOUT
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;			// Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;	// Optional
#pragma endregion

	if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr,
							   &mPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
	spdlog::info("Pipeline layout created");

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext;
	pipelineInfo.flags;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pTessellationState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo,
								  nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	spdlog::info("Graphics pipeline created");
	// ########### SHADER MODULE CLEANUP ###########

	vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
	spdlog::info("Shader modules cleaned up");
}
void Device::createFrameBuffers() {
	mSwapchainFramebuffers.resize(mSwapchainImageViews.size());
	for (size_t i = 0; i < mSwapchainImageViews.size(); i++) {
		VkImageView attachments[] = {mSwapchainImageViews[i]};
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = mSwapchainExtent.width;
		framebufferInfo.height = mSwapchainExtent.height;
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr,
								&mSwapchainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
	spdlog::info("Framebuffers created");
}

void Device::createCommandPool() {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(mPhysicalDevice);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
	spdlog::info("Command pool created");
}

void Device::createCommandBuffers() {
	mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

	if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
	spdlog::error("Command buffer allocated");
}

void Device::recordCommandBuffer(VkCommandBuffer commandBuffer,
								 uint32_t imageIndex) const {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;				   // Optional
	beginInfo.pInheritanceInfo = nullptr;  // Optional
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mRenderPass;
	renderPassInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = mSwapchainExtent;
	VkClearValue clearColor = {{{0.0f, 0.5f, 0.5f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
						 VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					  mGraphicsPipeline);
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)mSwapchainExtent.width;
	viewport.height = (float)mSwapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = mSwapchainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}
void Device::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = mSwapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	spdlog::info("Render pass created");
}
void Device::setupDebugMessenger() {
	if (!enableValidationLayers)
		return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr,
									 &mDebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

std::vector<const char*> Device::getRequiredExtensions() const {
	uint32_t glfwExtensionCount;
	const char** glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions,
										glfwExtensions + glfwExtensionCount);

	uint32_t availableExtensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount,
										   nullptr);
	std::vector<VkExtensionProperties> availableExtensions(
		availableExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount,
										   availableExtensions.data());

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
			throw std::runtime_error("failed to find required extension!" +
									 std::string(glfwExtension));
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
	vkEnumerateInstanceLayerProperties(&availableLayerCount,
									   availableExtensions.data());

	spdlog::info("Validation Layer Required Extensions:");
	for (const auto& validationExtension : mValidationLayers) {
		spdlog::info("{}", validationExtension);

		// std::cout << validationExtension << '\n';

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
	VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;	 // Optional
}

std::vector<char> Device::readFile(const std::string& path) {
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	return buffer;
}

VkShaderModule Device::createShaderModule(const std::vector<char>& code) const {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}

void Device::createSyncObjects() {
	mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mRenderFinishedSemaphores.resize(mSwapchainImages.size());
	mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < mSwapchainImages.size(); i++) {
		if (vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr,
							  &mRenderFinishedSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create Semaphore");
		}
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr,
							  &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(mDevice, &fenceCreateInfo, nullptr,
						  &mInFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error(
				"failed to create fences and image available semaphores");
		}
	}
	spdlog::info("Semaphores created");
	spdlog::info("Fence created");
}

void Device::recreateSwapchain() {
	int width = 0, height = 0;
	glfwGetFramebufferSize(mWindow.get(), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(mWindow.get(), &width, &height);
		glfwWaitEvents();
	}

	spdlog::warn("{}, {}", width, height);
	vkDeviceWaitIdle(mDevice);

	cleanupSwapchainResources();

	createSwapchain();
	createImageViews();
	createFrameBuffers();
}

void Device::cleanupSwapchainResources() {
	for (auto framebuffer : mSwapchainFramebuffers) {
		vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
	}

	for (auto imageView : mSwapchainImageViews) {
		vkDestroyImageView(mDevice, imageView, nullptr);
	}
}

void Device::cleanupSwapchain() {
	cleanupSwapchainResources();

	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
}

Device::~Device() {
	spdlog::warn(
		"Figure out what resources are destroyed on their own and what "
		"resources need to be destroyed manually.");

	cleanupSwapchain();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
	}
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

	vkDestroyDevice(mDevice, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
}
}  // namespace Vulkan
}  // namespace PhoenixEngine
