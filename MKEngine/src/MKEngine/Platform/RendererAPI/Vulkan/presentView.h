#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "MKEngine/Platform/Window.h"

namespace MKEngine {
	class VulkanDevice;
	class Window;

	typedef struct _SwapChainBuffers {
		VkImage image;
		VkImageView view;
	} SwapChainBuffer;
	class VulkanPresentView {
	public:
		VkSwapchainKHR SwapChain = VK_NULL_HANDLE;
		VkSurfaceKHR Surface;

		VkColorSpaceKHR ColorSpace;
		VkFormat ColorFormat;
		uint32_t ImageCount;
		std::vector<SwapChainBuffer> Buffers;

		std::vector<VkImage> Images;
		uint32_t QueueNodeIndex = UINT32_MAX;

		VulkanPresentView(VulkanDevice* device);
		~VulkanPresentView();
		void InitSurface(Window* window);
		void CreateSwapChain(uint32_t* width, uint32_t* height, bool vsync = false, bool fullscreen = false);
		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
		void CleanupSwapChain();

	private:
		VulkanDevice* device;
	};
}