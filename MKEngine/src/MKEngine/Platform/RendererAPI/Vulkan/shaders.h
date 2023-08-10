#pragma once
#include "mkpch.h"
#include "MKEngine/Core/Log.h"
#include "vulkan/vulkan.h"
#include "device.h"

namespace MKEngine {

	class VulkanDevice; 

	struct Shader {
		VkShaderModule resource = VK_NULL_HANDLE;
		//VkShaderStageFlags stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		//std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};
		//VkPushConstantRange pushConstants{};
	};

	struct ShaderCreateDesc
	{
		const char* pPath = "";
	};

	Shader createShader(
		VulkanDevice& device,
		ShaderCreateDesc desc);

	void destroyShader(
		VulkanDevice& device,
		Shader& shader);
}