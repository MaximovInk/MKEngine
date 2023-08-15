#version 450
//! #extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec3 fragColor;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout (push_constant) uniform constants{
    mat4 model;
} ObjectData;


void main() {
    gl_Position = ObjectData.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}