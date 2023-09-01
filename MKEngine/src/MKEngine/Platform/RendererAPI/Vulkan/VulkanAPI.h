#pragma once
#include <vulkan/vulkan_core.h>
#include "MKEngine/Platform/Window.h"
#include "device.h"
#include "presentView.h"
#include "vkTexture.h"
#include "Pipeline/graphicsPipeline.h"
#include "model.h"
#include "mesh.h"
#include "MKEngine/Platform/RendererAPI/camera.h"

namespace MKEngine {
	class VulkanAPI
	{
	public:
#if VULKAN_VALIDATION
		VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
#endif

		static Mesh testMesh;
		static VkTexture testTexture;
		static Model testModel;
		static Camera testCamera;

		VulkanDevice Device;

		std::map<std::int16_t, VulkanPresentView*> PresentViews;

		GraphicsPipeline GraphicsPipeline;

		void Initialize();
		void Finalize();

		void OnWindowCreate(Window* window);
		void OnWindowDestroy(const Window* window);
		void OnWindowResize(Window* window);
		void OnWindowRender(const Window* window);
	};
}








/*
 TEST
Buffer VertexBuffer;
	 Buffer IndicesBuffer;
	 VkDescriptorSetLayout DescriptorSetLayout;
	 VkTexture TestTexture;

	 if (TestTexture.Resource == VK_NULL_HANDLE)
		{
			TextureDescription description{};
			description.Path = "textures/texture.jpg";
			description.Usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			description.Format = VK_FORMAT_R8G8B8A8_SRGB;

			TestTexture = CreateTexture(description);
		}


	 
	 */
