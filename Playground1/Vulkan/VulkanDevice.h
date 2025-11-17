#pragma once

#include <vector>

#include "VulkanWindow.h"
#include "glfw3.h"

namespace PhoenixEngine {
    namespace Vulkan {

    	const int MAX_FRAMES_IN_FLIGHT = 2;
    	
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
        public:
	        const VkDevice* get() const { return &mDevice; }
	        const VkFence* getInFlightFence(uint32_t frameIndex) const { return &mInFlightFences[frameIndex]; }
	        const VkSemaphore* getImageAvailableSemaphore(uint32_t frameIndex) const { return &mImageAvailableSemaphores[frameIndex]; }
	        const VkSemaphore* getRenderFinishedSemaphore(uint32_t frameIndex) const { return &mRenderFinishedSemaphores[frameIndex]; }
	        const VkSwapchainKHR* getSwapchain() const { return &mSwapchain; }
	        VkCommandBuffer& getCommandBuffer(uint32_t frameIndex) { return mCommandBuffers[frameIndex]; }
	        VkQueue getGraphicsQueue() const { return mGraphicsQueue; }
	        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;
	        VkQueue getPresentQueue() const { return mPresentQueue; }
			void recreateSwapchain();

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
	        void createRenderPass();
	        void createGraphicsPipeline();
	        void createFrameBuffers();
	        void createCommandPool();
	        void createCommandBuffers();

        	void cleanupSwapchain();
        	
	        QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice&) const;
	        SwapchainSupportDetails checkSwapchainSupport(const VkPhysicalDevice&);
	        bool isDeviceSuitable(const VkPhysicalDevice&);
	        bool checkExtensionSupport(const VkPhysicalDevice& device) const ;
	        std::vector<const char *> getRequiredExtensions() const;
	        bool checkValidationLayerSupport() const;
	        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

	        static std::vector<char> readFile(const std::string& path);
	        VkShaderModule createShaderModule(const std::vector<char>& code) const ;
            
	        void createSyncObjects();


	        Window& mWindow;

	        VkInstance mInstance;
	        VkDebugUtilsMessengerEXT mDebugMessenger;
	        VkSurfaceKHR mSurface;
	        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	        VkDevice mDevice;

	        VkQueue mGraphicsQueue;
	        VkQueue mPresentQueue;

	        SwapchainSupportDetails mSwapchainSupportDetails;
        	std::unique_ptr<VkSwapchainKHR> mOldSwapchain = nullptr;
	        std::unique_ptr<VkSwapchainKHR> mSwapchain;
	        VkFormat mSwapchainImageFormat;
	        VkExtent2D mSwapchainExtent;

	        std::vector<VkImage> mSwapchainImages;
	        std::vector<VkImageView> mSwapchainImageViews;
	        std::vector<VkFramebuffer> mSwapchainFramebuffers;

	        VkRenderPass mRenderPass;
	        VkPipelineLayout mPipelineLayout;
	        VkPipeline mGraphicsPipeline;
            
	        VkCommandPool mCommandPool;
	        std::vector<VkCommandBuffer> mCommandBuffers;

	        std::vector<VkSemaphore> mImageAvailableSemaphores;
	        std::vector<VkSemaphore> mRenderFinishedSemaphores;
	        std::vector<VkFence> mInFlightFences;

        	
	        const std::vector<const char *> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };

	        const std::vector<const char*> mRequiredDeviceExtensions = { 
		        VK_KHR_SWAPCHAIN_EXTENSION_NAME
	        };
        };
    }
}

