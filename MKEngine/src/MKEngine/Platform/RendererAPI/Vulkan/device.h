#pragma once
#include "buffer.h"
#include "MKEngine/Platform/Window.h"
#include "pipeline.h"
#include "shaders.h"
#include "texture.h"

namespace MKEngine {
	constexpr uint32_t FRAME_IN_FLIGHT = 2;

	class VulkanPresentView;

	class VulkanDevice {
	public:
		VkInstance Instance = VK_NULL_HANDLE;
#if VULKAN_VALIDATION
		VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
#endif
		std::map<std::int16_t, VulkanPresentView*> PresentViews;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice LogicalDevice = VK_NULL_HANDLE;
		VkCommandPool CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer MainCommandBuffer = VK_NULL_HANDLE;

		VkPhysicalDeviceProperties Properties;
		VkPhysicalDeviceFeatures Features;
		VkPhysicalDeviceFeatures EnabledFeatures;
		VkPhysicalDeviceMemoryProperties MemoryProperties;

		std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
		std::vector<std::string> SupportedExtensions;

		VkQueue GraphicsQueue{};
		VkQueue PresentQueue{};

		Pipeline GraphicsPipeline;

		//TEST
		Buffer VertexBuffer;
		Buffer IndicesBuffer;
		VkDescriptorSetLayout DescriptorSetLayout;
		Texture TestTexture;

		const std::vector<const char*> DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		struct
		{
			uint32_t Graphics;
			uint32_t Compute;
			uint32_t Transfer;
			uint32_t Present; //combine with graphics?
		} QueueFamilyIndices;


		 VulkanDevice();
		~VulkanDevice();

		uint32_t		GetPresentViewQueueFamilyIndex() const;
		uint32_t		GetQueueFamilyIndex(VkQueueFlags queueFlags) const;
		bool			ExtensionSupported(const std::string& extension);
		VkResult        CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
			std::vector<const char*> enabledExtensions, void* pNextChain, 
			VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
		VkResult		CreateGraphicsPipeline(const GraphicsPipelineDesc& description);
		
		Buffer CreateBuffer(const BufferDescription& description);
		void DestroyBuffer(const Buffer& buffer) const;
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

		Shader CreateShader(ShaderCreateDescription description) const;
		void DestroyShader(const Shader& shader) const;

		Texture CreateTexture(const TextureDescription& description);
		void DestroyTexture(Texture texture);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(Buffer buffer, Texture image);

		VkCommandPool   CreateCommandPool(uint32_t queueFamilyIndex,
			VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const;
		void CreateCommandBuffer();

		void ImmediateSubmit(std::function<void(VkCommandBuffer)> const& callback) const;

		void OnWindowDrawTest(Window* window, int index);

		void OnWindowCreate(Window* window);
		void OnWindowDestroy(const Window* window);
		void OnWindowResize(Window* window);
		void OnWindowRender(const Window* window);
		void OnWindowRenderEnd(const MKEngine::Window* window);
		void WaitDeviceIdle() const;

	};
}
