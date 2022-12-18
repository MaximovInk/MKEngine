#include "mkpch.h"
#include "presentView.h"
#include "device.h"
#include "vkExtern.h"

namespace MKEngine {
	
	VulkanPresentView::VulkanPresentView(VulkanDevice* device)
	{
		this->device = device;
	}

	VulkanPresentView::~VulkanPresentView()
	{
		CleanupSwapChain();

		vkDestroySurfaceKHR(device->Instance, Surface, nullptr);
	}

	void VulkanPresentView::InitSurface(Window* window)
	{
		Surface = VkExtern::CreateWindowSurface(device->Instance, window);

		uint32_t queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device->PhysicalDevice,
			&queueCount, NULL);
		//assert(queueCount >= 1);
		MK_ASSERT(queueCount >= 1, "queueCount < 1!");

		std::vector<VkQueueFamilyProperties> queueProps(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device->PhysicalDevice,
			&queueCount, queueProps.data());

		std::vector<VkBool32> supportsPresent(queueCount);
		for (uint32_t i = 0; i < queueCount; i++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(device->PhysicalDevice, i,
				Surface, &supportsPresent[i]);
		}

		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < queueCount; i++)
		{
			if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (graphicsQueueNodeIndex == UINT32_MAX)
					graphicsQueueNodeIndex = i;

				if (supportsPresent[i] == VK_TRUE)
				{
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;
					break;
				}
			}
		}
		if (presentQueueNodeIndex == UINT32_MAX)
		{
			for (uint32_t i = 0; i < queueCount; ++i)
			{
				if (supportsPresent[i] == VK_TRUE)
				{
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
		{
			MK_LOG_CRITICAL("Could not find a graphics and/or presenting queue!");
		}
		if (graphicsQueueNodeIndex != presentQueueNodeIndex)
		{
			MK_LOG_CRITICAL(
				"Separate graphics and presenting queues are not supported yet");
		}
		QueueNodeIndex = graphicsQueueNodeIndex;

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->PhysicalDevice,
			Surface, &formatCount, NULL);
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->PhysicalDevice, 
			Surface, &formatCount, surfaceFormats.data());

		if ((formatCount == 1) && 
			(surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
			ColorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					ColorFormat = surfaceFormat.format;
					ColorSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			if (!found_B8G8R8A8_UNORM)
			{
				ColorFormat = surfaceFormats[0].format;
				ColorSpace = surfaceFormats[0].colorSpace;
			}
		}


	}
	
	void VulkanPresentView::CreateSwapChain(uint32_t* width, uint32_t* height,
		bool vsync, bool fullscreen)
	{
		VkSwapchainKHR oldSwapchain = SwapChain;

		VkSurfaceCapabilitiesKHR surfCaps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->PhysicalDevice,
			Surface, &surfCaps);

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device->PhysicalDevice, 
			Surface, &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);

		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device->PhysicalDevice,
				Surface, &presentModeCount, presentModes.data());
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

		uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
		if ((surfCaps.maxImageCount > 0)
			&& (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
		{
			desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
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
		createInfo.surface = Surface;
		createInfo.minImageCount = desiredNumberOfSwapchainImages;
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

		if (vkCreateSwapchainKHR(device->LogicalDevice, &createInfo,
			nullptr, &SwapChain) != VK_SUCCESS) {
			MK_LOG_CRITICAL("failed to create swap chain!");
		}

		if (oldSwapchain != VK_NULL_HANDLE) {
			for (uint32_t i = 0; i < ImageCount; i++)
			{
				vkDestroyImageView(device->LogicalDevice, Buffers[i].view, nullptr);
			}
			vkDestroySwapchainKHR(device->LogicalDevice, oldSwapchain, nullptr);
		}

		vkGetSwapchainImagesKHR(device->LogicalDevice, SwapChain,
			&ImageCount, nullptr);
		Images.resize(ImageCount);
		vkGetSwapchainImagesKHR(device->LogicalDevice, SwapChain, 
			&ImageCount, Images.data());

		Buffers.resize(ImageCount);

		for (uint32_t i = 0; i < ImageCount; i++)
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

			vkCreateImageView(device->LogicalDevice, &viewInfo, nullptr, &Buffers[i].view);
		}
	}

	VkResult VulkanPresentView::AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex)
	{
		return vkAcquireNextImageKHR(device->LogicalDevice, SwapChain, UINT64_MAX,
			presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
	}

	VkResult VulkanPresentView::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
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

	void VulkanPresentView::CleanupSwapChain()
	{
		if (SwapChain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < ImageCount; i++)
			{
				vkDestroyImageView(device->LogicalDevice, Buffers[i].view, nullptr);
			}
		}
		if (Surface != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device->LogicalDevice, SwapChain, nullptr);
			//vkDestroySurfaceKHR(device->Instance, Surface, nullptr);
		}

		//Surface = VK_NULL_HANDLE;
		SwapChain = VK_NULL_HANDLE;
	}
}