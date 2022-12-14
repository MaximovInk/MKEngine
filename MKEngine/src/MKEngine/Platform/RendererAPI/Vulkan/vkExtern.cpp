#include "mkpch.h"
#include "vkExtern.h"
#include "SDL_vulkan.h"
#include "SDL.h"

namespace MKEngine {

	std::vector<std::string> VkExtern::SupportedInstanceExtensions;

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	std::vector<const char*> getRequiredInstanceExtensions() {
		std::vector<const char*> extensionNames =
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

		return extensionNames;
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

		uint32_t extenshionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extenshionCount, nullptr);
		if (extenshionCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extenshionCount);
			if (vkEnumerateInstanceExtensionProperties(nullptr, &extenshionCount, &extensions.front()) == VK_SUCCESS)
			{
				for (VkExtensionProperties extension : extensions)
				{
					SupportedInstanceExtensions.push_back(extension.extensionName);
				}
			}
		}

		auto requiredExtensions = getRequiredInstanceExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		if (EnableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			createInfo.ppEnabledLayerNames = ValidationLayers.data();
		}

		VkInstance instance;
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			MK_LOG_CRITICAL("failed to create instance!");
			return VK_NULL_HANDLE;
		}
#if TRACE_INITIALIZATION_RENDERER
		else
			MK_LOG_INFO("[VULKAN] Instance successfully created");
#endif

		return instance;
	}

	VkDebugUtilsMessengerEXT VkExtern::CreateDebugMessenger(VkInstance instance)
	{
		if (!EnableValidationLayers) return VK_NULL_HANDLE;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo = {};
		createInfo.sType =
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;

		VkDebugUtilsMessengerEXT debugMessenger;
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
		return debugMessenger;
	}

	VkPhysicalDevice VkExtern::CreatePhysicalDevice(VkInstance instance)
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

		VkPhysicalDevice selectedPhyiscalDevice = VK_NULL_HANDLE;

		for (const auto& physicalDevice : physicalDevices) {

			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physicalDevice, &properties);

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				selectedPhyiscalDevice = physicalDevice;
		}

		if (selectedPhyiscalDevice == VK_NULL_HANDLE)
			selectedPhyiscalDevice = physicalDevices[0];

#if TRACE_INITIALIZATION_RENDERER
		{
			VkPhysicalDeviceProperties sProperties;
			vkGetPhysicalDeviceProperties(selectedPhyiscalDevice, &sProperties);
			MK_LOG_INFO("[VULKAN] Selected GPU: {}", sProperties.deviceName);
		}
#endif

		return selectedPhyiscalDevice;
	}
}















/*
bool IsDeviceExtenshionsSupported(VkPhysicalDevice physicalDevice) {
	uint32_t extenshionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extenshionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extenshionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extenshionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();

}
*/

/*
	void VkExtern::FindQueueFamilies(VkPhysicalDevice physicalDevice) {

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());


	}

*/
/*
//SRC: https://github.com/SaschaWillems/Vulkan/blob/86b5a1109b1b3d9a68f1effdec5c302671d593e0/base/VulkanDevice.cpp#L127
uint32_t GetQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkQueueFlags queueFlags) {

	if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				return i;
			}
		}
	}
*/


/*
static uint32_t VkExtern::tryGetGraphicsQueueFamilyIndex(
	VkPhysicalDevice _physicalDevice)
{
	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueCount, 0);

	std::vector<VkQueueFamilyProperties> queueProperties(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueCount, queueProperties.data());

	for (uint32_t queueIndex = 0; queueIndex < queueCount; ++queueIndex)
	{
		if (queueProperties[queueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			return queueIndex;
		}
	}

	return ~0u;
}
*/

