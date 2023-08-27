#pragma once

#include "buffer.h"
#include "vkTexture.h"
#include "shader.h"

namespace MKEngine
{
		Buffer CreateBuffer(const BufferDescription& description);
		void DestroyBuffer(const Buffer& buffer);
		void CopyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		Shader CreateShader(ShaderCreateDescription description);
		void DestroyShader(const Shader& shader);

		VkTexture CreateTexture(const TextureDescription& description);
		void DestroyTexture(VkTexture texture);
		void TransitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage( Buffer buffer, VkTexture image);
        
        VkVertexInputBindingDescription GetBindingDescription();

		std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();

        void ImmediateSubmit(std::function<void(VkCommandBuffer)> const& callback);

		VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDevice device);

}
