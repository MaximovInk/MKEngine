#include "mkpch.h"
#include "MKEngine/Core/Log.h"
#include "contentPipeline.h"

namespace MKEngine {
    std::vector<char> ContentPipeline::readFile(const std::string& filePath)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            MK_LOG_ERROR("failed to open file: {}", filePath);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
}