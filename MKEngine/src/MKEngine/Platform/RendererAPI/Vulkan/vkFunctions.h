#pragma once

#include "buffer.h"
#include "vkTexture.h"
#include "shader.h"

namespace MKEngine
{
		Shader CreateShader(ShaderCreateDescription description);
		void DestroyShader(const Shader& shader);

		VkTexture CreateTexture(const TextureDescription& description);
		void DestroyTexture(VkTexture texture);
		void TransitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage( Buffer buffer, VkTexture image);

        void ImmediateSubmit(std::function<void(VkCommandBuffer)> const& callback);

		VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDevice device);

}
