#pragma once
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "Image/Image.h"
//#include "buffer.h"

namespace MKEngine
{
	struct SamplerDesc
	{
		VkFilter FilterMode = VK_FILTER_LINEAR;
		VkSamplerReductionMode ReductionMode = VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
		VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkSamplerMipmapMode MipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	};

	struct TextureDescription
	{
		//Load from file
		const char* Path = "";
		//OR set data manually
		void* Data = nullptr;
		uint32_t Width;
		uint32_t Height;

		uint32_t MipCount = 1;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkImageUsageFlags Usage = 0;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	struct Buffer;

	struct Texture
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t MipCount = 1;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkSampler Sampler = VK_NULL_HANDLE;
		//VkImage Resource = VK_NULL_HANDLE;
		Image Image;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;

		static Texture CreateTexture(const TextureDescription& description);
		static void DestroyTexture(Texture texture);
	};

	

};
