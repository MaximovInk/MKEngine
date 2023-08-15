#pragma once
#include "mkpch.h"
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

	struct BufferDesciption
	{
		uint32_t Size = 0;
		VkBufferUsageFlags Usage = 0;
		void* Data = nullptr;
		DataAccess Access;
	};
}