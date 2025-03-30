#version 450

layout (location = 0) in vec3 inPos;
layout(location=4) in vec4 jointIndices;
layout(location=5) in vec4 jointWeights;

layout (set=0, binding = 0) uniform UBO
{
    mat4 depthMVP;
} ubo;

layout(push_constant) uniform PushConstant{
    mat4 model;
}primitive;

layout(set=2, binding=0) readonly buffer JointMatricesBlock{
    mat4 jointMatrices[];
}jmBuffer;

void main()
{
    //linear combination
    mat4 skinMat=jointWeights.x*jmBuffer.jointMatrices[int(jointIndices.x)]
    +jointWeights.y*jmBuffer.jointMatrices[int(jointIndices.y)]
    +jointWeights.z*jmBuffer.jointMatrices[int(jointIndices.z)]
    +jointWeights.w*jmBuffer.jointMatrices[int(jointIndices.w)];

    gl_Position =  ubo.depthMVP * primitive.model * skinMat * vec4(inPos, 1.0);
}