#include "mkpch.h"
#include "Fence.h"

#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/VkContext.h"

namespace MKEngine
{
	Fence Fence::Create(const VkFenceCreateInfo& description)
	{
		Fence fence;
		if (vkCreateFence(VkContext::API->LogicalDevice, &description, nullptr, &fence.Resource) != VK_SUCCESS)
		{
			MK_LOG_ERROR("Failed to create fence!");
		}

		return fence;
	}

	void Fence::Destroy(const Fence fence)
	{
		vkDestroyFence(VkContext::API->LogicalDevice, fence.Resource, nullptr);
	}

	bool Fence::Wait(const uint64_t timeout) const
	{
		const auto result = vkWaitForFences(VkContext::API->LogicalDevice, 1, &Resource, true, timeout);

		return result == VK_SUCCESS;
	}

	bool Fence::GetStatus() const
	{
		const auto result = vkGetFenceStatus(VkContext::API->LogicalDevice, Resource);

		return result == VK_SUCCESS;
	}

	bool Fence::Reset() const
	{
		const auto result = vkResetFences(VkContext::API->LogicalDevice, 1, &Resource);

		return result == VK_SUCCESS;
	}
}
