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
		static void DestroyDescriptorSetLayout(const DescriptorSetLayout& descriptorSetLayout);
		const BindingInfo& GetBindingInfo(uint32_t bindingIndex) const;
		const std::vector<BindingInfo>& GetBindingInfos() const;
	private:
		std::vector<BindingInfo> m_bindingInfos;
	};
}
