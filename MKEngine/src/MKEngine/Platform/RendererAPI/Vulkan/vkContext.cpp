#include "mkpch.h"
#include "VkContext.h"
#include "MKEngine/Core/Log.h"

namespace MKEngine {
	VkContext* VkContext::API;

	void ImmediateSubmit(std::function<void(VkCommandBuffer)> const& callback)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VkContext::API->CommandBuffer.CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(VkContext::API->LogicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to allocate command buffer for immediate submit!");
		}

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		callback(commandBuffer);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(VkContext::API->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(VkContext::API->GraphicsQueue);

		vkFreeCommandBuffers(VkContext::API->LogicalDevice, VkContext::API->CommandBuffer.CommandPool, 1, &commandBuffer);
	}

	void WaitDeviceIdle()
	{
		vkDeviceWaitIdle(VkContext::API->LogicalDevice);
	}
}
