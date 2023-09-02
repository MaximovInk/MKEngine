#pragma once
#include <vulkan/vulkan.h>

#include "RenderingDepthAttachment.h"
#include "RenderingColorAttachment.h"

namespace MKEngine
{

	class RenderingInfo
	{
	public:
		RenderingInfo(VkRect2D renderArea);

		VkRect2D GetRenderArea() const;

		RenderingColorAttachment& AddColorAttachment(const VkImageView& imageView);
		const std::vector<RenderingColorAttachment>& GetColorAttachmentInfos() const;

		RenderingDepthAttachment& SetDepthAttachment(const VkImageView& imageView);
		bool HasDepthAttachment() const;
		const RenderingDepthAttachment& GetDepthAttachmentInfo() const;

        inline VkRenderingInfoKHR GetRenderingInfo()
        {
            std::vector<VkRenderingAttachmentInfo> colorAttachments;
            colorAttachments.reserve(m_colorAttachmentsInfos.size());

            for (const RenderingColorAttachment& colorAttachmentInfo : m_colorAttachmentsInfos)
            {
                const auto& imageView = colorAttachmentInfo.ImageView;

                VkRenderingAttachmentInfo colorAttachment;
                colorAttachment.imageView = imageView;
                colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                if (colorAttachmentInfo.ResolveMode != VK_RESOLVE_MODE_NONE)
                {
                    colorAttachment.resolveMode = colorAttachmentInfo.ResolveMode;
                    colorAttachment.resolveImageView = colorAttachmentInfo.ResolveImageView;
                    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
                else
                {
                    colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
                    colorAttachment.resolveImageView = nullptr;
                    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                }

                colorAttachment.loadOp = colorAttachmentInfo.LoadOp;
                colorAttachment.storeOp = colorAttachmentInfo.StoreOp;
                colorAttachment.clearValue = colorAttachmentInfo.ClearValue;

                colorAttachments.emplace_back(colorAttachment);
            }

            VkRenderingInfoKHR renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
            renderingInfo.viewMask = 0;
            renderingInfo.renderArea = m_renderArea;
            renderingInfo.layerCount = 1;
            renderingInfo.colorAttachmentCount = colorAttachments.size();
            renderingInfo.pColorAttachments = colorAttachments.data();


            return renderingInfo;
        }

	private:
		VkRect2D m_renderArea;
		std::vector<RenderingColorAttachment> m_colorAttachmentsInfos;
		std::optional<RenderingDepthAttachment> m_depthAttachmentInfo;
		uint32_t m_viewMask = 0;
	};
}
