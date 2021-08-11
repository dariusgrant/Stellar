#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (std140, binding = 0) uniform bufferVals {
    mat4 mvp;
} myBufferVals;
layout (location = 0) in vec4 pos;
layout (location = 0) out vec4 outColor;

void main() {
    gl_Position = myBufferVals.mvp * pos;
    outColor = vec4(pos.x, pos.y, pos.z, 255);
}