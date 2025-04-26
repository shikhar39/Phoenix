#pragma once

#include "Vulkan/Window.h"
// #include "Device.h"
// #include "Renderer.h"

namespace PhoenixEngine {
    class App {

        public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;


        App();
        ~App();

        App(const App&) = delete;
        App& operator=(const App&) = delete;

        void run();
        private:		

        Vulkan::Window window{WIDTH, HEIGHT, "Phoenix"};
        // Device device;
        // Renderer renderer;
    };
}
