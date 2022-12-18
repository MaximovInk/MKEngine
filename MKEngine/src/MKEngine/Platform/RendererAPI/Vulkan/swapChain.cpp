
#include "mkpch.h"

/*
#include "swapChain.h"
#include "MKEngine/Core/Log.h"
#include "presentView.h"

namespace MKEngine {
	void VulkanSwapChain::Connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
	{

	}

	void VulkanSwapChain::Create(uint32_t* width, uint32_t* height, bool vsync, bool fullscreen)
	{
		VkSwapchainKHR oldSwapchain = SwapChain;

		VkSurfaceCapabilitiesKHR surfCaps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfCaps);

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);

		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
				&presentModeCount, presentModes.data());
		}

		VkExtent2D swapchainExtent = {};

		if (surfCaps.currentExtent.width == (uint32_t)-1)
		{
			swapchainExtent.width = *width;
			swapchainExtent.height = *height;
		}
		else
		{
			swapchainExtent = surfCaps.currentExtent;
			*width = surfCaps.currentExtent.width;
			*height = surfCaps.currentExtent.height;
		}

		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		if (!vsync)
		{
			for (size_t i = 0; i < presentModeCount; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}

				if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		uint32_t imageCount = surfCaps.minImageCount + 1;
		if ((surfCaps.maxImageCount > 0)
			&& (imageCount > surfCaps.maxImageCount))
		{
			imageCount = surfCaps.maxImageCount;
		}

		VkSurfaceTransformFlagsKHR preTransform;
		if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			preTransform = surfCaps.currentTransform;

		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags) {
			if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = ColorFormat;
		createInfo.imageColorSpace = ColorSpace;
		createInfo.imageExtent =
		{ swapchainExtent.width, swapchainExtent.height };
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;

		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.presentMode = swapchainPresentMode;
		createInfo.oldSwapchain = oldSwapchain;
		createInfo.compositeAlpha = compositeAlpha;

		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &SwapChain) != VK_SUCCESS) {
			MK_LOG_CRITICAL("failed to create swap chain!");
		}

		if (oldSwapchain != VK_NULL_HANDLE) {
			for (uint32_t i = 0; i < imageCount; i++)
			{
				vkDestroyImageView(device, Buffers[i].view, nullptr);
			}
			vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
		}
		vkGetSwapchainImagesKHR(device, SwapChain, &ImageCount, Images.data());

		Buffers.resize(imageCount);

		for (uint32_t i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.pNext = NULL;
			viewInfo.format = ColorFormat;
			viewInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.flags = 0;

			Buffers[i].image = Images[i];

			viewInfo.image = Buffers[i].image;

			vkCreateImageView(device, &viewInfo, nullptr, &Buffers[i].view);
		}
	}

	VkResult VulkanSwapChain::AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex)
	{
		return vkAcquireNextImageKHR(device, SwapChain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
	}

	VkResult VulkanSwapChain::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &SwapChain;
		presentInfo.pImageIndices = &imageIndex;

		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;
		}
		return vkQueuePresentKHR(queue, &presentInfo);
	}

	void VulkanSwapChain::Cleanup()
	{
		if (SwapChain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < ImageCount; i++)
			{
				vkDestroyImageView(device, Buffers[i].view, nullptr);
			}
		}
		if (surface != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device, SwapChain, nullptr);
			vkDestroySurfaceKHR(instance, surface, nullptr);
		}

		surface = VK_NULL_HANDLE;
		SwapChain = VK_NULL_HANDLE;
	}


}
*/