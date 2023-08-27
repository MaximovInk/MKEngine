#include <mkpch.h>
#include "VkContext.h"
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
			if (vkGetPhysicalDeviceWin32PresentationSupportKHR(VkContext::API->PhysicalDevice, i))
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
			VkContext::API->QueueFamilyIndices.Graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			queueInfo.queueFamilyIndex = VkContext::API->QueueFamilyIndices.Graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
			VkContext::API->QueueFamilyIndices.Graphics = 0;

		MK_LOG_INFO("graphics family: {0}", VkContext::API->QueueFamilyIndices.Graphics);

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			VkContext::API->QueueFamilyIndices.Compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (VkContext::API->QueueFamilyIndices.Compute != VkContext::API->QueueFamilyIndices.Graphics)
			{
				// If compute family index differs, 
				//we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = VkContext::API->QueueFamilyIndices.Compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			VkContext::API->QueueFamilyIndices.Compute = VkContext::API->QueueFamilyIndices.Graphics;

		MK_LOG_INFO("compute family: {0}", VkContext::API->QueueFamilyIndices.Compute);

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			VkContext::API->QueueFamilyIndices.Transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((VkContext::API->QueueFamilyIndices.Transfer != VkContext::API->QueueFamilyIndices.Graphics)
				&& (VkContext::API->QueueFamilyIndices.Transfer != VkContext::API->QueueFamilyIndices.Compute))
			{
				// If transfer family index differs, 
				//we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = VkContext::API->QueueFamilyIndices.Transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			VkContext::API->QueueFamilyIndices.Transfer = VkContext::API->QueueFamilyIndices.Graphics;

		MK_LOG_INFO("transfer family: {0}", VkContext::API->QueueFamilyIndices.Transfer);

		//Present view queue
		{
			VkContext::API->QueueFamilyIndices.Present = GetPresentViewQueueFamilyIndex();
			if (VkContext::API->QueueFamilyIndices.Present != VkContext::API->QueueFamilyIndices.Graphics)
			{
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = VkContext::API->QueueFamilyIndices.Present;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}

		}

		MK_LOG_INFO("present family: {0}", VkContext::API->QueueFamilyIndices.Present);

		std::vector<const char*> deviceExtensions(std::move(enabledExtensions));

		/*
		EXTENSIONS FOR DEVICE
		*/
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

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

		//Dynamic rendering
		dynamicRenderingFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		dynamicRenderingFeaturesKHR.dynamicRendering = VK_TRUE;
		deviceCreateInfo.pNext = &dynamicRenderingFeaturesKHR;

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

		return vkCreateDevice(VkContext::API->PhysicalDevice, &deviceCreateInfo, nullptr, &VkContext::API->LogicalDevice);
	}

	VkCommandPool   VulkanDevice::CreateCommandPool(const uint32_t queueFamilyIndex,
	                                                const VkCommandPoolCreateFlags createFlags)
	{
		VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
		commandPoolInfo.flags = createFlags;
		VkCommandPool commandPool;

		if (vkCreateCommandPool(VkContext::API->LogicalDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			MK_LOG_CRITICAL("Failed to create command pool");
		}
		else
			MK_LOG_INFO("CommandPool device successfully created");

		return commandPool;
	}

	void VulkanDevice::CreateCommandBuffer() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VkContext::API->CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(VkContext::API->LogicalDevice, &allocInfo, &VkContext::API->MainCommandBuffer) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to allocate command buffer!");
		}
	}

	VulkanDevice CreateDevice()
	{
		VulkanDevice device{};

		std::vector<const char*> enabledExtensions;

		VkContext::API->PhysicalDevice = VkExtern::CreatePhysicalDevice(VkContext::API->Instance);

		vkGetPhysicalDeviceProperties(VkContext::API->PhysicalDevice, &VkContext::API->Properties);
		vkGetPhysicalDeviceFeatures(VkContext::API->PhysicalDevice, &device.Features);
		vkGetPhysicalDeviceMemoryProperties(VkContext::API->PhysicalDevice, &device.MemoryProperties);
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(VkContext::API->PhysicalDevice, &queueFamilyCount, nullptr);
		MK_ASSERT(queueFamilyCount > 0, "QUEUE FAMILY COUNT MUST BE > 0");
		MK_LOG_INFO("QUEUE FAMILY COUNT: {0}", queueFamilyCount);
		device.QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(VkContext::API->PhysicalDevice, &queueFamilyCount, device.QueueFamilyProperties.data());

		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(VkContext::API->PhysicalDevice, nullptr, &extensionCount, nullptr);
		if (extensionCount > 0)
		{
			if (std::vector<VkExtensionProperties> extensions(extensionCount);
				vkEnumerateDeviceExtensionProperties(VkContext::API->PhysicalDevice, nullptr, &extensionCount, &extensions.front()) == VK_SUCCESS)
			{
				for (const auto& [extensionName, specVersion] : extensions)
				{
					device.SupportedExtensions.emplace_back(extensionName);
				}
			}
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = true;

		if (device.CreateLogicalDevice(deviceFeatures, DEVICE_EXTENSIONS, nullptr) != VK_SUCCESS)
			MK_LOG_CRITICAL("Failed to create logical device!");
		else
			MK_LOG_INFO("Logical device successfully created");

		VkContext::API->GraphicsQueue = VkExtern::GetQueue(VkContext::API->LogicalDevice, VkContext::API->QueueFamilyIndices.Graphics, 0);
		VkContext::API->PresentQueue = VkExtern::GetQueue(VkContext::API->LogicalDevice, VkContext::API->QueueFamilyIndices.Present, 0);

		VkContext::API->CommandPool = device.CreateCommandPool(VkContext::API->QueueFamilyIndices.Graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		device.CreateCommandBuffer();

		VkContext::API->Begin = reinterpret_cast<PFN_vkCmdBeginRendering>(vkGetInstanceProcAddr(VkContext::API->Instance, "vkCmdBeginRendering"));
		VkContext::API->End = reinterpret_cast<PFN_vkCmdEndRendering>(vkGetInstanceProcAddr(VkContext::API->Instance, "vkCmdEndRendering"));

		return device;
	}

}