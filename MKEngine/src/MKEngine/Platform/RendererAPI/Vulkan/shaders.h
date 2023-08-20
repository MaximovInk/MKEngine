#pragma once

namespace MKEngine {

	struct Shader {
		VkShaderModule Resource = VK_NULL_HANDLE;
	};

	struct ShaderCreateDescription
	{
		const char* Path = "";
	};


}