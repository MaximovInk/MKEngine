#include <mkpch.h>
#include "vkState.h"
#include "device.h"
#include "MKEngine/Core/Log.h"
#include "buffer.h"
#include "vkExtern.h"

#if defined(_WIN32)
#include "vulkan/vulkan_win32.h"
#endif

namespace MKEngine {

	uint32_t VulkanDevice::GetPresentViewQueueFamilyIndex() {

		std::vector<VkBool32> supportsPresent(QueueFamilyProperties.size());
		for (uint32_t i = 0; i < QueueFamilyProperties.size(); i++)
		{
#if defined(_WIN32)
			if (vkGetPhysicalDeviceWin32PresentationSupportKHR(vkState::API->PhysicalDevice, i))
				return i;
#else
			if (QueueFamilyProperties[i].queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
				return i;
#endif
		}

		MK_LOG_ERROR("Cannot find present view queue family index!");

		return 0;

	}

	uint32_t VulkanDevice::GetQueueFamilyIndex(const VkQueueFlags queueFlags)
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

		return 0;
	}

	bool VulkanDevice::ExtensionSupported(const std::string& extension)
	{
		return (std::find(SupportedExtensions.begin(), SupportedExtensions.end(), extension) != SupportedExtensions.end());
	}

	VkResult VulkanDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
		std::vector<const char*> enabledExtensions, void* pNextChain, VkQueueFlags requestedQueueTypes)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		constexpr float defaultQueuePriority(0.0f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			vkState::API->QueueFamilyIndices.Graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			queueInfo.queueFamilyIndex = vkState::API->QueueFamilyIndices.Graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
			vkState::API->QueueFamilyIndices.Graphics = 0;

		MK_LOG_INFO("graphics family: {0}", vkState::API->QueueFamilyIndices.Graphics);

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			vkState::API->QueueFamilyIndices.Compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (vkState::API->QueueFamilyIndices.Compute != vkState::API->QueueFamilyIndices.Graphics)
			{
				// If compute family index differs, 
				//we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = vkState::API->QueueFamilyIndices.Compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			vkState::API->QueueFamilyIndices.Compute = vkState::API->QueueFamilyIndices.Graphics;

		MK_LOG_INFO("compute family: {0}", vkState::API->QueueFamilyIndices.Compute);

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			vkState::API->QueueFamilyIndices.Transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((vkState::API->QueueFamilyIndices.Transfer != vkState::API->QueueFamilyIndices.Graphics)
				&& (vkState::API->QueueFamilyIndices.Transfer != vkState::API->QueueFamilyIndices.Compute))
			{
				// If transfer family index differs, 
				//we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = vkState::API->QueueFamilyIndices.Transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			vkState::API->QueueFamilyIndices.Transfer = vkState::API->QueueFamilyIndices.Graphics;

		MK_LOG_INFO("transfer family: {0}", vkState::API->QueueFamilyIndices.Transfer);

		//Present view queue
		{
			vkState::API->QueueFamilyIndices.Present = GetPresentViewQueueFamilyIndex();
			if (vkState::API->QueueFamilyIndices.Present != vkState::API->QueueFamilyIndices.Graphics)
			{
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = vkState::API->QueueFamilyIndices.Present;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}

		}

		MK_LOG_INFO("present family: {0}", vkState::API->QueueFamilyIndices.Present);

		std::vector<const char*> deviceExtensions(std::move(enabledExtensions));

		/*
		EXTENSIONS FOR DEVICE
		*/
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
#if VULKAN_VALIDATION
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
		deviceCreateInfo.enabledLayerCount = 0;
#endif

		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		if (pNextChain) {
			physicalDeviceFeatures2.features = enabledFeatures;
			physicalDeviceFeatures2.pNext = pNextChain;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &physicalDeviceFeatures2;
		}

		if (!deviceExtensions.empty())
		{
			for (const char* enabledExtension : deviceExtensions)
			{
				if (!ExtensionSupported(enabledExtension)) {
					std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
				}
			}

			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		this->EnabledFeatures = enabledFeatures;

		return vkCreateDevice(vkState::API->PhysicalDevice, &deviceCreateInfo, nullptr, &vkState::API->LogicalDevice);
	}

	VkCommandPool   VulkanDevice::CreateCommandPool(const uint32_t queueFamilyIndex,
	                                                const VkCommandPoolCreateFlags createFlags)
	{
		VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
		commandPoolInfo.flags = createFlags;
		VkCommandPool commandPool;

		if (vkCreateCommandPool(vkState::API->LogicalDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			MK_LOG_CRITICAL("Failed to create command pool");
		}
		else
			MK_LOG_INFO("CommandPool device successfully created");

		return commandPool;
	}

	void VulkanDevice::CreateCommandBuffer() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = vkState::API->CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(vkState::API->LogicalDevice, &allocInfo, &vkState::API->MainCommandBuffer) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to allocate command buffer!");
		}
	}

	VulkanDevice CreateDevice()
	{
		VulkanDevice device{};

		std::vector<const char*> enabledExtensions;

		vkState::API->PhysicalDevice = VkExtern::CreatePhysicalDevice(vkState::API->Instance);

		vkGetPhysicalDeviceProperties(vkState::API->PhysicalDevice, &vkState::API->Properties);
		vkGetPhysicalDeviceFeatures(vkState::API->PhysicalDevice, &device.Features);
		vkGetPhysicalDeviceMemoryProperties(vkState::API->PhysicalDevice, &device.MemoryProperties);
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(vkState::API->PhysicalDevice, &queueFamilyCount, nullptr);
		MK_ASSERT(queueFamilyCount > 0, "QUEUE FAMILY COUNT MUST BE > 0");
		MK_LOG_INFO("QUEUE FAMILY COUNT: {0}", queueFamilyCount);
		device.QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vkState::API->PhysicalDevice, &queueFamilyCount, device.QueueFamilyProperties.data());

		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(vkState::API->PhysicalDevice, nullptr, &extensionCount, nullptr);
		if (extensionCount > 0)
		{
			if (std::vector<VkExtensionProperties> extensions(extensionCount);
				vkEnumerateDeviceExtensionProperties(vkState::API->PhysicalDevice, nullptr, &extensionCount, &extensions.front()) == VK_SUCCESS)
			{
				for (const auto& [extensionName, specVersion] : extensions)
				{
					device.SupportedExtensions.emplace_back(extensionName);
				}
			}
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = true;

		if (device.CreateLogicalDevice(deviceFeatures, DeviceExtensions, nullptr) != VK_SUCCESS)
			MK_LOG_CRITICAL("Failed to create logical device!");
		else
			MK_LOG_INFO("Logical device successfully created");

		vkState::API->GraphicsQueue = VkExtern::GetQueue(vkState::API->LogicalDevice, vkState::API->QueueFamilyIndices.Graphics, 0);
		vkState::API->PresentQueue = VkExtern::GetQueue(vkState::API->LogicalDevice, vkState::API->QueueFamilyIndices.Present, 0);

		vkState::API->CommandPool = device.CreateCommandPool(vkState::API->QueueFamilyIndices.Graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		device.CreateCommandBuffer();

		return device;
	}

	void DestroyDevice(VulkanDevice device)
	{
		if (vkState::API->CommandPool)
			vkDestroyCommandPool(vkState::API->LogicalDevice, vkState::API->CommandPool, nullptr);

		if (vkState::API->LogicalDevice)
			vkDestroyDevice(vkState::API->LogicalDevice, nullptr);
	}

}