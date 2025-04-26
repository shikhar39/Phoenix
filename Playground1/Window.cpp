#include "stdafx.h"
#include "Window.h"




namespace PhoenixEngine {
    Window::Window(int inWidth, int inHeight, std::string inName) : width(inWidth), height(inHeight), name(inName) {
        init();
    }

    void Window::init() {
        int maj, min, rev;
        glfwGetVersion(&maj, &min, &rev);

        std::cout << "Glfw version: " << maj << min << rev << std::endl;
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    }

    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}
