#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{

	struct ImageViewDescription
	{
		VkImage Image;
		bool IsSwapchain;
		uint32_t Width;
		uint32_t Height;
		VkImageType Type = VK_IMAGE_TYPE_2D;

	};

	class ImageView
	{
		VkImageView View;
		VkImage Image;

		static ImageView CreateImageView(ImageViewDescription description);
		static void DestroyImageView(ImageView view);
	};
}