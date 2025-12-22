#pragma once

#include "../Window.h"

namespace PhoenixEngine {
namespace Vulkan {
class Window : public PhoenixEngine::Window {
public:
  Window(int inWidth, int inHeight, std::string inName);
  ~Window() {};

  void createSurface(VkInstance &instance, VkSurfaceKHR &surface) const;
  VkExtent2D getSwapchainExtent(VkSurfaceCapabilitiesKHR &capabilities);

protected:
  void setupHandle() override {};
};
} // namespace Vulkan
} // namespace PhoenixEngine
