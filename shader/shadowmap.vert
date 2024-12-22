#version 450

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO
{
    mat4 depthMVP;
} ubo;

layout(push_constant) uniform PushConstant{
    mat4 model;
}primitive;

void main()
{
    gl_Position =  ubo.depthMVP * primitive.model*vec4(inPos, 1.0);
}