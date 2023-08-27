#include "mkpch.h"
#include "pipelineColorAttachment.h"

namespace MKEngine {

    const ColorAttachmentInfo& PipelineColorAttachment::GetColorAttachmentInfo(uint32_t attachmentLocation) const
    {
        return m_colorAttachmentsInfos[attachmentLocation];
    }

    const std::vector<ColorAttachmentInfo>& PipelineColorAttachment::GetColorAttachmentsInfos() const
    {
        return m_colorAttachmentsInfos;
    }

    void PipelineColorAttachment::AddColorAttachment(const VkFormat format, const std::optional<PipelineBlendingInfo>& blending)
    {
        ColorAttachmentInfo colorAttachmentInfo;
        colorAttachmentInfo.Format = format;
        colorAttachmentInfo.Blending = blending;
        m_colorAttachmentsInfos.emplace_back(colorAttachmentInfo);
    }

    bool PipelineColorAttachment::HasDepthAttachment() const
    {
        return m_depthAttachmentInfo.has_value();
    }
    const DepthAttachmentInfo& PipelineColorAttachment::GetDepthAttachmentInfo() const
    {
        return m_depthAttachmentInfo.value();
    }

    void PipelineColorAttachment::SetDepthAttachment(const VkFormat format, const VkCompareOp depthTestPassCondition, const bool writeEnabled)
    {
        DepthAttachmentInfo depthInfo;
        depthInfo.Format = format;
        depthInfo.DepthTestPassCondition = depthTestPassCondition;
        depthInfo.WriteEnabled = writeEnabled;
        m_depthAttachmentInfo = depthInfo;
    }
}