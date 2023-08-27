#pragma once
#include <vulkan/vulkan.h>
#include "../shader.h"

#include "vertexInputDescription.h"
#include "pipelineColorAttachment.h"


namespace MKEngine
{
	struct GraphicsPipelineDescription
	{
		std::vector<Shader> Shaders;
		//AttachmentLayout AttachmentLayout{};  
		//DepthStencilDesc DepthStencil{};
		VertexInputDesc VertexInput;
		PipelineColorAttachment ColorAttachment;

		VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
		VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPipelineLayout PipelineLayout;
		VkDescriptorSetLayout DescriptorSetLayout;

		

		//VkExtent2D SwapChainExtent;
		//VkFormat SwapChainFormat;
	};

}