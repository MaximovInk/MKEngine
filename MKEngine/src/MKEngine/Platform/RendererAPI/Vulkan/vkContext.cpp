#include "mkpch.h"
#include "VkContext.h"

namespace MKEngine {
	VkContext* VkContext::API;

	void WaitDeviceIdle()
	{
		vkDeviceWaitIdle(VkContext::API->LogicalDevice);
	}
}