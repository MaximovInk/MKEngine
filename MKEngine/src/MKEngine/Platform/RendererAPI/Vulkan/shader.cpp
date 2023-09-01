#include "mkpch.h"

#include "spirv_reflect.h"

#include "VkContext.h"

#include "shader.h"
#include "vkExtern.h"
#include "MKEngine/Core/Log.h"

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
    Shader Shader::CreateShader(ShaderCreateDescription description)
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

    void Shader::DestroyShader(const Shader& shader)
    {
		vkDestroyShaderModule(VkContext::API->LogicalDevice, shader.Resource, nullptr);
    }
}