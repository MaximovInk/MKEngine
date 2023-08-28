#include "mkpch.h"
#include "buffer.h"

#include "VkContext.h"
#include "vkFunctions.h"
#include "MKEngine/Core/Log.h"

namespace MKEngine {
	Buffer Buffer::Create(const BufferDescription& description)
	{
		Buffer buffer {};

		VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = description.Size;
		bufferInfo.usage = description.Usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		buffer.Size = description.Size;

		if (description.Data && description.Access == DataAccess::Device)
			bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		if (description.Access == DataAccess::Host)
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		vmaCreateBuffer(VkContext::API->VMAAllocator, &bufferInfo,
			&allocationCreateInfo, &buffer.Resource, &buffer.Allocation, nullptr);

		if (description.Access == DataAccess::Host) {
			vmaMapMemory(VkContext::API->VMAAllocator, buffer.Allocation, &buffer.MappedData);

		}
		if (description.Data != nullptr)
		{
			if (description.Access == DataAccess::Host)
			{
				memcpy(buffer.MappedData, description.Data, description.Size);
			}
			else
			{
				BufferDescription stagingDescription;
				stagingDescription.Size = description.Size;
				stagingDescription.Data = description.Data;
				stagingDescription.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingDescription.Access = DataAccess::Host;

				const Buffer stagingBuffer = Create(stagingDescription);

				Copy(stagingBuffer.Resource, buffer.Resource, description.Size);

				Destroy(stagingBuffer);
			}
		}

		return buffer;
	}

	/*
	Buffer Buffer::Create(const BufferDescription& description)
	{
		Buffer buffer {};

		VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = description.Size;
		bufferInfo.usage = description.Usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (description.Data)
			bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		if (description.Access == DataAccess::Host)
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		vmaCreateBuffer(VkContext::API->VMAAllocator, &bufferInfo,
			&allocationCreateInfo, &buffer.Resource, &buffer.Allocation, nullptr);

		if (description.Access == DataAccess::Host) {
			vmaMapMemory(VkContext::API->VMAAllocator, buffer.Allocation, &buffer.MappedData);

		}
		if (description.Data)
		{
			if (description.Access == DataAccess::Host)
			{
				memcpy(buffer.MappedData, description.Data, buffer.Size);
			}
			else
			{
				BufferDescription stagingDescription;
				stagingDescription.Size = description.Size;
				stagingDescription.Data = description.Data;
				stagingDescription.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingDescription.Access = DataAccess::Host;

				const Buffer stagingBuffer = Create(stagingDescription);

				Copy(stagingBuffer.Resource, buffer.Resource, description.Size);

				Destroy(stagingBuffer);
			}
		}

		return buffer;
	}
	 
	 */

    void Buffer::Destroy(const Buffer& buffer)
	{
		WaitDeviceIdle();

		if (buffer.MappedData)
		{
			MK_LOG_INFO("Unmapping data");
			vmaUnmapMemory(VkContext::API->VMAAllocator, buffer.Allocation);
		}

		MK_LOG_INFO("buffer destroyed");
		vmaDestroyBuffer(VkContext::API->VMAAllocator, buffer.Resource, buffer.Allocation);
	}

	void Buffer::Copy(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
	{
		ImmediateSubmit([&](VkCommandBuffer commandBuffer)
			{
				VkBufferCopy copyRegion{};
				copyRegion.srcOffset = 0; // Optional
				copyRegion.dstOffset = 0; // Optional
				copyRegion.size = size;
				vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
			});

		MK_LOG_INFO("Copied buffer");
	}
}
