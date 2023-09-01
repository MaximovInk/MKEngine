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
		VkAttachmentLoadOp LoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		VkClearValue ClearValue;
	};
}