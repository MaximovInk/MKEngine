#pragma once

//#include <array>
//#include <vulkan/vulkan_core.h>
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace MKEngine {

    struct ObjectData {
        glm::mat4 Model;
    };

    struct Vertex
    {
        glm::vec2 Position;
        glm::vec3 Color;
        glm::vec2 TexCoord;

      
    };

    const std::vector<Vertex> VERTICES = {
     {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
     {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
     {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
     {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };

    const std::vector<uint16_t> INDICES = {
    0, 1, 2, 2, 3, 0
    };

}