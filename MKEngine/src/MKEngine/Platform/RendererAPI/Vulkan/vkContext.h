#pragma once
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"
#include "DescriptorSet/descriptorSet.h"
#include "CommandBuffer/commandBuffer.h"

namespace MKEngine
{
	class VkContext
	{
	public:
		VkInstance Instance = nullptr;
		VkPhysicalDevice PhysicalDevice = nullptr;
		VkDevice LogicalDevice = nullptr;

		CommandBuffer CommandBuffer;

		GraphicsPipeline GraphicsPipeline;

		VkQueue GraphicsQueue{};
		VkQueue PresentQueue{};

		VkPhysicalDeviceProperties Properties;
		VmaAllocator VmaAllocator;

		struct
		{
			uint32_t Graphics;
			uint32_t Compute;
			uint32_t Transfer;
			uint32_t Present;
		} QueueFamilyIndices;
		DescriptorSetLayout DescriptorSetLayout;

		static VkContext* API;

		PFN_vkCmdBeginRendering Begin;
		PFN_vkCmdEndRendering End;
	};

	void ImmediateSubmit(std::function<void(VkCommandBuffer)> const& callback);

	void WaitDeviceIdle();

}
