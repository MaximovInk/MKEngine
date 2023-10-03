#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
	class Semaphore {
	public:
		VkSemaphore Resource;

		static Semaphore Create();
		static void Destroy(Semaphore semaphore);
	};
}
