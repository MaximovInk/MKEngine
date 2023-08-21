#include "mkpch.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "vk_mem_alloc.h"

#include "vkState.h"
#include "vkFunctions.h"
#include "vkExtern.h"

std::vector<char> ReadFile(std::string filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		MK_LOG_ERROR("Cannot read file: {0}", filename);
	}

	const size_t size(static_cast<size_t>(file.tellg()));

	std::vector<char> buffer(size);
	file.seekg(0);
	file.read(buffer.data(), size);

	file.close();

	return buffer;
}
namespace MKEngine {

	VkPipelineLayout CreatePipelineLayout(const VkDevice device, const VkDescriptorSetLayout descriptorSetLayout) {
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

	VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDevice device)
	{
		//UniformBuffers
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		//Sampler
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


		//Create descriptorSetLayout
		const std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to create descriptor set layout!");
		}
		return descriptorSetLayout;
	}

	VkRenderPass CreateRenderPass(const VkDevice device, const VkFormat format) {
		VkRenderPass renderPass;
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			MK_LOG_ERROR("Failed to create render pass");
		}

		return renderPass;
	}

	Pipeline CreateGraphicsPipeline(const GraphicsPipelineDescescription& description)
	{
		VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		//Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		auto bindingDescription = GetBindingDescription();
		auto attributeDescriptions = GetAttributeDescriptions();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		createInfo.pVertexInputState = &vertexInputInfo;

		//Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		createInfo.pInputAssemblyState = &inputAssembly;

		//Vertex Shader
		ShaderCreateDescription vertexShaderDesc;
		vertexShaderDesc.Path = "shaders/vert.spv";
		Shader vertexShader = CreateShader(vertexShaderDesc);
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertexShader.Resource;
		vertShaderStageInfo.pName = "main";
		shaderStages.push_back(vertShaderStageInfo);

		//Viewport and scissor - Dynamic state
		VkViewport viewport = { 0.0, 0.0, 32.0, 32.0, 0.0, 1.0 };
		VkDynamicState dynamicState[] = { VK_DYNAMIC_STATE_VIEWPORT , VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineViewportStateCreateInfo  viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.pViewports = nullptr;
		viewportState.viewportCount = 1;
		viewportState.pScissors = nullptr;
		viewportState.scissorCount = 1;

		VkPipelineDynamicStateCreateInfo  dynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicStateCreateInfo.dynamicStateCount = 2;
		dynamicStateCreateInfo.pDynamicStates = dynamicState;

		createInfo.pViewportState = &viewportState;
		createInfo.pDynamicState = &dynamicStateCreateInfo;

		//Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		createInfo.pRasterizationState = &rasterizer;

		//Fragment shader Shader
		ShaderCreateDescription fragmentShaderDesc;
		fragmentShaderDesc.Path = "shaders/frag.spv";
		Shader fragmentShader = CreateShader(fragmentShaderDesc);
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShader.Resource;
		fragShaderStageInfo.pName = "main";
		shaderStages.push_back(fragShaderStageInfo);
		createInfo.stageCount = shaderStages.size();
		createInfo.pStages = shaderStages.data();

		//Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		createInfo.pMultisampleState = &multisampling;

		//Color blend
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		createInfo.pColorBlendState = &colorBlending;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		//PipelineLayout
		VkDescriptorSetLayout descriptorSetLayout = CreateDescriptorSetLayout(vkState::API->LogicalDevice);
		VkPipelineLayout pipelineLayout = CreatePipelineLayout(vkState::API->LogicalDevice, descriptorSetLayout);
		createInfo.layout = pipelineLayout;

		//Renderpass
		VkRenderPass renderPass = CreateRenderPass(vkState::API->LogicalDevice, description.SwapChainFormat);
		createInfo.renderPass = renderPass;

		//Extra
		createInfo.basePipelineHandle = nullptr;

		//Create graphics pipeline
		VkPipeline vkPipeline;

		if (vkCreateGraphicsPipelines(vkState::API->LogicalDevice, nullptr, 1, &createInfo, nullptr, &vkPipeline) != VK_SUCCESS)
			MK_LOG_ERROR("Failed to create graphics pipeline");

		Pipeline output;
		output.Reference = vkPipeline;
		output.PipelineLayout = pipelineLayout;
		output.RenderPass = renderPass;
		output.DescriptorSetLayout = descriptorSetLayout;

		DestroyShader(fragmentShader);
		DestroyShader(vertexShader);

		return output;
	}

	Buffer CreateBuffer(const BufferDescription& description)
	{
		Buffer buffer;

		const VkDeviceSize bufferSize = description.Size;

		VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = description.Size;
		bufferInfo.usage = description.Usage;

		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		if(description.Data)
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		if (description.Access == DataAccess::Host)
		{
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}

		vmaCreateBuffer(vkState::API->VMAAllocator, &bufferInfo,
			&allocationCreateInfo, &buffer.Resource, &buffer.Allocation, nullptr);

		if (description.Access == DataAccess::Host)
		{
			vmaMapMemory(vkState::API->VMAAllocator, buffer.Allocation, &buffer.MappedData);
		}

		if (description.Data)
		{
			if (description.Access == DataAccess::Host)
			{
				memcpy(buffer.MappedData, description.Data, buffer.Size);
			}
			else
			{
				BufferDescription stagingDescription;
				stagingDescription.Size = bufferSize;
				stagingDescription.Data = description.Data;
				stagingDescription.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingDescription.Access = DataAccess::Host;

				const Buffer stagingBuffer = CreateBuffer(stagingDescription);

				CopyBuffer(stagingBuffer.Resource, buffer.Resource, description.Size);

				DestroyBuffer(stagingBuffer);
			}
		}

		return buffer;
	}

	void DestroyBuffer(const Buffer& buffer)
	{
		if (buffer.MappedData)
		{
			vmaUnmapMemory(vkState::API->VMAAllocator, buffer.Allocation);
		}

		vmaDestroyBuffer(vkState::API->VMAAllocator, buffer.Resource, buffer.Allocation);
	}

	void CopyBuffer( const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
	{
		ImmediateSubmit([&](VkCommandBuffer commandBuffer)
			{
				VkBufferCopy copyRegion{};
				copyRegion.srcOffset = 0; // Optional
				copyRegion.dstOffset = 0; // Optional
				copyRegion.size = size;
				vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
			});
	}

	Shader CreateShader(const ShaderCreateDescription description) 
	{
		Shader shader;
		const auto code = ReadFile(description.Path);
		shader.Resource = VkExtern::CreateShaderModule(vkState::API->LogicalDevice, code);
		return shader;
	}

	void DestroyShader(const Shader& shader) 
	{
		vkDestroyShaderModule(vkState::API->LogicalDevice, shader.Resource, nullptr);
	}

	VkTexture CreateTexture(const TextureDescription& description)
	{
		//Load from file
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(description.Path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		const VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			MK_LOG_ERROR("failed to load texture image!");
		}

		//Create staging buffer
		BufferDescription stagingBufferDescription{};
		stagingBufferDescription.Access = DataAccess::Host;
		stagingBufferDescription.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferDescription.Data = pixels;
		stagingBufferDescription.Size = imageSize;
		const Buffer stagingBuffer = CreateBuffer(stagingBufferDescription);
		//Free stbi image 
		stbi_image_free(pixels);

		//Create image
		VkTexture texture;
		texture.Width = texWidth;
		texture.Height = texHeight;
		VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(texWidth);
		imageInfo.extent.height = static_cast<uint32_t>(texHeight);
		imageInfo.extent.depth = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.mipLevels = description.MipCount;
		imageInfo.format = description.Format;
		imageInfo.initialLayout = description.Layout;
		imageInfo.usage = description.Usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VmaAllocationCreateInfo imageAllocCreateInfo = {};
		imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		vmaCreateImage(vkState::API->VMAAllocator, &imageInfo, &imageAllocCreateInfo, &texture.Resource, &texture.Allocation, nullptr);

		//Set TransitionLayout for copy
		TransitionImageLayout(texture.Resource, description.Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		//Copy from buffer to image
		CopyBufferToImage(stagingBuffer, texture);

		//Set TransitionLayout for shaders 
		TransitionImageLayout(texture.Resource, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		DestroyBuffer(stagingBuffer);

		//Create ImageView
		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = texture.Resource;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vkState::API->LogicalDevice, &viewInfo, nullptr, &texture.View) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to create texture image view!");
		}

		//Create Sampler
		VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = vkState::API->Properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(vkState::API->LogicalDevice, &samplerInfo, nullptr, &texture.Sampler) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to create texture sampler!");
		}

		return texture;
	}

	void DestroyTexture(VkTexture texture)
	{
		vkDestroySampler(vkState::API->LogicalDevice, texture.Sampler, nullptr);
		vkDestroyImageView(vkState::API->LogicalDevice, texture.View, nullptr);

		vmaDestroyImage(vkState::API->VMAAllocator, texture.Resource, texture.Allocation);
	}

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		ImmediateSubmit([&](VkCommandBuffer commandBuffer)
			{
				VkPipelineStageFlags sourceStage;
				VkPipelineStageFlags destinationStage;

				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = oldLayout;
				barrier.newLayout = newLayout;

				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

				barrier.image = image;
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;

				if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

					sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				}
				else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
					destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else {
					MK_LOG_ERROR("unsupported layout transition!");
				}

				vkCmdPipelineBarrier(
					commandBuffer,
					sourceStage, destinationStage,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);
			});


	}

	void CopyBufferToImage(Buffer buffer, VkTexture image)
	{
		ImmediateSubmit([&](VkCommandBuffer commandBuffer)
			{
				VkBufferImageCopy region{};
				region.bufferOffset = 0;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;

				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel = 0;
				region.imageSubresource.baseArrayLayer = 0;
				region.imageSubresource.layerCount = 1;

				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = {
					image.Width,
					image.Height,
					1
				};

				vkCmdCopyBufferToImage(
					commandBuffer,
					buffer.Resource,
					image.Resource,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region
				);
			});

	}

	VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, Position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, Color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, TexCoord);

		return attributeDescriptions;
	}

	void ImmediateSubmit(std::function<void(VkCommandBuffer)> const& callback)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = vkState::API->CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(vkState::API->LogicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to allocate command buffer for immediate submit!");
		}

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		callback(commandBuffer);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(vkState::API->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(vkState::API->GraphicsQueue);

		vkFreeCommandBuffers(vkState::API->LogicalDevice, vkState::API->CommandPool, 1, &commandBuffer);
	}


}