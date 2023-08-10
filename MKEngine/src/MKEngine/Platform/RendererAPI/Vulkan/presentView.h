#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "MKEngine/Platform/Window.h"

namespace MKEngine {

	class VulkanDevice;
	class Window;

	struct ViewSync
	{
		VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
		VkFence inFlightFence = VK_NULL_HANDLE;
	};

	typedef struct _SwapChainBuffers {
		VkImage image;
		VkImageView view;
		VkFramebuffer framebuffer;
		VkCommandBuffer commandBuffer;
		ViewSync sync;
	} SwapChainBuffer;

	class VulkanPresentView {
	public:
		VkSwapchainKHR SwapChain = VK_NULL_HANDLE;
		VkSurfaceKHR Surface;

		VkColorSpaceKHR ColorSpace;
		VkFormat ColorFormat;
		uint32_t ImageCount;
		std::vector<SwapChainBuffer> Buffers;
		VkExtent2D SwapChainExtent;
		//ViewSync Sync;
		int maxFramesInFlight, frameNumber;


		//std::vector<VkImage> Images;


		VulkanPresentView(VulkanDevice* device);
		~VulkanPresentView();
		void InitSurface(Window* window);
		void CreateSwapChain(uint32_t* width, uint32_t* height, bool vsync = false, bool fullscreen = false);
		void CreateFramebuffer();
		void CreateCommandBuffers();
		void CreateSync();

		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
		
		
		void CleanupSwapChain();

		void RecordDrawCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void Render();

	private:
		VulkanDevice* device;
	};
}