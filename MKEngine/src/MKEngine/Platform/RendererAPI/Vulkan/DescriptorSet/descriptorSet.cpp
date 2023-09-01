#include "mkpch.h"
#include "descriptorSet.h"

#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/vkContext.h"


namespace MKEngine {

	DescriptorSet DescriptorSet::Create(const DescriptorSetDescription& description)
	{
		DescriptorSet descriptorSet;
		descriptorSet.m_layout = description.Layout;

		std::vector<VkDescriptorPoolSize> poolSizes;

		for (uint32_t i = 0; i < description.Layout.GetBindingInfos().size(); i++)
		{
			const auto& [Type, Count, Flags, ShaderStages] = description.Layout.GetBindingInfo(i);

			VkDescriptorPoolSize descriptorPoolSize;
			descriptorPoolSize.type = Type;
			descriptorPoolSize.descriptorCount = Count;

			poolSizes.emplace_back(descriptorPoolSize);
		}

		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;
		if (vkCreateDescriptorPool(VkContext::API->LogicalDevice, &poolInfo, nullptr, &descriptorSet.Pool) != VK_SUCCESS) {
			MK_LOG_ERROR("Failed to create descriptor pool!");
		}

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = descriptorSet.Pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &description.Layout.Resource;

		if (vkAllocateDescriptorSets(VkContext::API->LogicalDevice, &allocInfo, &descriptorSet.Resource) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to allocate descriptor sets!");
		}

		return descriptorSet;
	}

	void DescriptorSet::Destroy(DescriptorSet descriptorSet)
	{
		vkDestroyDescriptorPool(VkContext::API->LogicalDevice, descriptorSet.Pool, nullptr);
		descriptorSet.Resource = nullptr;
	}

	void DescriptorSet::BindBuffer(const uint32_t bindingIndex, const VkBuffer buffer, const uint32_t offset, const uint32_t range) const
	{
		VkDescriptorBufferInfo bufferInfo;

		bufferInfo.buffer = buffer;
		bufferInfo.offset = offset;
		bufferInfo.range = range;

		MK_LOG_INFO("BUFFER, {0}", range);

		const auto& [Type, Count, Flags, ShaderStages] = m_layout.GetBindingInfo(bindingIndex);

		VkWriteDescriptorSet descriptorWrite { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptorWrite.dstSet = Resource;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = Type;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(VkContext::API->LogicalDevice, 1,
			&descriptorWrite, 0, nullptr);
	}

	void DescriptorSet::BindCombinedImageSampler(const uint32_t bindingIndex, const VkImageView imageView,
	                                             const VkSampler sampler) const
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;


		const auto& [Type, Count, Flags, ShaderStages] = m_layout.GetBindingInfo(bindingIndex);

		VkWriteDescriptorSet descriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };		descriptorWrite.dstSet = Resource;
				descriptorWrite.dstSet = Resource;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = Type;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(VkContext::API->LogicalDevice, 1,
			&descriptorWrite, 0, nullptr);
	}

}
