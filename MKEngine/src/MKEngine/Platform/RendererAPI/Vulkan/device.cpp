#include "mkpch.h"
#include "device.h"
#include "MKEngine/Core/Log.h"

#include "vkExtern.h"

namespace MKEngine {
	uint32_t VulkanDevice::GetQueueFamilyIndex(VkQueueFlags queueFlags) const
	{
		if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(QueueFamilyProperties.size()); i++)
			{
				if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) 
					&& ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
				}
			}
		}

		if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(QueueFamilyProperties.size()); i++)
			{
				if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) 
					&& ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) 
					&& ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					return i;
				}
			}
		}

		for (uint32_t i = 0; i < static_cast<uint32_t>(QueueFamilyProperties.size()); i++)
		{
			if ((QueueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			{
				return i;
			}
		}

		MK_LOG_ERROR("Could not find a matching queue family index");
	}

	bool VulkanDevice::ExtensionSupported(std::string extension)
	{
		return (std::find(SupportedExtensions.begin(), SupportedExtensions.end(), extension) != SupportedExtensions.end());
	}

	VulkanDevice::VulkanDevice(/*VkPhysicalDevice physicalDevice,*/ /*
	VkPhysicalDeviceFeatures enabledFeatures,
		std::vector<const char*> enabledExtensions, void* pNextChain
	*/)
	{
		Instance = VkExtern::CreateInstance();

		VkPhysicalDeviceFeatures enabledFeatures;
		std::vector<const char*> enabledExtensions;
		void* pNextChain = nullptr;

		PhysicalDevice = VkExtern::CreatePhysicalDevice(Instance);

		vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);
		vkGetPhysicalDeviceFeatures(PhysicalDevice, &Features);
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemoryProperties);
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, nullptr);
		MK_ASSERT(queueFamilyCount > 0, "QUEUE FAMILY COUNT MUST BE > 0");
		QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, QueueFamilyProperties.data());

		uint32_t extenshionCount = 0;
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extenshionCount, nullptr);
		if (extenshionCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extenshionCount);
			if (vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extenshionCount, &extensions.front()) == VK_SUCCESS)
			{
				for (VkExtensionProperties extenshion : extensions)
				{
					SupportedExtensions.push_back(extenshion.extensionName);
				}
			}
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		if (CreateLogicalDevice(deviceFeatures, DeviceExtensions, nullptr) != VK_SUCCESS)
			MK_LOG_ERROR("FAILED TO CREATE LOGICAL DEVICE");
#if TRACE_INITIALIZATION_RENDERER
		else
			MK_LOG_INFO("[VULKAN] Logical device successfully created");
#endif
	}

	VulkanDevice::~VulkanDevice()
	{
		if (CommandPool)
			vkDestroyCommandPool(Device, CommandPool, nullptr);
		if (Device)
			vkDestroyDevice(Device, nullptr);
		if(Instance)
			vkDestroyInstance(Instance, nullptr);
	}


	VkResult VulkanDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
		std::vector<const char*> enabledExtensions, void* pNextChain, VkQueueFlags requestedQueueTypes)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		const float defaultQueuePriority(0.0f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			QueueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = QueueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
			QueueFamilyIndices.graphics = 0;

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			QueueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (QueueFamilyIndices.compute != QueueFamilyIndices.graphics)
			{
				// If compute family index differs, 
				//we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = QueueFamilyIndices.compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			QueueFamilyIndices.compute = QueueFamilyIndices.graphics;

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			QueueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((QueueFamilyIndices.transfer != QueueFamilyIndices.graphics)
				&& (QueueFamilyIndices.transfer != QueueFamilyIndices.compute))
			{
				// If transfer family index differs, 
				//we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = QueueFamilyIndices.transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			QueueFamilyIndices.transfer = QueueFamilyIndices.graphics;

		std::vector<const char*> deviceExtensions(enabledExtensions);

		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
		if (pNextChain) {
			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			physicalDeviceFeatures2.features = enabledFeatures;
			physicalDeviceFeatures2.pNext = pNextChain;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &physicalDeviceFeatures2;
		}

		if (deviceExtensions.size() > 0)
		{
			for (const char* enabledExtension : deviceExtensions)
			{
				if (!ExtensionSupported(enabledExtension)) {
					std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
				}
			}

			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		this->EnabledFeatures = enabledFeatures;

		return vkCreateDevice(PhysicalDevice, &deviceCreateInfo, nullptr, &Device);
	}
}