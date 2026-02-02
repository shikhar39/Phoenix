#include "stdafx.hpp"

#include "App.hpp"

namespace PhoenixEngine {
App::App() {
}

App::~App() {
}

void App::run() {
	spdlog::info("Starting Phoenix Engine app");
	while (!mWindow.shouldClose()) {
		glfwPollEvents();
		mRenderer.drawFrame();
	}

	vkDeviceWaitIdle(mDevice.get());
}
}  // namespace PhoenixEngine
