#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
	class CommandBuffer
	{
		VkCommandBuffer Resource;

		static CommandBuffer Create();
		static void Destroy(CommandBuffer commandBuffer);


	};

}