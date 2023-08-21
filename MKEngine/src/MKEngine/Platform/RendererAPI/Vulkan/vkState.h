#pragma once
#include <SDL_vulkan.h>
#include "vulkan/vulkan.h"

namespace MKEngine
{
	class vkState
	{
	public:
		VkInstance Instance = nullptr;
		VkPhysicalDevice PhysicalDevice = nullptr;
		VkDevice LogicalDevice = nullptr;

		VkCommandPool CommandPool = nullptr;
		VkCommandBuffer MainCommandBuffer = nullptr;

		VkPipeline GraphicsPipeline = nullptr;
		VkRenderPass RenderPass = nullptr;
		VkPipelineLayout PipelineLayout = nullptr;

		VkQueue GraphicsQueue{};
		VkQueue PresentQueue{};

		VkPhysicalDeviceProperties Properties;

		struct
		{
			uint32_t Graphics;
			uint32_t Compute;
			uint32_t Transfer;
			uint32_t Present;
		} QueueFamilyIndices;
		VkDescriptorSetLayout DescriptorSetLayout;

		static vkState* API;
	};

	void WaitDeviceIdle();

}
