#pragma once
#include "vulkan/vulkan.h"

namespace MKEngine {

	struct Pipeline {
		VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
		VkPipeline Reference = VK_NULL_HANDLE;
		VkRenderPass RenderPass;
	};

	struct RasterizationDesc
	{
		VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;         
		VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; 
	};

	struct DepthStencilDesc
	{
		bool DepthTestEnable = false;                       
		bool DepthWriteEnable = false;                      
		VkCompareOp DepthCompareOp = VK_COMPARE_OP_GREATER;  
	};

	struct ColorAttachmentDesc
	{
		VkFormat Format = VK_FORMAT_UNDEFINED;  
		bool BlendEnable = false;              
	};


	struct AttachmentLayout
	{
		std::vector<ColorAttachmentDesc> ColorAttachments;
		VkFormat DepthStencilFormat = VK_FORMAT_UNDEFINED;
	};

	struct GraphicsPipelineDesc
	{
		//Shaders shaders;                     
		AttachmentLayout AttachmentLayout{};  
		RasterizationDesc Rasterization{};    
		DepthStencilDesc DepthStencil{};   

		VkExtent2D SwapChainExtent;
		VkFormat SwapChainFormat;
	};
}