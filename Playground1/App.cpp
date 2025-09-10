#include "stdafx.h"
#include "App.h"

namespace PhoenixEngine {

    App::App() {
    }
    
    App::~App() {
    }
    
    void App::run() {
        spdlog::info("Starting Phoenix Engine app");
        while (!window.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }
    }

    void App::drawFrame()
    {

    }


}