#pragma once
#include <vulkan/vulkan.h>

#include "descriptorSetLayout.h"

namespace MKEngine
{
	struct DescriptorSetDescription
	{
		DescriptorSetLayout Layout;
	};

	class DescriptorSet
	{
	public:
		VkDescriptorSet Resource;
		VkDescriptorPool Pool;

		DescriptorSet() = default;

		static DescriptorSet Create(const DescriptorSetDescription& description);
		static void Destroy(DescriptorSet descriptorSet);

		void BindBuffer(uint32_t bindingIndex, const VkBuffer buffer, uint32_t offset, uint32_t range) const;
		void BindCombinedImageSampler(uint32_t bindingIndex, const VkImageView imageView, const VkSampler sampler) const;
	private:

		std::vector<VkDescriptorPoolSize> m_poolSizes;
		DescriptorSetLayout m_layout;
	};

}
