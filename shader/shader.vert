#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 texCoord;

layout(location=0) out vec2 fragTexcoord;
layout(location=1) out vec3 FragPos;
layout(location=2) out vec3 Normal;

layout(set=0, binding=0) uniform UBO{
    mat4 view;
    mat4 project;
}ubo;

layout(set=1,binding=0) uniform ModelBlock{
    mat4 model;
}modelUniform;

void main(){
    mat4 model=modelUniform.model;
    FragPos=vec3(model*vec4(position,1.0));
    Normal=mat3(transpose(inverse(model)))*normal;
    fragTexcoord=texCoord;
    gl_Position=ubo.project*ubo.view*vec4(position, 1.0);
}