// #pragma once
//
// #include <vulkan/vulkan.h>
//
// #include "VulkanDevice.h"
//
//
//
// namespace PhoenixEngine
// {
//     namespace  Vulkan
//     {
//         struct SwapchainSupportDetails
//         {
//             VkSurfaceCapabilitiesKHR surfaceCapabilities;
//             std::vector<VkSurfaceFormatKHR> formats;
//             std::vector<VkPresentModeKHR> presentModes;
//         };
//
//         class Swapchain
//         {
//         public:
//             Swapchain(Device&);
//         private:
//             Device& mDevice;
//             
//             VkSwapchainKHR mSwapchain;
//             std::vector<VkImage> mSwapchainImages;
//             std::vector<VkImageView> mSwapchainImageViews;
//
//         };
//     }
// }
//
//
