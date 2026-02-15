#pragma once

#include "stdafx.hpp"

#include "VulkanDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace PhoenixEngine {
namespace Vulkan {
class Model {
public:
	struct Vertex {
		glm::vec2 position;

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
	};

	Model(Device&, const std::vector<Vertex>&);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	void bind(VkCommandBuffer commandBuffer);

	void draw(VkCommandBuffer commanbBuffer);
private:
	void createVertexBuffers(const std::vector<Vertex>&);

	Device& mDevice;

	VkBuffer mVertexBuffer;
	VkDeviceMemory mMemory;
	uint32_t mVertexCount;
};
} //namespace Vulkan
} //namespace PhoenixEngine
