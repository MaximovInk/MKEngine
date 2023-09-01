#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
	struct BindingInfo
	{
		VkDescriptorType Type;
		uint32_t Count;
		VkDescriptorBindingFlags Flags;
		VkShaderStageFlags ShaderStages;
	};
	
	enum DescriptorSetType
	{
		Push,
		UpdateAfterBind
	};

	class DescriptorSetLayoutDescription
	{
	public:
		DescriptorSetType Type;

		DescriptorSetLayoutDescription() = default;

		const BindingInfo& GetBindingInfo(uint32_t bindingIndex) const;
		const std::vector<BindingInfo>& GetBindingInfos() const;
		void AddBinding(VkDescriptorType type, uint32_t count, VkShaderStageFlags shaderStages = VK_SHADER_STAGE_ALL);
		

	private:
		std::vector<BindingInfo> m_bindingInfos;
	};

}
