#pragma once
#include <vector>
#include "MKEngine/Platform/Window.h"
#include "buffer.h"
#include "DescriptorSet/descriptorSet.h"
#include "Image/ImageView.h"
#include "VkContext.h"

namespace MKEngine {

	class Device;
	class Window;

	struct ViewSync
	{
		VkSemaphore ImageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;
		//VkFence InFlightFence = VK_NULL_HANDLE;
	};

	struct SwapChainInput {
		uint32_t Width; 
		uint32_t Height;
		bool Vsync;
		bool Fullscreen;
	};

	typedef struct SwapChainBuffers {
		ImageView View;
		ImageView Depth;
		CommandBuffer CmdBuffer;
		ViewSync Sync;
		Buffer UniformBuffer;
		DescriptorSet DescriptorSet;

	} SwapChainBuffer;

	class PresentView {
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

		PresentView() = default;

		static PresentView* Create();
		static void Destroy(PresentView* presentView);

		void InitSurface(Window* window);
		void CreateSwapChain();
		void FinalizeCreation();

		void RecreateSwapChain();

		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const;
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE) const;

		void Render();
	private:
		Window* m_windowRef;

		void CreateCommandBuffers();
		void CreateSync();
		void CleanupSwapChain(bool destroySwapChain = true) const;

		void CreateUniformBuffers();
		void UpdateUniformBuffer() const;
		void CreateDescriptorSets();

		CommandBuffer m_currentBufferDraw;
		uint32_t m_currentImageIndexDraw;

		bool m_renderIsBegin = false;

		void EndRender();
		void BeginRender();
	};


}
