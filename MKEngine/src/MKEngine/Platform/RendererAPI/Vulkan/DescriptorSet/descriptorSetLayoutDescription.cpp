#include "mkpch.h"
#include "descriptorSetLayoutDescription.h"

namespace MKEngine
{
	const BindingInfo& DescriptorSetLayoutDescription::GetBindingInfo(uint32_t bindingIndex) const
	{
		return m_bindingInfos[bindingIndex];
	}

	const std::vector<BindingInfo>& DescriptorSetLayoutDescription::GetBindingInfos() const
	{
		return m_bindingInfos;
	}

	void DescriptorSetLayoutDescription::AddBinding(VkDescriptorType type, uint32_t count, VkShaderStageFlags shaderStages)
	{
		BindingInfo bindingInfo;
		bindingInfo.Type = type;
		bindingInfo.Count = count;
		bindingInfo.Flags = {};
		bindingInfo.ShaderStages = shaderStages;

		m_bindingInfos.emplace_back(bindingInfo);
	}
}