#pragma once

#include <glfw3.h>
#include <string>

namespace PhoenixEngine {
    class Window {
        public:
        Window(int inWidth, int inHeight, std::string inName);
        virtual ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool shouldClose() const { return glfwWindowShouldClose(window); }
        
        private:
        void init();
        
        int width;
        int height;

        std::string name;

        GLFWwindow* window;

        protected:
        virtual void setupHandle() = 0; // Putting this in to try making the class abstract.
    };
}
