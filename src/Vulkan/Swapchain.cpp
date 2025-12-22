#include "stdafx.h"
// #include "Swapchain.h"
//
// namespace  PhoenixEngine
// {
//     namespace Vulkan
//     {
//         Swapchain::Swapchain(Device& device) : mDevice{device}
//         {
//             
//             SwapchainSupportDetails swapchainSupportDetails = mDevice.checkSwapchainSupport(mPhysicalDevice);
//
//             VkSurfaceFormatKHR chosenFormat = chooseSwapchainFormat(swapchainSupportDetails.formats);
// 			mSwapchainImageFormat = chosenFormat.format;
//             VkPresentModeKHR chosenPresentMode = chooseSwapchainPresentMode(swapchainSupportDetails.presentModes);
//
//             QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
//             uint32_t indicesList[] = {indices.graphicsFamily, indices.presentFamily};
//
//             uint32_t imageCount = swapchainSupportDetails.surfaceCapabilities.minImageCount + 1;
//
//             if (swapchainSupportDetails.surfaceCapabilities.maxImageCount > 0 && imageCount > swapchainSupportDetails.surfaceCapabilities.maxImageCount)
//             {
//                 imageCount = swapchainSupportDetails.surfaceCapabilities.maxImageCount;
//             }
//
//             VkSwapchainCreateInfoKHR createInfo = {};
//             createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//             createInfo.imageFormat = chosenFormat.format;
//             createInfo.imageColorSpace = chosenFormat.colorSpace;
//             createInfo.presentMode = chosenPresentMode;
//             createInfo.imageArrayLayers = 1;
//             createInfo.imageExtent = mWindow.getSwapchainExtent(swapchainSupportDetails.surfaceCapabilities);
//             createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//             createInfo.minImageCount = imageCount;
//             createInfo.surface = mSurface;
//
//             if (indices.graphicsFamily != indices.presentFamily)
//             {
//                 createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//                 createInfo.queueFamilyIndexCount = 2;
//                 createInfo.pQueueFamilyIndices = indicesList;
//             }
//             else
//             {
//                 createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//                 createInfo.queueFamilyIndexCount = 0;
//                 createInfo.pQueueFamilyIndices = nullptr;
//             }
//
//             createInfo.preTransform = swapchainSupportDetails.surfaceCapabilities.currentTransform;
//             createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//             createInfo.clipped = VK_TRUE;
//
//             if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("Failed to create swapchain!");
//             }
//
//             spdlog::warn("Look at the difference between the swapchain buffers and the frame buffers");
//
//             uint32_t swapchainImageCount;
//             vkGetSwapchainImagesKHR(mDevice, mSwapchain, &swapchainImageCount, nullptr);
//             mSwapchainImages.resize(swapchainImageCount);
//             vkGetSwapchainImagesKHR(mDevice, mSwapchain, &swapchainImageCount, mSwapchainImages.data());
//
//         }
//
//         const VkSurfaceFormatKHR& Swapchain::chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
//         {
//             for (auto &format : availableFormats)
//             {
//                 if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&  format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
//                 {
//
//                     return format;
//                 }
//             }
//
//             return availableFormats[0];
//         }
//
//         const VkPresentModeKHR Swapchain::chooseSwapchainPresentMode(
//             const std::vector<VkPresentModeKHR>& availablePresentModes) const
//         {
//             for (auto &presentMode : availablePresentModes)
//             {
//                 if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
//                 {
//                     return presentMode;
//                 }
//             }
//
//             return VK_PRESENT_MODE_FIFO_KHR;
//         }
//
//         void Swapchain::createSwapchainImageViews() {
//             mSwapchainImageViews.resize(mSwapchainImageViews.size());
//             
// 			for (int i = 0; i < mSwapchainImages.size(); i++)
// 			{
//                 VkImageViewCreateInfo imageViewCreateInfo = {};
// 			    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
// 			    imageViewCreateInfo.image = mSwapchainImages[i];
// 			    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
// 			    imageViewCreateInfo.format = mSwapchainImageFormat;
//
// 			    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//                 imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//                 imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//                 imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//
// 			    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
//                 imageViewCreateInfo.subresourceRange.levelCount = 1;
// 			    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
// 			    imageViewCreateInfo.subresourceRange.layerCount = 1;
// 			    
//                 vkCreateImageView(mDevice, &imageViewCreateInfo, nullptr, &mSwapchainImageViews[i]);
// 			}
//             
//         }
//     }
// }
