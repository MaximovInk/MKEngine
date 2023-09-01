#include "mkpch.h"

#include "descriptorSetLayout.h"

#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/VkContext.h"

namespace MKEngine
{
	DescriptorSetLayout DescriptorSetLayout::CreateDescriptorSetLayout(
		const DescriptorSetLayoutDescription& description)
	{
		DescriptorSetLayout descriptorSetLayout;

		std::vector<VkDescriptorSetLayoutBinding> vkBindings;
		vkBindings.reserve(description.GetBindingInfos().size());
		std::vector<VkDescriptorBindingFlags> vkBindingsFlags;
		vkBindingsFlags.reserve(description.GetBindingInfos().size());

		for (uint32_t i = 0; i < description.GetBindingInfos().size(); i++)
		{
			const auto& binding = description.GetBindingInfo(i);

			VkDescriptorSetLayoutBinding vkBinding;

			vkBinding.binding = i;
			vkBinding.descriptorType = binding.Type;
			vkBinding.descriptorCount = binding.Count;
			vkBinding.stageFlags = binding.ShaderStages;
			vkBinding.pImmutableSamplers = nullptr;
			vkBindings.emplace_back(vkBinding);

			vkBindingsFlags.emplace_back(binding.Flags);

			descriptorSetLayout.m_bindingInfos.emplace_back(binding);
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		bindingFlagsCreateInfo.bindingCount = vkBindingsFlags.size();
		bindingFlagsCreateInfo.pBindingFlags = vkBindingsFlags.data();

		VkDescriptorSetLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		createInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
		createInfo.pBindings = vkBindings.data();
		createInfo.pNext = &bindingFlagsCreateInfo;

		if (vkCreateDescriptorSetLayout(VkContext::API->LogicalDevice, &createInfo, nullptr, &descriptorSetLayout.Resource) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to create descriptor set layout!");
		}

		return descriptorSetLayout;
	}

	void DescriptorSetLayout::DestroyDescriptorSetLayout(const DescriptorSetLayout& descriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(VkContext::API->LogicalDevice, descriptorSetLayout.Resource, nullptr);
	}

	const BindingInfo& DescriptorSetLayout::GetBindingInfo(const uint32_t bindingIndex) const
	{
		return m_bindingInfos[bindingIndex];
	}

	const std::vector<BindingInfo>& DescriptorSetLayout::GetBindingInfos() const
	{
		return m_bindingInfos;
	}
}
