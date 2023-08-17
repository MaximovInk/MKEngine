#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "MKEngine/Platform/Window.h"
#include "../vertex.h"
#include "buffer.h"

namespace MKEngine {

	class VulkanDevice;
	class Window;

	struct ViewSync
	{
		VkSemaphore ImageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;
		VkFence InFlightFence = VK_NULL_HANDLE;
	};

	struct SwapChainInput {
		uint32_t Width; 
		uint32_t Height;
		bool Vsync;
		bool Fullscreen;
	};

	typedef struct SwapChainBuffers {
		VkImage Image;
		VkImageView View;
		VkFramebuffer FrameBuffer;
		VkCommandBuffer CommandBuffer;
		ViewSync Sync;
		Buffer UniformBuffer;
		VkDescriptorSet DescriptorSet;

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
		int MaxFramesInFlight, FrameNumber;

		//Test
		std::vector<Buffer> UniformBuffers;
		VkDescriptorPool DescriptorPool;

		explicit VulkanPresentView(VulkanDevice* device);
		~VulkanPresentView();
		void InitSurface(Window* window);
		void CreateSwapChain();
		void FinalizeCreation();

		void RecreateSwapChain();

		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const;
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE) const;

		void BeginRender();
		void EndRender();

		/*TEST*/

		void Record(int testIndex);

	private:
		VulkanDevice* m_device;
		Window* m_windowRef;

		void CreateFrameBuffer();
		void CreateCommandBuffers();
		void CreateSync();
		void CleanupSwapChain() const;

		void RecordDrawCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex, int testIndex);

		void CreateUniformBuffers();
		void UpdateUniformBuffer(uint32_t currentImage);
		void CreateDescriptorPool();
		void CreateDescriptorSets();

		VkCommandBuffer m_currentBufferDraw;
		uint32_t m_currentImageIndexDraw;
	};
}