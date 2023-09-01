#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace MKEngine {

	struct ShaderCreateDescription
	{
		const char* Path = "";
	};

	struct Shader {
		VkShaderModule Resource = VK_NULL_HANDLE;
		VkShaderStageFlags Stage;
		const char* EntryPoint = "main";
		VkPushConstantRange PushConstants{};
		std::vector<VkDescriptorSetLayoutBinding> LayoutBindings{};

		static Shader CreateShader(ShaderCreateDescription description);
		static void DestroyShader(const Shader& shader);
	};

	

}
