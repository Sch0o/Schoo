#version 450

layout(location=0) in vec2 fragTexCoord;
layout(location=1) in vec3 FragPos;
layout(location=2) in vec3 Normal;

layout(location=0) out vec4 outcolor;

layout( set=0,binding=1) uniform Constant{
    vec3 lightPos;
    vec3 lightColor;
    vec3 viewPos;
}constant;

layout(set=1, binding=1) uniform sampler2D texSampler;

void main(){
    vec3 lightColor=constant.lightColor;

    //ambient
    float ambientStrength=0.1;
    vec3 ambient = ambientStrength * lightColor;
    //diffuse
    vec3 norm=normalize(Normal);
    vec3 lightDir =normalize(constant.lightPos-FragPos);
    float diffStrength=max(dot(norm,lightDir),0.0);
    vec3 diffuse=diffStrength*lightColor;
    //specular
    float specularStrength = 0.5;
    vec3 viewDir=normalize(constant.viewPos-FragPos);
    vec3 reflectDir=reflect(-lightDir,norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    outcolor=vec4(ambient+diffuse+specular,1.0)*texture(texSampler,fragTexCoord);
}