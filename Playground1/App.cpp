#include "stdafx.h"
#include "App.h"

namespace PhoenixEngine {

    App::App() {
    }
    
    App::~App() {
    }
    
    void App::run() {
        while (!window.shouldClose()) {
            glfwPollEvents();
        }
    }
}