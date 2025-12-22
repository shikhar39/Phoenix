#pragma once

#include "EngineChoice.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanWindow.h"

namespace PhoenixEngine {
class App {

public:
  static constexpr int WIDTH =
      800; // Should we make width and height a property of the window rather
           // than the app
  static constexpr int HEIGHT = 600;

  uint32_t currentFrameIndex = 0;

  App();
  ~App();

  App(const App &) = delete;
  App &operator=(const App &) = delete;

  void run();
  void drawFrame();

private:
  Vulkan::Window window{WIDTH, HEIGHT, "Phoenix"};
  Vulkan::Device device{window};
};
} // namespace PhoenixEngine
