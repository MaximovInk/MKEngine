#include "mkpch.h"
#include "Semaphore.h"

#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/VkContext.h"

namespace MKEngine {
	Semaphore Semaphore::Create()
    {
        Semaphore semaphore;

        constexpr VkSemaphoreCreateInfo createInfo { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        if (vkCreateSemaphore(VkContext::API->LogicalDevice, &createInfo, nullptr, &semaphore.Resource) != VK_SUCCESS)
            MK_LOG_ERROR("Failed to create semaphore");

        return semaphore;
    }

    void Semaphore::Destroy(const Semaphore semaphore)
    {
        vkDestroySemaphore(VkContext::API->LogicalDevice, semaphore.Resource, nullptr);
    }

}
