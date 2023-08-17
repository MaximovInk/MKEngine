#pragma once
#include "mkpch.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

namespace MKEngine {
	struct Buffer
	{
		VkDeviceSize Size = 0;
		VkDeviceMemory Memory = VK_NULL_HANDLE;
		VkBuffer Resource = VK_NULL_HANDLE;
		void* MappedData = nullptr;
	};

	enum DataAccess
	{
		Host,
		Device
	};

	struct BufferDescription
	{
		uint32_t Size = 0;
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