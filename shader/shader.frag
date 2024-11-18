#version 450

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 fragTexCoord;

layout(location=0) out vec4 outcolor;

layout(set=0, binding=1) uniform sampler2D texSampler;

void main(){
    outcolor=texture(texSampler,fragTexCoord);
}