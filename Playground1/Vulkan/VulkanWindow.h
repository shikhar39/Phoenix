#pragma once

#include "Playground1/Window.h"

namespace PhoenixEngine {
    namespace Vulkan {
        class VulkanWindow : public PhoenixEngine::Window {
            public:
            VulkanWindow(int inWidth, int inHeight, std::string inName);
            ~VulkanWindow() {};
            
            protected:
            void setupHandle() override {};

        };
    }
}

