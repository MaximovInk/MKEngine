#include "mkpch.h"
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "vk_mem_alloc.h"

#include "MKEngine/Core/Log.h"
#include "buffer.h"
#include "VkContext.h"

namespace MKEngine {
    Texture Texture::CreateTexture(const TextureDescription& description)
    {

		MK_LOG_INFO("CREATING TEXTURE");
		//Load from file
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(description.Path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		const VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			MK_LOG_ERROR("failed to load texture image!");
		}

		//Create staging buffer
		BufferDescription stagingBufferDescription{};
		stagingBufferDescription.Access = ACCESS_HOST;
		stagingBufferDescription.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferDescription.Data = pixels;
		stagingBufferDescription.Size = imageSize;
		const Buffer stagingBuffer = Buffer::Create(stagingBufferDescription);
		//Free stbi image 
		stbi_image_free(pixels);

		//Create image
		Texture texture;
		texture.Width = texWidth;
		texture.Height = texHeight;

		ImageDescription imageDescription{};
		imageDescription.Format = description.Format;
		imageDescription.InitialLayout = description.Layout;
		imageDescription.Usage = description.Usage;
		imageDescription.MipLevels = description.MipCount;
		imageDescription.Width = static_cast<uint32_t>(texWidth);
		imageDescription.Height = static_cast<uint32_t>(texHeight);

		texture.Image = Image::Create(imageDescription);

		//Set TransitionLayout for copy
		Image::TransitionImageLayout(texture.Image, description.Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		//Copy from buffer to image
		Image::CopyBufferToImage(stagingBuffer.Resource, texture.Image);

		//Set TransitionLayout for shaders 
		Image::TransitionImageLayout(texture.Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		Buffer::Destroy(stagingBuffer);

		//Create ImageView
		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = texture.Image.Resource;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(VkContext::API->LogicalDevice, &viewInfo, nullptr, &texture.View) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to create texture image view!");
		}

		//Create Sampler
		VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = VkContext::API->Properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(VkContext::API->LogicalDevice, &samplerInfo, nullptr, &texture.Sampler) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to create texture sampler!");
		}

		return texture;
    }

	void Texture::DestroyTexture(Texture texture)
	{
		if (texture.Sampler != VK_NULL_HANDLE)
			vkDestroySampler(VkContext::API->LogicalDevice, texture.Sampler, nullptr);
		if (texture.View != VK_NULL_HANDLE)
			vkDestroyImageView(VkContext::API->LogicalDevice, texture.View, nullptr);
		if (texture.Image.Resource != VK_NULL_HANDLE)
			Image::Destroy(texture.Image);
	}



}
