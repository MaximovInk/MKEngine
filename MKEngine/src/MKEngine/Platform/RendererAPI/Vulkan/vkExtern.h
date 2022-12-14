#pragma once
#include "mkpch.h"
#include "MKEngine/Core/core.h"
#include "vulkan/vulkan.h"
#include "MKEngine/Core/Log.h"

#define TRACE_INITIALIZATION_RENDERER 1

/*
	bool bInitSuccess = CreateInstance()
	&& CreatePhysicalDevice()
	&& CreateWindowSurface()
	&& CreateQueueFamily()
	&& CreateLogicalDevice()
	&& CreateSwapChain();
	*/

namespace MKEngine {

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			switch (messageSeverity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				MK_LOG_WARN("[VULKAN][VALIDIDATION LAYERS] {}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				MK_LOG_ERROR("[VULKAN][VALIDIDATION LAYERS] {}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
				MK_LOG_ERROR("[VULKAN][VALIDIDATION LAYERS] {}", pCallbackData->pMessage);
				break;
			}
		}

		return VK_FALSE;
	}

	const std::vector<const char*> ValidationLayers = {
"VK_LAYER_KHRONOS_validation"
	};

	class VkExtern {
	public:
#if _DEBUG
		static const bool EnableValidationLayers = true;
#else
		static const bool enableValidationLayers = false;
#endif

		static std::vector<std::string> SupportedInstanceExtensions;

		static VkInstance CreateInstance();
		static VkDebugUtilsMessengerEXT CreateDebugMessenger(VkInstance instance);
		static VkPhysicalDevice CreatePhysicalDevice(VkInstance instance);

		/*
		static VkSurfaceKHR CreateWindowSurface();
		static void CreateQueueFamily();
		
		static void CreateSwapChain();
		*/

	};

}