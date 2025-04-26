#pragma once

#include "Playground1/Window.h"

namespace PhoenixEngine {
    namespace Vulkan {
        class Window : public PhoenixEngine::Window {
            public:
            Window(int inWidth, int inHeight, std::string inName);
            ~Window() {};
            
            protected:
            void setupHandle() override {};

        };
    }
}

