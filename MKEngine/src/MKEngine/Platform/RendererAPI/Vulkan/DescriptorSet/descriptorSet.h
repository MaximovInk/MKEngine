#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
	class DescriptorSet
	{
	public:
		VkDescriptorSet Resource;

		void BindTexture();
		void BindBuffer();
		void BindSampler();
		void Create();

	private:
		std::vector<VkDescriptorPoolSize> poolSizes;

	};

}
