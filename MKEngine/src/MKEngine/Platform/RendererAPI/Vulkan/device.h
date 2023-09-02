#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine {

	const std::vector DEVICE_EXTENSIONS = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	class Device {
	public:
		VkPhysicalDeviceFeatures Features;
		VkPhysicalDeviceFeatures EnabledFeatures;
		VkPhysicalDeviceMemoryProperties MemoryProperties;

		std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
		std::vector<std::string> SupportedExtensions;

		VkPhysicalDeviceDynamicRenderingFeaturesKHR DynamicRenderingFeaturesKhr{};

		Device() = default;

		static Device Create();

		uint32_t		GetPresentViewQueueFamilyIndex() const;
		uint32_t		GetQueueFamilyIndex(VkQueueFlags queueFlags) const;
		bool			ExtensionSupported(const std::string& extension);
		VkResult        CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
			std::vector<const char*> enabledExtensions, void* pNextChain, 
			VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
	
		VkCommandPool   CreateCommandPool(uint32_t queueFamilyIndex,
			VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		void CreateCommandBuffer();

	};
}
