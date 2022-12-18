#pragma once

#include "vulkan/vulkan.h"
#include "mkpch.h"
#include "MKEngine/Platform/Window.h"

namespace MKEngine {

	//class Window;

	class VulkanPresentView;

	struct VulkanDevice {
	public:
		VkInstance Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
		//VkSurfaceKHR Surface = VK_NULL_HANDLE;
		//VulkanPresentView* PresentView;
		//VulkanPresentView* PresentViews;
		std::map<std::int16_t, VulkanPresentView*> PresentViews;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice Device = VK_NULL_HANDLE;
		VkCommandPool CommandPool = VK_NULL_HANDLE;

		VkPhysicalDeviceProperties Properties;
		VkPhysicalDeviceFeatures Features;
		VkPhysicalDeviceFeatures EnabledFeatures;
		VkPhysicalDeviceMemoryProperties MemoryProperties;

		std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
		std::vector<std::string> SupportedExtensions;

		const std::vector<const char*> DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		struct
		{
			uint32_t graphics;
			uint32_t compute;
			uint32_t transfer;
		} QueueFamilyIndices;

		 VulkanDevice();
		~VulkanDevice();

		uint32_t		GetQueueFamilyIndex(VkQueueFlags queueFlags) const;
		bool			ExtensionSupported(std::string extension);
		VkResult        CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
			std::vector<const char*> enabledExtensions, void* pNextChain, 
			VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

		VkCommandPool   CreateCommandPool(uint32_t queueFamilyIndex,
			VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		void OnWindowCreate(MKEngine::Window* window);
		void OnWindowDestroy(MKEngine::Window* window);
	};
}