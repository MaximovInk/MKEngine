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
			const auto& [Type, Count, Flags, ShaderStages] = description.GetBindingInfo(i);

			VkDescriptorSetLayoutBinding vkBinding;

			vkBinding.binding = i;
			vkBinding.descriptorType = Type;
			vkBinding.descriptorCount = Count;
			vkBinding.stageFlags = ShaderStages;
			vkBinding.pImmutableSamplers = nullptr;
			vkBindings.emplace_back(vkBinding);

			vkBindingsFlags.emplace_back(Flags);
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		bindingFlagsCreateInfo.bindingCount = vkBindingsFlags.size();
		bindingFlagsCreateInfo.pBindingFlags = vkBindingsFlags.data();

		VkDescriptorSetLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		createInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
		createInfo.pBindings = vkBindings.data();
		//createInfo.pNext = &bindingFlagsCreateInfo;

		if (vkCreateDescriptorSetLayout(VkContext::API->LogicalDevice, &createInfo, nullptr, &descriptorSetLayout.Resource) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to create descriptor set layout!");
		}

		return descriptorSetLayout;
	}

	void DescriptorSetLayout::DestroyDescriptorSetLayout(DescriptorSetLayout descriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(VkContext::API->LogicalDevice, descriptorSetLayout.Resource, nullptr);
	}
}
