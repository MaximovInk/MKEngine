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

		if (vmaCreateImage(VkContext::API->VmaAllocator, &imageInfo, &imageAllocCreateInfo, &image.Resource, &image.Allocation, nullptr))
			MK_LOG_ERROR("Failed to create vkImage!");

		image.m_description = { description };
		image.m_layout = description.InitialLayout;
		image.m_aspectFlags = VK_IMAGE_ASPECT_NONE;

        return image;
    }

    void Image::Destroy(const Image& image)
    {
		if (image.Resource != VK_NULL_HANDLE)
			vmaDestroyImage(VkContext::API->VmaAllocator, image.Resource, image.Allocation);
    }

	Image Image::Create(const VkImage source, const VkImageAspectFlags aspectFlags)
	{
		Image image;

		image.Resource = source;
		image.m_aspectFlags = aspectFlags;

		return image;
	}

	bool HasStencilComponent(const VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void Image::TransitionImageLayout(const VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout)
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

				barrier.image = Resource;
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
				else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

					sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				}
				else {
					MK_LOG_ERROR("unsupported layout transition!");
				}


				if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
					barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

					if (HasStencilComponent(format)) {
						barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
					}
				}
				else {
					barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				}

				m_aspectFlags = barrier.subresourceRange.aspectMask;

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

    VkImageAspectFlags Image::GetAspectFlags() const
    {
		return m_aspectFlags;
    }

    VkImageLayout Image::GetLayout() const
    {
		return m_layout;
    }

    void Image::SetLayout(const VkImageLayout newLayout)
    {
		m_layout = newLayout;
    }
}
