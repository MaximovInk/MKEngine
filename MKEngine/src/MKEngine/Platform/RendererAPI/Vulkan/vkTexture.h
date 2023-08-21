#pragma once
#include <vulkan/vulkan_core.h>

namespace MKEngine
{
	struct VkTexture
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t MipCount = 1;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkSampler Sampler = VK_NULL_HANDLE;
		VkImage Resource = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
	};

	struct SamplerDesc
	{
		VkFilter FilterMode = VK_FILTER_LINEAR;                                             
		VkSamplerReductionMode ReductionMode = VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;  
		VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;           
		VkSamplerMipmapMode MipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;                    
	};

	struct TextureDescription
	{
		const char* Path = "";                              
		uint32_t MipCount = 1;                               
		VkFormat Format = VK_FORMAT_UNDEFINED;             
		VkImageUsageFlags Usage = 0;                     
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED; 
	};

};
