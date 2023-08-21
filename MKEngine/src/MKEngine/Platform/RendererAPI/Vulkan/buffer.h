#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

namespace MKEngine {
	struct Buffer
	{
		VkDeviceSize Size = 0;
		VkBuffer Resource = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		void* MappedData = nullptr;
	};

	enum DataAccess
	{
		Host,
		Device
	};

	struct BufferDescription
	{
		VkDeviceSize Size = 0;
		VkBufferUsageFlags Usage = 0;
		void* Data = nullptr;
		DataAccess Access;
	};


	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

}