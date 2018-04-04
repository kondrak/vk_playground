#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 ModelViewProjectionMatrix;
} ubo;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexCoord;
layout(location = 0) out vec2 TexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.ModelViewProjectionMatrix * vec4(inVertex, 1.0);
    TexCoord = inTexCoord;
}
