#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
	struct RenderingDepthAttachment
	{
	public:
		VkImageView ImageView;
		VkImageView ResolveImageView;
		VkResolveModeFlagBits ResolveMode = VK_RESOLVE_MODE_NONE;
		VkAttachmentLoadOp LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	};
}