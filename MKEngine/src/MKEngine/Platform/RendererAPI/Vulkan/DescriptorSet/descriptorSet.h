#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
	class DescriptorSet
	{
	public:
		VkDescriptorSet Resource;

		void BindImage(uint32_t bindingIndex, VkImage image, uint32_t arrayIndex = 0);
		void BindBuffer();
		void BindSampler();
		void Create();

	private:
		std::vector<VkDescriptorPoolSize> poolSizes;

	};

}
