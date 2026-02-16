#pragma once

#include "Vulkan/VulkanDevice.hpp"
#include "Vulkan/VulkanWindow.hpp"
#include "Vulkan/VulkanRenderer.hpp"

namespace PhoenixEngine {
class App {
public:
	static constexpr int WIDTH =
		800;  // Should we make width and height a property of the window rather
			  // than the app
	static constexpr int HEIGHT = 600;

	uint32_t currentFrameIndex = 0;

	App();
	~App();

	App(const App&) = delete;
	App& operator=(const App&) = delete;

	void run();
private:
	void createPipelineLayout();
	void createPipeline();
	
	Vulkan::Window mWindow{WIDTH, HEIGHT, "Phoenix"};
	Vulkan::Device mDevice{mWindow};
	Vulkan::Renderer mRenderer{mWindow, mDevice};

	// I'd say these are temporary.
	VkPipelineLayout mPipelineLayout{VK_NULL_HANDLE};
	VkPipeline mPipeline{VK_NULL_HANDLE};
};
}  // namespace PhoenixEngine
