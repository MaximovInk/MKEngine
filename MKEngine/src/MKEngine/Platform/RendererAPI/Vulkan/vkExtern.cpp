#include <mkpch.h>

#include <SDL_vulkan.h>
#include <SDL.h>

#include "vkExtern.h"

#if defined(_WIN32)
#include "vulkan/vulkan_win32.h"
#endif

namespace MKEngine {
	std::vector<std::string> VkExtern::SupportedInstanceExtensions;

#if VULKAN_VALIDATION
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			switch (messageSeverity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				MK_LOG_WARN("[VALIDIDATION LAYERS] {}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				MK_LOG_ERROR("[VALIDIDATION LAYERS] {}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
				MK_LOG_ERROR("[VALIDIDATION LAYERS] {}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				break;
			default: ;
			}
		}

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")); func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")); func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	VkDebugUtilsMessengerEXT VkExtern::CreateDebugMessenger(const VkInstance instance)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		VkDebugUtilsMessengerEXT debugMessenger;
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			MK_LOG_CRITICAL("Failed to set up debug messenger!");
		}
		else
			MK_LOG_INFO("DebugCallback successfully created");


		return debugMessenger;
	}

	void VkExtern::DestroyDebugMessenger(const VkInstance instance, const VkDebugUtilsMessengerEXT messenger)
	{
		DestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
	}
