#pragma once
#include <vulkan/vulkan.h>

namespace MKEngine
{
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