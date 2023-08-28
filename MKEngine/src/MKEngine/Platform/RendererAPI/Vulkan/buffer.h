#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"

namespace MKEngine {


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

	struct Buffer
	{
		VkDeviceSize Size = 0;
		VkBuffer Resource = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		void* MappedData = nullptr;

		static Buffer Create(const BufferDescription& description);
		static void Destroy(const Buffer& buffer);
		static void Copy(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size);

	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

}