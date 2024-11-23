#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
layout(location=2) in vec2 texCoord;

layout(location=0) out vec4 fragColor;
layout(location=1) out vec2 fragTexcoord;

layout(set=0, binding=0) uniform UBO{
    mat4 view;
    mat4 project;
}ubo;
layout(set=1,binding=0) uniform ModelBlock{
    mat4 model;
}modelUniform;

void main(){
    gl_Position=ubo.project*ubo.view*vec4(position, 1.0);
    fragColor=vec4(color, 1.0);
    fragTexcoord=texCoord;
}