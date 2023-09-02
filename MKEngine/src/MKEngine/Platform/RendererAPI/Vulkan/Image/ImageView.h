#pragma once
#include <vulkan/vulkan.h>

#include "Image.h"

namespace MKEngine
{

	struct ImageViewDescription
	{
		Image Image;
		bool IsSwapchain;
		VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
	};

	class ImageView
	{
	public:
		VkImageView Resource;
		Image Image;

		static ImageView CreateImageView(ImageViewDescription description);
		static void DestroyImageView(ImageView view);
	};
}
