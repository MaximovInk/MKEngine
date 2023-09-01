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

	private:;
		VkRect2D m_renderArea;
		std::vector<RenderingColorAttachment> m_colorAttachmentsInfos;
		std::optional<RenderingDepthAttachment> m_depthAttachmentInfo;
		uint32_t m_viewMask = 0;
	};
}
