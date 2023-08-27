#include "mkpch.h"
#include "graphicsPipeline.h"

#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/RendererAPI/vertex.h"
#include "MKEngine/Platform/RendererAPI/Vulkan/VkContext.h"

namespace MKEngine {
	VkPipelineInputAssemblyStateCreateInfo GraphicsPipeline::GetInputAssembly(GraphicsPipelineDescription& description)
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		return inputAssembly;
	}

	std::vector<VkPipelineShaderStageCreateInfo> GraphicsPipeline::GetShaderStages(GraphicsPipelineDescription& description)
	{
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
		shaderStageCreateInfos.reserve(description.Shaders.size());

		//SHADERS
		for (auto [Resource, Stage, EntryPoint, PushConstants, LayoutBindings] : description.Shaders)
		{
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			shaderStageCreateInfo.pName = EntryPoint;
			shaderStageCreateInfo.module = Resource;
			shaderStageCreateInfo.stage = static_cast<VkShaderStageFlagBits>(Stage);

			shaderStageCreateInfos.push_back(shaderStageCreateInfo);
		}

		return shaderStageCreateInfos;
	}

	void GraphicsPipeline::DestroyShaders(GraphicsPipelineDescription& description)
	{
		for (auto [Resource, Stage, EntryPoint, PushConstants, LayoutBindings] : description.Shaders)
		{
			vkDestroyShaderModule(VkContext::API->LogicalDevice, Resource, nullptr);
		}
	}

    VkPipelineRasterizationStateCreateInfo GraphicsPipeline::GetRasterizationState(GraphicsPipelineDescription& description)
    {
        VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = description.CullMode;
        rasterizer.frontFace = description.FrontFace;
        rasterizer.depthBiasEnable = VK_FALSE;

        return rasterizer;
    }

    VkPipelineMultisampleStateCreateInfo GraphicsPipeline::GetMultisampleState(GraphicsPipelineDescription& description)
    {
        VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        return multisampling;
    }

    VkPipelineColorBlendStateCreateInfo GraphicsPipeline::GetColorBlendState(GraphicsPipelineDescription& description, std::vector<VkPipelineColorBlendAttachmentState>& colorAttachmentsBlending)
    {
        VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = colorAttachmentsBlending.size();
        colorBlending.pAttachments = colorAttachmentsBlending.data();
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        return colorBlending;
    }

    VkPipelineDepthStencilStateCreateInfo GraphicsPipeline::GetDepthStencilState(GraphicsPipelineDescription& description)
    {
        VkPipelineDepthStencilStateCreateInfo depthState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthState.depthTestEnable = description.ColorAttachment.HasDepthAttachment();
        depthState.depthWriteEnable = description.ColorAttachment.HasDepthAttachment() ? description.ColorAttachment.GetDepthAttachmentInfo().WriteEnabled : false;
        depthState.depthCompareOp = description.ColorAttachment.HasDepthAttachment() ? description.ColorAttachment.GetDepthAttachmentInfo().DepthTestPassCondition : VK_COMPARE_OP_ALWAYS;
        depthState.depthBoundsTestEnable = false;
        depthState.stencilTestEnable = false;
        return depthState;
    }

    VkPipelineRenderingCreateInfo GraphicsPipeline::GetDynamicRenderingInfo(GraphicsPipelineDescription& description, std::vector<VkFormat>& colorAttachmentsFormat)
    {
        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        pipelineRenderingCreateInfo.viewMask = 0;
        pipelineRenderingCreateInfo.colorAttachmentCount = colorAttachmentsFormat.size();
        pipelineRenderingCreateInfo.pColorAttachmentFormats = colorAttachmentsFormat.data();
        pipelineRenderingCreateInfo.depthAttachmentFormat = description.ColorAttachment.HasDepthAttachment() ? description.ColorAttachment.GetDepthAttachmentInfo().Format : VK_FORMAT_UNDEFINED;
        pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
        return pipelineRenderingCreateInfo;
    }

    void GraphicsPipeline::GetColorAttachments(GraphicsPipelineDescription& description, std::vector<VkFormat>& colorAttachmentsFormat, std::vector<VkPipelineColorBlendAttachmentState>& colorAttachmentsBlending)
    {
        colorAttachmentsFormat.reserve(description.ColorAttachment.GetColorAttachmentsInfos().size());
        colorAttachmentsBlending.reserve(description.ColorAttachment.GetColorAttachmentsInfos().size());

        for (const auto& [Format, Blending] : description.ColorAttachment.GetColorAttachmentsInfos())
        {
            colorAttachmentsFormat.emplace_back(Format);

            VkPipelineColorBlendAttachmentState blending;
            if (Blending.has_value())
            {
                blending.blendEnable = true;
                blending.srcColorBlendFactor = Blending->SrcColorBlendFactor;
                blending.dstColorBlendFactor = Blending->DstColorBlendFactor;
                blending.colorBlendOp = Blending->ColorBlendOp;
                blending.srcAlphaBlendFactor = Blending->SrcAlphaBlendFactor;
                blending.dstAlphaBlendFactor = Blending->DstAlphaBlendFactor;
                blending.alphaBlendOp = Blending->AlphaBlendOp;
                blending.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                    | VK_COLOR_COMPONENT_A_BIT;
            }
            else
            {
                blending.blendEnable = false;
                blending.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                blending.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                blending.colorBlendOp = VK_BLEND_OP_ADD;
                blending.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                blending.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                blending.alphaBlendOp = VK_BLEND_OP_ADD;
                blending.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                    | VK_COLOR_COMPONENT_A_BIT;
            }

            colorAttachmentsBlending.emplace_back(blending);
        }

    }

	void GraphicsPipeline::DestroyGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
	{
		vkDestroyPipeline(VkContext::API->LogicalDevice, graphicsPipeline.Reference, nullptr);
	}

	GraphicsPipeline GraphicsPipeline::CreateGraphicsPipeline(GraphicsPipelineDescription& description)
	{
		VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

		auto shaderStageCreateInfos = GetShaderStages(description);
		createInfo.stageCount = shaderStageCreateInfos.size();
		createInfo.pStages = shaderStageCreateInfos.data();

		//Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInput = description.VertexInput.Get();
		createInfo.pVertexInputState = &vertexInput;

		//Input Assembly
		auto inputAssembly = GetInputAssembly(description);
		createInfo.pInputAssemblyState = &inputAssembly;

		//Viewport and scissor - Dynamic state

		VkViewport viewport = { 0.0, 0.0, 32.0, 32.0, 0.0, 1.0 };

		VkPipelineViewportStateCreateInfo  viewportStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportStateCreateInfo.pViewports = nullptr;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pScissors = nullptr;
		viewportStateCreateInfo.scissorCount = 1;

		VkPipelineDynamicStateCreateInfo  dynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		std::vector<VkDynamicState> dynamicStates;
		dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);

		dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
		dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

		createInfo.pViewportState = &viewportStateCreateInfo;
		createInfo.pDynamicState = &dynamicStateCreateInfo;

		//Rasterizer
		auto rasterizer = GetRasterizationState(description);
		createInfo.pRasterizationState = &rasterizer;

		//Multisampling
		auto multisampling = GetMultisampleState(description);
		createInfo.pMultisampleState = &multisampling;

		//Depth stencil
		auto depthStencil = GetDepthStencilState(description);
		createInfo.pDepthStencilState = &depthStencil;

		//Pipeline layout
		auto pipelineLayout = description.PipelineLayout;
		createInfo.layout = pipelineLayout;
		

		std::vector<VkFormat> colorAttachmentsFormat;
		std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentsBlending;
		GetColorAttachments(description, colorAttachmentsFormat, colorAttachmentsBlending);

		//Color attachments
		auto colorBlendState = GetColorBlendState(description, colorAttachmentsBlending);
		createInfo.pColorBlendState = &colorBlendState;

		//Dynamic rendering
		auto dynamicRendering = GetDynamicRenderingInfo(description, colorAttachmentsFormat);
		createInfo.pNext = &dynamicRendering;

		//Create graphics pipeline
		VkPipeline vkPipeline;

		if (vkCreateGraphicsPipelines(VkContext::API->LogicalDevice, nullptr, 1, &createInfo, nullptr, &vkPipeline) != VK_SUCCESS)
			MK_LOG_ERROR("Failed to create graphics pipeline");

		GraphicsPipeline output;
		output.Reference = vkPipeline;
		output.PipelineLayout = pipelineLayout;

		//Maybe not destroy for multiply usage
		DestroyShaders(description);

		return output;
	}
    VkPipelineLayout CreatePipelineLayout(const VkDevice device, const VkDescriptorSetLayout descriptorSetLayout)
	{
		VkPipelineLayout pipelineLayout;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		VkPushConstantRange pushConstantInfo;
		pushConstantInfo.offset = 0;
		pushConstantInfo.size = sizeof(ObjectData);
		pushConstantInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantInfo;

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			MK_LOG_ERROR("Failed to create pipeline layout");
		}

		return pipelineLayout;
	}
}
