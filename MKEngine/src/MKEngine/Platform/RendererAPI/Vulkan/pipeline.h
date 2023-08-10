#pragma once
#include "mkpch.h"
#include "vulkan/vulkan.h"

namespace MKEngine {

	struct Pipeline {
		//VkPipelineBindPoint type = VK_PIPELINE_BIND_POINT_MAX_ENUM;
		//VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		//VkDescriptorUpdateTemplate updateTemplate = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		//VkPushConstantRange pushConstants{};
		VkRenderPass renderPass;
	};

	struct RasterizationDesc
	{
		VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;         
		VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; 
	};

	struct DepthStencilDesc
	{
		bool bDepthTestEnable = false;                       
		bool bDepthWriteEnable = false;                      
		VkCompareOp depthCompareOp = VK_COMPARE_OP_GREATER;  
	};

	struct ColorAttachmentDesc
	{
		VkFormat format = VK_FORMAT_UNDEFINED;  
		bool bBlendEnable = false;              
	};


	struct AttachmentLayout
	{
		std::vector<ColorAttachmentDesc> colorAttachments;
		VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
	};

	struct GraphicsPipelineDesc
	{
		//Shaders shaders;                     
		AttachmentLayout attachmentLayout{};  
		RasterizationDesc rasterization{};    
		DepthStencilDesc depthStencil{};   

		VkExtent2D swapChainExtent;
		VkFormat swapChainFormat;
	};
}