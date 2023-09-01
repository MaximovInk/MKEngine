#pragma once
#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

namespace MKEngine
{
	struct ImageDescription
	{
		uint32_t Width;
		uint32_t Height;
		VkImageType Type = VK_IMAGE_TYPE_2D;
		VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL;
		uint32_t MipLevels = 1;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkImageLayout InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageUsageFlags Usage = 0;
		VkSharingMode SharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageCreateFlags Flags = 0;
	};

	class Image
	{
	public:
		VkImage Resource;
		VmaAllocation Allocation;

		Image() = default;

		static Image Create(const ImageDescription& description);
		static void Destroy(const Image& image);

		static void CopyBufferToImage(VkBuffer buffer, const Image& image);
		static void TransitionImageLayout(Image image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		ImageDescription GetDescription() const;
	private:
		ImageDescription m_description;
	};
}