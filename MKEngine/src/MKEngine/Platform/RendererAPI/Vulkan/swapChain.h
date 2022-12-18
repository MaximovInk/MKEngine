/*
#pragma once
#include <vector>
#include "vulkan/vulkan.h"

namespace MKEngine {
	typedef struct _SwapChainBuffers {
		VkImage image;
		VkImageView view;
	} SwapChainBuffer;

	class VulkanSwapChain {
	public:
		VkSwapchainKHR SwapChain = VK_NULL_HANDLE;

		VkColorSpaceKHR ColorSpace;
		VkFormat ColorFormat;
		uint32_t ImageCount;
		std::vector<VkImage> Images;
		std::vector<SwapChainBuffer> Buffers;

		uint32_t QueueNodeIndex = UINT32_MAX;

		void Connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
		void Create(uint32_t* width, uint32_t* height, bool vsync = false, bool fullscreen = false);
		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
		void Cleanup();
	private:
		VkInstance instance;
		VkDevice device;
		VkPhysicalDevice physicalDevice;
		VkSurfaceKHR surface;
	};
}
*/