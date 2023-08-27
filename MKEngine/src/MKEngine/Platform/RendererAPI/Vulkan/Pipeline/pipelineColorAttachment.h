#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
	struct PipelineBlendingInfo
	{
		VkBlendFactor SrcColorBlendFactor;
		VkBlendFactor DstColorBlendFactor;
		VkBlendOp ColorBlendOp;
		VkBlendFactor SrcAlphaBlendFactor;
		VkBlendFactor DstAlphaBlendFactor;
		VkBlendOp AlphaBlendOp;
	};

	struct ColorAttachmentInfo
	{
		VkFormat Format;
		std::optional<PipelineBlendingInfo> Blending;
	};

	struct DepthAttachmentInfo
	{
		VkFormat Format;
		VkCompareOp DepthTestPassCondition;
		bool WriteEnabled;
	};

	class PipelineColorAttachment
	{
	public:
		const ColorAttachmentInfo& GetColorAttachmentInfo(uint32_t attachmentLocation) const;
		const std::vector<ColorAttachmentInfo>& GetColorAttachmentsInfos() const;
		void AddColorAttachment(VkFormat format, const std::optional<PipelineBlendingInfo>& blending = std::nullopt);

		bool HasDepthAttachment() const;
		const DepthAttachmentInfo& GetDepthAttachmentInfo() const;
		void SetDepthAttachment(VkFormat format, VkCompareOp depthTestPassCondition, bool writeEnabled);

	private:
		std::vector<ColorAttachmentInfo> m_colorAttachmentsInfos;
		std::optional<DepthAttachmentInfo> m_depthAttachmentInfo;
	};

}