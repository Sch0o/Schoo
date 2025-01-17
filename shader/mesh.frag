#version 450

layout(location=0) in vec2 fragTexCoord;
layout(location=1) in vec3 FragPos;
layout(location=2) in vec3 Normal;
layout(location=3) in vec3 lightColor;
layout(location=4) in vec3 lightPos;
layout(location=5) in vec3 viewPos;
layout(location=6) in vec4 lightSpace_pos;

layout(location=0) out vec4 outcolor;

layout(set=0, binding=1) uniform sampler2D shadowMap;
layout(set=1, binding=0) uniform sampler2D texture0;

float pcf(vec4 lightSpace_pos,int range){
    float shadow=0.0;
    //透视除法
    vec3 projCoords=lightSpace_pos.xyz/lightSpace_pos.w;
    float currentDepth=projCoords.z;
    vec2 texelSize=1.0/textureSize(shadowMap,0);
    for(int x=-range;x<=range;x++){
        for(int y=-range;y<=range;y++){
            if ( currentDepth > 0 && currentDepth < 1.0 )
            {
                float closestDepth=texture(shadowMap,projCoords.xy+vec2(x,y)*texelSize).r;
                if(currentDepth-0.0012 > closestDepth)
                    shadow+=1;
            }

        }
    }
    float f=range+range+1;
    float nums=f*f;
    return shadow/nums;

}
vec3 Blinn_Phong(vec3 lightDir,vec3 normal,vec3 viewDir,vec3 lightColor,float shadow){
    //ambient
    float ambientStrength=0.1;
    vec3 ambient = ambientStrength * lightColor;
    //diffuse
    float diffStrength=max(dot(normal,lightDir),0.0);
    vec3 diffuse=diffStrength*lightColor;
    //specular
    float specularStrength = 0.5;
    vec3 reflectDir=reflect(-lightDir,normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    return ambient+(1.0-shadow)*(diffuse+specular);
}
vec3 NPR(vec3 lightDir,vec3 normal,vec3 viewDir,vec3 lightColor,float shadow){
    float shadowRange=0.24;
    vec3 shadowColor=vec3(0.7,0.7,0.8);
    float shadowSmooth=0.4;
    //ambient
    float ambientStrength=0.1;
    vec3 ambient = ambientStrength * lightColor;
    //diffuse
    float halfLambert=dot(normal,lightDir)*0.5+0.5;
    float ramp=smoothstep(0,shadowSmooth,halfLambert-shadowRange);
    vec3 diffuse=(1-ramp)*shadowColor+ramp*lightColor;
    //specular
    float specularStrength = 0.5;
    vec3 reflectDir=reflect(-lightDir,normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    return ambient+(1.0-shadow)*(diffuse+specular);
}

void main(){
    vec4 baseColor =texture(texture0,fragTexCoord).rgba;

    vec3 lightDir =normalize(lightPos-FragPos);
    vec3 normal=normalize(Normal);
    vec3 viewDir=normalize(viewPos-FragPos);
    //shadow
    float shadow=pcf(lightSpace_pos,3);

    vec3 lighting=Blinn_Phong(lightDir,normal,viewDir,lightColor,shadow);
    //vec3 lighting=NPR(lightDir,normal,viewDir,lightColor,shadow);
    outcolor=vec4(lighting,1.0f)*baseColor;

}