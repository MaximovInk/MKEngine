#include "mkpch.h"

#include "CommandBuffer.h"

#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/VkContext.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/vkExtern.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/Rendering/RenderingInfo.h"

namespace MKEngine
{
	CommandBuffer CommandBuffer::Create(const CommandBufferDescription& description)
	{
		CommandBuffer commandBuffer;

		if(description.CommandPool == VK_NULL_HANDLE)
		{
			VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			commandPoolInfo.queueFamilyIndex = description.FamilyIndex;
			commandPoolInfo.flags = description.Flags;

			if (vkCreateCommandPool(VkContext::API->LogicalDevice, &commandPoolInfo, nullptr, &commandBuffer.CommandPool) != VK_SUCCESS)
				MK_LOG_CRITICAL("Failed to create command pool");
			commandBuffer.m_ownerOfPool = true;
		}else
		{
			commandBuffer.m_ownerOfPool = false;
			commandBuffer.CommandPool = description.CommandPool;
		}

		VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocInfo.commandPool = commandBuffer.CommandPool;
		allocInfo.level = description.Level;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(VkContext::API->LogicalDevice, &allocInfo, &commandBuffer.Resource) != VK_SUCCESS)
			MK_LOG_ERROR("failed to allocate command buffer!");

		VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		commandBuffer.WaitFence = Fence::Create(createInfo);

		return commandBuffer;
	}

	void CommandBuffer::Destroy(const CommandBuffer& commandBuffer)
	{
		if(commandBuffer.m_ownerOfPool)
			vkDestroyCommandPool(VkContext::API->LogicalDevice, commandBuffer.CommandPool, nullptr);
		else
			vkFreeCommandBuffers(VkContext::API->LogicalDevice, commandBuffer.CommandPool, 1, &commandBuffer.Resource);

		Fence::Destroy(commandBuffer.WaitFence);
	}

	void CommandBuffer::Begin() const
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(Resource, &beginInfo) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to begin recording command buffer!");
		}
	}

	void CommandBuffer::End() const
	{
		if (vkEndCommandBuffer(Resource) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to record command buffer!");
		}
	}

	void CommandBuffer::Reset() const
	{
		vkResetCommandBuffer(Resource, 0);
	}

	void CommandBuffer::WaitForExecute() const
	{
		WaitFence.Wait();
	}

	void CommandBuffer::ImageMemoryBarrier(Image image,
	                                       const VkPipelineStageFlags srcStageMask, const VkAccessFlags srcAccessMask, const VkPipelineStageFlags dstStageMask,
	                                       const VkAccessFlags dstAccessMask,const VkImageLayout oldImageLayout, const VkImageLayout newImageLayout) const
	{
		VkImageMemoryBarrier2 imageMemoryBarrier { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		imageMemoryBarrier.srcStageMask = srcStageMask;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstStageMask = dstStageMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image.Resource;
		imageMemoryBarrier.subresourceRange.aspectMask = image.GetAspectFlags();
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

		VkDependencyInfo dependencyInfo { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

		vkCmdPipelineBarrier2(Resource, &dependencyInfo);

		image.SetLayout(newImageLayout);
	}

	void CommandBuffer::CopyBufferToImage(const VkBuffer buffer, const Image& image)
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

	void CommandBuffer::SetViewport(const float x, const float y, const float width, const float height, const float minDepth, const float maxDepth) const
	{
		const VkViewport viewport{ x, y, width, height, minDepth, maxDepth };
		vkCmdSetViewport(Resource, 0, 1, &viewport);
	}

	void CommandBuffer::SetScissor(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) const
	{
		const VkRect2D scissor{ x, y, width, height };
		vkCmdSetScissor(Resource, 0, 1, &scissor);
	}

	void CommandBuffer::BeginRendering(const RenderingInfo& _wip) const
	{
		VkClearValue colorClear;
		colorClear.color = VkClearColorValue({ 0, 0, 0, 0 });

		VkClearValue depthClear;
		depthClear.depthStencil = VkClearDepthStencilValue({ 1.0f, 0 });

		VkRenderingInfoKHR createInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };

		std::vector<VkRenderingAttachmentInfo> colorAttachments;
		colorAttachments.reserve(_wip.GetColorAttachmentInfos().size());

		createInfo.viewMask = 0;
		createInfo.renderArea = _wip.GetRenderArea();
		createInfo.layerCount = 1;

		for (auto attachmentCreateInfo : _wip.GetColorAttachmentInfos())
		{
			VkRenderingAttachmentInfo colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
			colorAttachment.imageView = attachmentCreateInfo.ImageView;
			colorAttachment.imageLayout = attachmentCreateInfo.Layout;
			colorAttachment.loadOp = attachmentCreateInfo.LoadOp;
			colorAttachment.storeOp = attachmentCreateInfo.StoreOp;
			colorAttachment.clearValue = colorClear;

			colorAttachments.emplace_back(colorAttachment);
		}

		createInfo.colorAttachmentCount = colorAttachments.size();
		createInfo.pColorAttachments = colorAttachments.data();

		if(_wip.HasDepthAttachment())
		{
			const auto& depthAttachmentInfo = _wip.GetDepthAttachmentInfo();

			VkRenderingAttachmentInfo depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
			depthAttachment.imageView = depthAttachmentInfo.ImageView;
			depthAttachment.loadOp = depthAttachmentInfo.LoadOp;
			depthAttachment.storeOp = depthAttachmentInfo.StoreOp;
			depthAttachment.imageLayout = depthAttachmentInfo.Layout;
			depthAttachment.clearValue = depthClear;

			createInfo.pDepthAttachment = &depthAttachment;
			createInfo.pStencilAttachment = &depthAttachment;
		}else
		{
			createInfo.pDepthAttachment = VK_NULL_HANDLE;
			createInfo.pStencilAttachment = VK_NULL_HANDLE;
		}

		VkContext::API->Begin(Resource, &createInfo);
	}

	void CommandBuffer::EndRendering() const
	{
		VkContext::API->End(Resource);
	}

	void CommandBuffer::BindPipeline(const GraphicsPipeline pipeline)
	{
		vkCmdBindPipeline(Resource, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Resource);
		m_currentPipeline = pipeline;
	}

	void CommandBuffer::UnbindPipeline()
	{
		m_currentPipeline = {};
	}

	void CommandBuffer::BindDescriptorSet(const DescriptorSet& descriptorSet) const
	{
		vkCmdBindDescriptorSets(
			Resource,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_currentPipeline.PipelineLayout,
			0,
			1,
			&descriptorSet.Resource,
			0,
			nullptr);
	}

}

