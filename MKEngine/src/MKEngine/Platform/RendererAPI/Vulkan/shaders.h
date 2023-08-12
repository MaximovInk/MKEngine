#pragma once
#include "mkpch.h"
#include "vulkan/vulkan.h"
#include "device.h"

namespace MKEngine {

	class VulkanDevice; 

	struct Shader {
		VkShaderModule Resource = VK_NULL_HANDLE;
	};

	struct ShaderCreateDesc
	{
		const char* Path = "";
	};

	Shader CreateShader(
		const VulkanDevice& device,
		ShaderCreateDesc desc);

	void DestroyShader(
		const VulkanDevice& device,
		const Shader& shader);
}