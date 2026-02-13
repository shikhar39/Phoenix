#include "stdafx.hpp"

#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "SwapChain.hpp"

#include <memory>
#include <cstdint>
#include <filesystem>

namespace PhoenixEngine {
namespace Vulkan {
class Renderer {
public:
	Renderer(Window&, Device&);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer &operator=(const Renderer&) = delete;

	void drawFrame();
	VkCommandBuffer getCurrentCommandBuffer() const {
		return mCommandBuffers[currentFrameIndex];
  }
private:
	void createCommandBuffers();
	void freeCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer&, uint32_t imageIndex) const;
	void recreateSwapChain();

	Window& mWindow;
	Device& mDevice;
	std::unique_ptr<SwapChain> mSwapChain;

	//temporary
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;

	//temporary
	void createGraphicsPipeline();
	std::vector<char> readFile(const std::filesystem::path& relative);

	VkShaderModule createShaderModule(const std::vector<char>& code) const;
	std::vector<VkCommandBuffer> mCommandBuffers;

	uint32_t mCurrentImageIndex;
	uint32_t currentFrameIndex{0};
};

}
}
