#include "mkpch.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "vk_mem_alloc.h"

#include "spirv_reflect.h"

#include "VkContext.h"
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
	/*
	 
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

	 
	 
	 */


	static VkShaderStageFlags GetShaderStage(
		const SpvReflectShaderStageFlagBits reflectShaderStage)
	{
		switch (reflectShaderStage)
		{
		case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
			return VK_SHADER_STAGE_COMPUTE_BIT;

		case SPV_REFLECT_SHADER_STAGE_TASK_BIT_NV:
			return VK_SHADER_STAGE_TASK_BIT_NV;

		case SPV_REFLECT_SHADER_STAGE_MESH_BIT_NV:
			return VK_SHADER_STAGE_MESH_BIT_NV;

		case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
			return VK_SHADER_STAGE_VERTEX_BIT;

		case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;

		default:
			MK_LOG_ERROR(!"Unsupported SpvReflectShaderStageFlagBits!");
			return {};
		}
	}

	static VkDescriptorType GetDescriptorType(
		const SpvReflectDescriptorType reflectDescriptorType)
	{
		switch (reflectDescriptorType)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			break;
		case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
			break;
		}

		MK_LOG_ERROR(!"Unsupported SpvReflectDescriptorType!");
		return {};
	}

	Shader CreateShader(const ShaderCreateDescription description) 
	{
		Shader shader;
		const auto code = ReadFile(description.Path);
		
		SpvReflectShaderModule spvModule;
		spvReflectCreateShaderModule(code.size(), code.data(), &spvModule);

		if (spvModule.entry_point_count < 1)
			MK_LOG_ERROR("Shader has not entry points: {0}", description.Path);

		shader.Stage = GetShaderStage(spvModule.shader_stage);
		shader.Resource = VkExtern::CreateShaderModule(VkContext::API->LogicalDevice, code);

		if (spvModule.push_constant_block_count > 0 && spvModule.push_constant_blocks != nullptr)
		{
			shader.PushConstants.stageFlags = shader.Stage;
			shader.PushConstants.offset = spvModule.push_constant_blocks->offset;
			shader.PushConstants.size = spvModule.push_constant_blocks->size;
		}

		uint32_t spvBindingCount = 0;
		spvReflectEnumerateDescriptorBindings(&spvModule, &spvBindingCount, nullptr);
		std::vector<SpvReflectDescriptorBinding*> spvBindings(spvBindingCount);
		spvReflectEnumerateDescriptorBindings(&spvModule, &spvBindingCount, spvBindings.data());

		shader.LayoutBindings.reserve(spvBindingCount);
		for (uint32_t layoutBindingIndex = 0; layoutBindingIndex < spvBindingCount; ++layoutBindingIndex)
		{
			VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
			descriptorSetLayoutBinding.binding = spvBindings[layoutBindingIndex]->binding;
			descriptorSetLayoutBinding.descriptorCount = 1;
			descriptorSetLayoutBinding.descriptorType = GetDescriptorType(spvBindings[layoutBindingIndex]->descriptor_type);
			descriptorSetLayoutBinding.stageFlags = shader.Stage;
			descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

			shader.LayoutBindings.push_back(descriptorSetLayoutBinding);
		}

		spvReflectDestroyShaderModule(&spvModule);



		return shader;
	}

	void DestroyShader(const Shader& shader) 
	{
		vkDestroyShaderModule(VkContext::API->LogicalDevice, shader.Resource, nullptr);
	}

	VkTexture CreateTexture(const TextureDescription& description)
	{

		MK_LOG_INFO("CREATING TEXTURE");
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
		const Buffer stagingBuffer = Buffer::Create(stagingBufferDescription);
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

		vmaCreateImage(VkContext::API->VMAAllocator, &imageInfo, &imageAllocCreateInfo, &texture.Resource, &texture.Allocation, nullptr);

		//Set TransitionLayout for copy
		TransitionImageLayout(texture.Resource, description.Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		//Copy from buffer to image
		CopyBufferToImage(stagingBuffer, texture);

		//Set TransitionLayout for shaders 
		TransitionImageLayout(texture.Resource, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		Buffer::Destroy(stagingBuffer);

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

		if (vkCreateImageView(VkContext::API->LogicalDevice, &viewInfo, nullptr, &texture.View) != VK_SUCCESS) {
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
		samplerInfo.maxAnisotropy = VkContext::API->Properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(VkContext::API->LogicalDevice, &samplerInfo, nullptr, &texture.Sampler) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to create texture sampler!");
		}

		return texture;
	}

	void DestroyTexture(VkTexture texture)
	{
		MK_LOG_INFO("DEstroying texture {0}, {1}, {2}", texture.Sampler != VK_NULL_HANDLE, texture.View != VK_NULL_HANDLE, texture.Resource != VK_NULL_HANDLE);

		 if(texture.Sampler != VK_NULL_HANDLE)
		 vkDestroySampler(VkContext::API->LogicalDevice, texture.Sampler, nullptr);
		if (texture.View != VK_NULL_HANDLE)
		 vkDestroyImageView(VkContext::API->LogicalDevice, texture.View, nullptr);
		if (texture.Resource != VK_NULL_HANDLE)
		 vmaDestroyImage(VkContext::API->VMAAllocator, texture.Resource, texture.Allocation);
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

	void ImmediateSubmit(std::function<void(VkCommandBuffer)> const& callback)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VkContext::API->CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(VkContext::API->LogicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
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

		vkQueueSubmit(VkContext::API->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(VkContext::API->GraphicsQueue);

		vkFreeCommandBuffers(VkContext::API->LogicalDevice, VkContext::API->CommandPool, 1, &commandBuffer);
	}


}