#pragma once
#include "mkpch.h"
#include "MKEngine/Core/core.h"
#include "vulkan/vulkan.h"
#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/Window.h"

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

		static void DestroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messenger);
		static VkSurfaceKHR CreateWindowSurface(VkInstance instance, MKEngine::Window* window);

		/*
		static void CreateQueueFamily();
		
		static void CreateSwapChain();
		*/

	};

}