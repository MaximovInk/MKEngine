#pragma once
#include <vulkan/vulkan.h>
#include "graphicsPipelineDescription.h"

namespace MKEngine {

	VkPipelineLayout CreatePipelineLayout(const VkDevice device, const VkDescriptorSetLayout descriptorSetLayout);


	struct GraphicsPipeline {
	public:
		VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
		VkPipeline Reference = VK_NULL_HANDLE;
		//VkDescriptorSetLayout DescriptorSetLayout;

		static void DestroyGraphicsPipeline(GraphicsPipeline& graphicsPipeline);
		static GraphicsPipeline CreateGraphicsPipeline(GraphicsPipelineDescription& description);

	private:
		static VkPipelineInputAssemblyStateCreateInfo GetInputAssembly(GraphicsPipelineDescription& description);
		static std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages(GraphicsPipelineDescription& description);
		static void DestroyShaders(GraphicsPipelineDescription& description);
		static VkPipelineRasterizationStateCreateInfo GetRasterizationState(GraphicsPipelineDescription& description);
		static VkPipelineMultisampleStateCreateInfo GetMultisampleState(GraphicsPipelineDescription& description);
		static VkPipelineColorBlendStateCreateInfo GetColorBlendState(GraphicsPipelineDescription& description, std::vector<VkPipelineColorBlendAttachmentState>& colorAttachmentsBlending);
		static VkPipelineRenderingCreateInfo GetDynamicRenderingInfo(GraphicsPipelineDescription& description, std::vector<VkFormat>& colorAttachmentsFormat);
		static void GetColorAttachments(GraphicsPipelineDescription& description, std::vector<VkFormat>& colorAttachmentsFormat, std::vector<VkPipelineColorBlendAttachmentState>& colorAttachmentsBlending);
		static VkPipelineDepthStencilStateCreateInfo GetDepthStencilState(GraphicsPipelineDescription& description);
	};


	



}