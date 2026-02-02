#include "stdafx.hpp"

#include "VulkanWindow.hpp"

#include <spdlog/spdlog.h>

namespace PhoenixEngine {
namespace Vulkan {
Window::Window(int inWidth, int inHeight, std::string inName)
	: PhoenixEngine::Window(inWidth, inHeight, inName) {
}

void Window::createSurface(VkInstance& instance, VkSurfaceKHR& surface) const {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
		VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
}
}  // namespace Vulkan
}  // namespace PhoenixEngine
