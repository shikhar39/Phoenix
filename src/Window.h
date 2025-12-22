#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace PhoenixEngine {
class Window {
public:
  Window(int inWidth, int inHeight, std::string inName);
  virtual ~Window();

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  bool shouldClose() const { return glfwWindowShouldClose(window); }

  bool wasframebufferResized() { return framebufferResized; }
  void resetFramebufferResized() { framebufferResized = false; }

  GLFWwindow *get() { return window; }

private:
  virtual void init();

  int width;
  int height;

  std::string name;

protected:
  GLFWwindow *window;
  bool framebufferResized = false;
  virtual void
  setupHandle() = 0; // Putting this in to try making the class abstract.
  static void framebufferSizeCallback(GLFWwindow *window, int width,
                                      int height);
};
} // namespace PhoenixEngine
