#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 texCoord;
layout(location=4) in vec4 jointIndices;
layout(location=5) in vec4 jointWeights;

layout(location=0) out vec2 fragTexcoord;
layout(location=1) out vec3 FragPos;
layout(location=2) out vec3 Normal;
layout(location=3) out vec3 lightColor;
layout(location=4) out vec3 lightPos;
layout(location=5) out vec3 viewPos;
layout(location=6) out vec4 lightSpace_Pos;


layout(set=0, binding=0) uniform UBO{
    mat4 view;
    mat4 project;
    mat4 lightSpace;
    vec4 lightPos;
    vec4 lightColor;
    vec4 viewPos;
}ubo;

layout(push_constant) uniform PushConstant{
    mat4 model;
}primitive;

layout(set=1,bind=0)readonly buffer JointMatrices{
    mat4 jointMatrices[];
};


const mat4 bias = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 1.0, 0.0,
0.5, 0.5, 0.0, 1.0);

void main(){
    mat4 model=primitive.model;

    FragPos=vec3(model*vec4(position, 1.0));
    Normal=mat3(transpose(inverse(model)))*normal;

    fragTexcoord=texCoord;
    lightColor=ubo.lightColor.xyz;
    viewPos=ubo.viewPos.xyz;
    lightPos=ubo.lightPos.xyz;

    lightSpace_Pos=bias*ubo.lightSpace*primitive.model*vec4(position, 1.0);
    gl_Position=ubo.project*ubo.view*model*vec4(position, 1.0);
}