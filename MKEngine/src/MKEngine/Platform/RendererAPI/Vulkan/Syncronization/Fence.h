#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
	class Fence
	{
	public:
		VkFence Resource;

		static Fence Create(const VkFenceCreateInfo& description);
		static void Destroy(Fence fence);

		bool Wait(uint64_t timeout = UINT64_MAX) const;
		bool GetStatus() const;
		bool Reset() const;
	};
}
