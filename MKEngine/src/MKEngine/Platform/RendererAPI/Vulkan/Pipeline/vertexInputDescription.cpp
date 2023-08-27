#include "mkpch.h"
#include "vertexInputDescription.h"

namespace MKEngine {

	void VertexInputDesc::VertexDefineSlot(const uint32_t bufferSlot, const int stride, const VkVertexInputRate inputRate)
	{
		VkVertexInputBindingDescription slotDescription;
		slotDescription.binding = bufferSlot;
		slotDescription.stride = stride;
		slotDescription.inputRate = inputRate;

		m_slots.emplace_back(slotDescription);
	}

	void VertexInputDesc::DefineAttribute(const uint32_t bufferSlot, const uint32_t attributeLocation, VkFormat dataType, const uint32_t offset)
	{
		VkVertexInputAttributeDescription attributeDescription;
		attributeDescription.binding = bufferSlot;
		attributeDescription.location = attributeLocation;
		attributeDescription.format = dataType;
		attributeDescription.offset = offset;

		m_attributes.emplace_back(attributeDescription);
	}

	VkPipelineVertexInputStateCreateInfo VertexInputDesc::Get() const
	{
		VkPipelineVertexInputStateCreateInfo description{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		description.vertexBindingDescriptionCount = m_slots.size();
		description.pVertexBindingDescriptions = m_slots.data();
		description.vertexAttributeDescriptionCount = m_attributes.size();
		description.pVertexAttributeDescriptions = m_attributes.data();

		return description;
	}

}