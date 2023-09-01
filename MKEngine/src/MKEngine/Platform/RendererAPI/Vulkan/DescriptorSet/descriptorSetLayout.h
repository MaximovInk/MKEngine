#pragma once
#include <vulkan/vulkan.h>
#include "descriptorSetLayoutDescription.h"

namespace MKEngine
{
	class DescriptorSetLayout
	{
	public:
		VkDescriptorSetLayout Resource;

		DescriptorSetLayout() = default;

		static DescriptorSetLayout CreateDescriptorSetLayout(const DescriptorSetLayoutDescription& description);
		static void DestroyDescriptorSetLayout(DescriptorSetLayout descriptorSetLayout);
	};
}
