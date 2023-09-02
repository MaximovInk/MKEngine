#include "mkpch.h"
#include "RenderingInfo.h"

namespace MKEngine {
	RenderingInfo::RenderingInfo(const VkRect2D renderArea) : m_renderArea(renderArea)
	{

	}

	VkRect2D RenderingInfo::GetRenderArea() const
    {
        return m_renderArea;
    }

    RenderingColorAttachment& RenderingInfo::AddColorAttachment(const VkImageView& imageView)
    {
        return m_colorAttachmentsInfos.emplace_back(
            RenderingColorAttachment{ .ImageView = imageView });
    }

    const std::vector<RenderingColorAttachment>& RenderingInfo::GetColorAttachmentInfos() const
    {
        return m_colorAttachmentsInfos;
    }

    RenderingDepthAttachment& RenderingInfo::SetDepthAttachment(const VkImageView& imageView)
    {
        m_depthAttachmentInfo = std::make_optional<RenderingDepthAttachment>(
            RenderingDepthAttachment{ .ImageView = imageView });
        return m_depthAttachmentInfo.value();
    }

    bool RenderingInfo::HasDepthAttachment() const
    {
        return m_depthAttachmentInfo.has_value();
    }

    const RenderingDepthAttachment& RenderingInfo::GetDepthAttachmentInfo() const
    {
        return m_depthAttachmentInfo.value();
    }

   
}
