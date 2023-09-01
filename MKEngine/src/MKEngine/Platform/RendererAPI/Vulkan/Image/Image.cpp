#include "mkpch.h"
#include "Image.h"

#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/vkContext.h"

namespace MKEngine {
    Image Image::Create(const ImageDescription& description)
    {
        Image image;

		VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = description.Type;
		imageInfo.extent.width = static_cast<uint32_t>(description.Width);
		imageInfo.extent.height = static_cast<uint32_t>(description.Height);
		imageInfo.extent.depth = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.mipLevels = description.MipLevels;
		imageInfo.format = description.Format;
		imageInfo.initialLayout = description.InitialLayout;
		imageInfo.usage = description.Usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VmaAllocationCreateInfo imageAllocCreateInfo = {};
		imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		VkResult result = vmaCreateImage(VkContext::API->VmaAllocator, &imageInfo, &imageAllocCreateInfo, &image.Resource, &image.Allocation, nullptr);
		if (result != VK_SUCCESS)
			MK_LOG_ERROR("Failed to create vkImage! {0}", result);

		image.m_description = { description };

        return image;
    }

    void Image::Destroy(const Image& image)
    {
		if (image.Resource != VK_NULL_HANDLE)
			vmaDestroyImage(VkContext::API->VmaAllocator, image.Resource, image.Allocation);
    }

    void Image::CopyBufferToImage(const VkBuffer buffer, const Image& image)
	{
		ImmediateSubmit([&](const VkCommandBuffer commandBuffer)
			{
				VkBufferImageCopy region{};
				region.bufferOffset = 0;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;

				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel = 0;
				region.imageSubresource.baseArrayLayer = 0;
				region.imageSubresource.layerCount = 1;

				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = {
					image.GetDescription().Width,
					image.GetDescription().Height,
					1
				};

				vkCmdCopyBufferToImage(
					commandBuffer,
					buffer,
					image.Resource,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region
				);
			});

	}

    void Image::TransitionImageLayout(const Image image, VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout)
	{
		ImmediateSubmit([&](const VkCommandBuffer commandBuffer)
			{
				VkPipelineStageFlags sourceStage;
				VkPipelineStageFlags destinationStage;

				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = oldLayout;
				barrier.newLayout = newLayout;

				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

				barrier.image = image.Resource;
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;

				if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

					sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				}
				else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
					destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else {
					MK_LOG_ERROR("unsupported layout transition!");
				}

				vkCmdPipelineBarrier(
					commandBuffer,
					sourceStage, destinationStage,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);
			});

	}

    ImageDescription Image::GetDescription() const
	{
		return m_description;
	}
}
