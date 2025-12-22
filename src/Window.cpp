#include "Window.h"
#include "stdafx.h"
#include <GLFW/glfw3.h>
#include <memory>

namespace PhoenixEngine {
Window::Window(int inWidth, int inHeight, std::string inName)
    : width(inWidth), height(inHeight), name(inName) {
  init();
}

void Window::init() {
  int maj, min, rev;
  glfwGetVersion(&maj, &min, &rev);

  std::cout << "Glfw version: " << maj << min << rev << std::endl;
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void Window::framebufferSizeCallback(GLFWwindow *inWindow, int width,
                                     int height) {
  auto handle = reinterpret_cast<Window *>(glfwGetWindowUserPointer(inWindow));
  handle->framebufferResized = true;
  spdlog::warn("window resized!");
}
Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
} // namespace PhoenixEngine
