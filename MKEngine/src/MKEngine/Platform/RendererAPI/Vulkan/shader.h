#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace MKEngine {

	struct Shader {
		VkShaderModule Resource = VK_NULL_HANDLE;
		VkShaderStageFlags Stage;
		const char* EntryPoint = "main";
		VkPushConstantRange PushConstants{};
		std::vector<VkDescriptorSetLayoutBinding> LayoutBindings{};
	};

	struct ShaderCreateDescription
	{
		const char* Path = "";
	};


}
