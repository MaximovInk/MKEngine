#pragma once
#include <vulkan/vulkan.h>

#include "MKEngine/Platform/RendererAPI/Vulkan/buffer.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/DescriptorSet/descriptorSet.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/Image/Image.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/Pipeline/graphicsPipeline.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/Syncronization/Fence.h"

namespace MKEngine
{
	class RenderingInfo;

	struct CommandBufferDescription
	{
		VkCommandPoolCreateFlags Flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		uint32_t FamilyIndex;
		VkCommandBufferLevel Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VkCommandPool CommandPool = VK_NULL_HANDLE;

	};

	class CommandBuffer
	{
	public:
		VkCommandBuffer Resource;
		VkCommandPool CommandPool;
		Fence WaitFence;

		static CommandBuffer Create(const CommandBufferDescription& description);
		static void Destroy(const CommandBuffer& commandBuffer);

		void Begin() const;
		void End() const;
		void Reset() const;
		void WaitForExecute() const;

		void ImageMemoryBarrier(Image image, VkPipelineStageFlags srcStageMask, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout) const;

		static void CopyBufferToImage(VkBuffer buffer, const Image& image);

		void SetViewport(float x, float y, float width, float height, float minDepth = 0, float maxDepth = 1) const;
		void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) const;

		void BeginRendering(const RenderingInfo& renderingInfo) const;
		void EndRendering() const;

		void BindPipeline(GraphicsPipeline pipeline);
		void UnbindPipeline();

		void BindDescriptorSet(const DescriptorSet& descriptorSet) const;

	private:
		bool m_ownerOfPool = false;
		GraphicsPipeline m_currentPipeline;
	};

}
