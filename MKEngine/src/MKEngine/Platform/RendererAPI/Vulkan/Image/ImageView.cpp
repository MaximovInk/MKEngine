#include "mkpch.h"

#include "ImageView.h"

#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/vkContext.h"

namespace MKEngine {
    ImageView ImageView::CreateImageView(ImageViewDescription description)
    {
        ImageView imageView;
		imageView.Image = description.Image;


		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.pNext = nullptr;
		viewInfo.format = description.Format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.image = description.Image.Resource;

        if(description.IsSwapchain)
        {

			viewInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
        }
		
		if (vkCreateImageView(VkContext::API->LogicalDevice, &viewInfo, nullptr, &imageView.Resource) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to create texture image view!");
		}
        return imageView;
    }

    void ImageView::DestroyImageView(ImageView view)
    {
		vkDestroyImageView(VkContext::API->LogicalDevice, view.Resource, nullptr);
    }
}
