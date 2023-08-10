#include "mkpch.h"
#include "shaders.h"
#include "vkExtern.h"

namespace MKEngine {


    std::vector<char> readFile(std::string filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            MK_LOG_ERROR("Cannot read file: {0}", filename);
        }

        size_t filesize(static_cast<size_t>(file.tellg()));

        std::vector<char> buffer(filesize);
        file.seekg(0);
        file.read(buffer.data(), filesize);

        file.close();

        return buffer;
    }


    Shader createShader(VulkanDevice& device, ShaderCreateDesc desc)
    {
        Shader shader;

        /*
        
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = VkExtern::createShaderModule(device.LogicalDevice, vertShaderCode);
        VkShaderModule fragShaderModule = VkExtern::createShaderModule(device.LogicalDevice, fragShaderCode);

        */
        auto code = readFile(desc.pPath);

        shader.resource = VkExtern::createShaderModule(device.LogicalDevice, code);

        return shader;
    }

    void destroyShader(VulkanDevice& device, Shader& shader)
    {
        vkDestroyShaderModule(device.LogicalDevice, shader.resource, nullptr);
    }
}