#include "mkpch.h"
#include "shaders.h"
#include "vkExtern.h"

namespace MKEngine {

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


    Shader CreateShader(const VulkanDevice& device, const ShaderCreateDesc desc)
    {
        Shader shader;
        const auto code = ReadFile(desc.Path);
        shader.Resource = VkExtern::CreateShaderModule(device.LogicalDevice, code);
        return shader;
    }

    void DestroyShader(const VulkanDevice& device, const Shader& shader)
    {
        vkDestroyShaderModule(device.LogicalDevice, shader.Resource, nullptr);
    }
}