#pragma once
#include <vulkan/vulkan.h>

#include "Image.h"

namespace MKEngine
{

	struct ImageViewDescription
	{
		Image Image;
		bool IsSwapchain = false;
		VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
		VkImageAspectFlags Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	};

	class ImageView
	{
	public:
		VkImageView Resource;
		Image Image;

		static ImageView Create(ImageViewDescription description);
		static void Destroy(ImageView view);
	};
}