#endif

	std::vector<const char*> GetRequiredInstanceExtensions() {
		std::vector extensionNames =
		{
			VK_KHR_SURFACE_EXTENSION_NAME
		};
		 
#if defined(_WIN32)
		extensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		extensionNames.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
		extensionNames.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
		extensionNames.push_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		extensionNames.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		extensionNames.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
		extensionNames.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
		extensionNames.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
		extensionNames.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#endif

#if VULKAN_VALIDATION
		extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		return extensionNames;
	}

	void VkExtern::PrintVulkanVersion()
	{
		uint32_t version{ 0 };
		vkEnumerateInstanceVersion(&version);
		MK_LOG_INFO("VULKAN VERSION Variant: {0}\nMAJOR {1}\nMINOR {2}\nPATCH {3}",
			VK_API_VERSION_VARIANT(version),
			VK_API_VERSION_MAJOR(version),
			VK_API_VERSION_MINOR(version),
			VK_API_VERSION_PATCH(version));

		version &= ~(0xFFFU);
	}

	VkInstance VkExtern::CreateInstance()
	{
		VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pApplicationName = "MKEngine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "MKEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pApplicationInfo = &appInfo;

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		if (extensionCount > 0)
		{
			if (std::vector<VkExtensionProperties> extensions(extensionCount); 
				vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, &extensions.front()) == VK_SUCCESS)
			{
				for (const auto& [extensionName, specVersion] : extensions)
				{
					SupportedInstanceExtensions.emplace_back(extensionName);
				}
			}
		}

		const auto requiredExtensions = GetRequiredInstanceExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#if VULKAN_VALIDATION
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
#endif

		VkInstance instance;
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			MK_LOG_CRITICAL("Failed to create instance!");
			return VK_NULL_HANDLE;
		}

		MK_LOG_INFO("Instance successfully created");

		return instance;
	}

	VkPhysicalDevice VkExtern::CreatePhysicalDevice(const VkInstance instance)
	{
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

		if (physicalDeviceCount == 0)
		{
			MK_LOG_CRITICAL("Failed to find GPUs with Vulkan support!");
			return VK_NULL_HANDLE;
		}

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

		VkPhysicalDevice selectedPhysicalDevice = VK_NULL_HANDLE;

		for (const auto& physicalDevice : physicalDevices) {

			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physicalDevice, &properties);

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				selectedPhysicalDevice = physicalDevice;
		}

		if (selectedPhysicalDevice == VK_NULL_HANDLE)
			selectedPhysicalDevice = physicalDevices[0];

		{
			VkPhysicalDeviceProperties sProperties;
			vkGetPhysicalDeviceProperties(selectedPhysicalDevice, &sProperties);
			MK_LOG_INFO("Selected GPU: {}", sProperties.deviceName);
		}
		
		return selectedPhysicalDevice;
	}

	std::vector<std::string> VkExtern::LogAlphaCompositeBits(const VkCompositeAlphaFlagsKHR bits)
	{
		std::vector<std::string> result;

		if (bits & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
			result.emplace_back("opaque (alpha ignored)");
		}
		if (bits & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
			result.emplace_back("pre multiplied (alpha expected to already be multiplied in image)");
		}
		if (bits & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
			result.emplace_back("post multiplied (alpha will be applied during composition)");
		}
		if (bits & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
			result.emplace_back("inherited");
		}

		return result;
	}

	std::vector<std::string> VkExtern::LogImageUsageBits(const VkImageUsageFlags bits)
	{
		std::vector<std::string> result;
		if (bits & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
			result.emplace_back("transfer src");
		}
		if (bits & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
			result.emplace_back("transfer dst");
		}
		if (bits & VK_IMAGE_USAGE_SAMPLED_BIT) {
			result.emplace_back("sampled");
		}
		if (bits & VK_IMAGE_USAGE_STORAGE_BIT) {
			result.emplace_back("storage");
		}
		if (bits & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
			result.emplace_back("color attachment");
		}
		if (bits & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			result.emplace_back("depth/stencil attachment");
		}
		if (bits & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
			result.emplace_back("transient attachment");
		}
		if (bits & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
			result.emplace_back("input attachment");
		}
		if (bits & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
			result.emplace_back("fragment density map");
		}
		if (bits & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
			result.emplace_back("fragment shading rate attachment");
		}
		return result;
	}

	std::string VkExtern::LogPresentMode(const VkPresentModeKHR presentMode)
	{
		
		if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			return "immediate";
		}
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return "mailbox";
		}
		if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
			return "fifo";
		}
		if (presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
			return "relaxed fifo";
		}
		if (presentMode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) {
			return "shared demand refresh";
		}
		if (presentMode == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR) {
			return "shared continuous refresh";
		}
		return "none/undefined";
	}

	VkShaderModule VkExtern::CreateShaderModule(const VkDevice logicalDevice, const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			MK_LOG_ERROR("Failed to create shader module!");
			return nullptr;
		}

		return shaderModule;
	}

	VkSurfaceKHR VkExtern::CreateWindowSurface(const VkInstance instance, const MKEngine::Window* window)
	{
		VkSurfaceKHR surface;

		if (const auto sdlWindow = static_cast<SDL_Window*>(window->GetNativeWindow());
			SDL_Vulkan_CreateSurface(sdlWindow, instance, &surface) != SDL_TRUE) {
			MK_LOG_CRITICAL("Failed to set up surface!");
		}
		else
			MK_LOG_INFO("Surface successfully created");

		return surface;
	}

	void VkExtern::WaitFence(const VkDevice logicalDevice, const VkFence fence)
	{
		vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX);
	}

	void VkExtern::ResetFence(const VkDevice logicalDevice, const VkFence fence)
	{
		vkResetFences(logicalDevice, 1, &fence);
	}

	VkBool32 VkExtern::GetSupportedDepthStencilFormat(const VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat)
	{
		const std::vector<VkFormat> formatList = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
		};

		for (auto& format : formatList)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				*depthStencilFormat = format;
				return true;
			}
		}

		return false;
	}

	VkBool32 VkExtern::GetSupportedDepthFormat(const VkPhysicalDevice physicalDevice, VkFormat* depthFormat)
	{
		// Since all depth formats may be optional, we need to find a suitable depth format to use
		// Start with the highest precision packed format
		const std::vector<VkFormat> formatList = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& format : formatList)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				*depthFormat = format;
				return true;
			}
		}

		return false;
	}
	VkQueue VkExtern::GetQueue(const VkDevice logicalDevice, const int familyIndex, const int queueIndex)
	{
		VkQueue queue;
		vkGetDeviceQueue(logicalDevice, familyIndex, queueIndex, &queue);

		return queue;
	}

	std::vector<std::string> VkExtern::LogTransformBits(const VkSurfaceTransformFlagsKHR bits)
	{
		std::vector<std::string> result;

		if (bits & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
			result.emplace_back("identity");
		}
		if (bits & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) {
			result.emplace_back("90 degree rotation");
		}
		if (bits & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) {
			result.emplace_back("180 degree rotation");
		}
		if (bits & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) {
			result.emplace_back("270 degree rotation");
		}
		if (bits & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR) {
			result.emplace_back("horizontal mirror");
		}
		if (bits & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR) {
			result.emplace_back("horizontal mirror, then 90 degree rotation");
		}
		if (bits & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR) {
			result.emplace_back("horizontal mirror, then 180 degree rotation");
		}
		if (bits & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR) {
			result.emplace_back("horizontal mirror, then 270 degree rotation");
		}
		if (bits & VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR) {
			result.emplace_back("inherited");
		}

		return result;
	}

	VkSemaphore VkExtern::CreateSemaphore(const VkDevice device)
	{
		constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		VkSemaphore semaphore;
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);

		return semaphore;
	}

	VkFence VkExtern::CreateFence(const VkDevice device, const VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo fenceCreateInfo { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceCreateInfo.flags = flags;

		VkFence fence;
		vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);

		return fence;
	}

	uint32_t VkExtern::FindMemoryType(const VkPhysicalDevice device, const uint32_t typeFilter,
	                                  const VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			MK_LOG_ERROR("failed to find suitable memory type!");

			return 0;
		}
}
