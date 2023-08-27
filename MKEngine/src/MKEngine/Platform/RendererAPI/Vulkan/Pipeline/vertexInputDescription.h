#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
	/*
	 		//Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		auto bindingDescription = GetBindingDescription();
		auto attributeDescriptions = GetAttributeDescriptions();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		createInfo.pVertexInputState = &vertexInputInfo;
	 
	 */

	class VertexInputDesc
	{
	public:
		void VertexDefineSlot(uint32_t bufferSlot, int stride, VkVertexInputRate inputRate);
		void DefineAttribute(uint32_t bufferSlot, uint32_t attributeLocation, VkFormat dataType, uint32_t offset);

		VkPipelineVertexInputStateCreateInfo Get() const;

	private:
		std::vector<VkVertexInputAttributeDescription> m_attributes;
		std::vector<VkVertexInputBindingDescription> m_slots;

	};



}