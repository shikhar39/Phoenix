#pragma once

#include "EngineChoice.h"
#include "Vulkan/VulkanWindow.h"
#include "DX12/DXWindow.h"

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
# else
    #ifdef _DX12
        DX12::Window window{WIDTH, HEIGHT, "Phoenix"};
    #endif
#endif
        // Device device;
        // Renderer renderer;
    };
}
