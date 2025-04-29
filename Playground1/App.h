#pragma once

#include "Vulkan/VulkanWindow.h"
// #include "Device.h"
// #include "Renderer.h"

namespace PhoenixEngine {
    class App {

        public:
        static constexpr int WIDTH = 800; // Should we make width and height a property of the window rather than the app 
        static constexpr int HEIGHT = 600;


        App();
        ~App();

        App(const App&) = delete;
        App& operator=(const App&) = delete;

        void run();
        private:		

#ifdef _VK
        Vulkan::Window window{WIDTH, HEIGHT, "Phoenix"};
#elif _DX12
        DX12::Window window2{WIDTH, HEIGHT, "Phoenix"};        
#endif
        // Device device;
        // Renderer renderer;
    };
}
