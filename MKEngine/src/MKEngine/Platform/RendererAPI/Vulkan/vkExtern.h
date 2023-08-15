#pragma once
#include "mkpch.h"
#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/Window.h"
#include "vulkan/vulkan.h"

#include "vulkan/vulkan_win32.h"

#include <vulkan/vk_enum_string_helper.h>


namespace MKEngine {

#if VULKAN_VALIDATION
	inline std::vector validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};
#endif

	class VkExtern {
	public:
		static std::vector<std::string> SupportedInstanceExtensions;

#if VULKAN_VALIDATION

		static VkDebugUtilsMessengerEXT CreateDebugMessenger(VkInstance instance);
		static void DestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger);
#endif

		static void PrintVulkanVersion();
		static VkInstance CreateInstance();
		
		static VkPhysicalDevice CreatePhysicalDevice(VkInstance instance);

		static VkSurfaceKHR CreateWindowSurface(VkInstance instance, const MKEngine::Window* window);

		static void WaitFence(VkDevice logicalDevice, VkFence fence);
		static void ResetFence(VkDevice logicalDevice, VkFence fence);
		static VkBool32 GetSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat);
		static VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);

		static VkQueue GetQueue(VkDevice logicalDevice, int familyIndex, int queueIndex);

		static std::vector<std::string> LogTransformBits(const VkSurfaceTransformFlagsKHR bits);
		static std::vector<std::string> LogAlphaCompositeBits(const VkCompositeAlphaFlagsKHR bits);
		static std::vector<std::string> LogImageUsageBits(const VkImageUsageFlags bits);
		static std::string LogPresentMode(VkPresentModeKHR presentMode);

		static VkShaderModule CreateShaderModule(VkDevice logicalDevice, const std::vector<char>& code);

		static VkSemaphore CreateSemaphore(VkDevice device);
		static VkFence CreateFence(VkDevice device, VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT);
	};

}