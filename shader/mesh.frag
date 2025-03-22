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

#define PI 3.141592653589793
#define PI2 6.283185307179586
#define PI_HALF 1.5707963267948966
#define RECIPROCAL_PI 0.3183098861837907
#define RECIPROCAL_PI2 0.15915494309189535
#define EPSILON 1e-6
#define bias 0.0012
//PCSS
#define NUM_SAMPLES 16
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10
#define LIGHT_WIDTH 100
vec2 poissonDisk[NUM_SAMPLES];

float rand_1to1(float x){
    //(-1,1)
    return fract(sin(x)*10000.0);
}

float rand_2to1(vec2 uv){
    //(0,1)
    float a=12.9898, b=78.233, c=43758.5453;
    float dt=dot(uv.xy, vec2(a, b));
    float sn=mod(dt, PI);
    return fract(sin(sn)*c);
}

void poissonDiskSamples(const in vec2 randomSeed) {

    float ANGLE_STEP = PI2 * float(NUM_RINGS) / float(NUM_SAMPLES);
    float INV_NUM_SAMPLES = 1.0 / float(NUM_SAMPLES);

    float angle = rand_2to1(randomSeed) * PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for (int i = 0; i < NUM_SAMPLES; i ++) {
        poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
        radius += radiusStep;
        angle += ANGLE_STEP;
    }
}
void uniformDiskSamples( const in vec2 randomSeed ) {

    float randNum = rand_2to1(randomSeed);
    float sampleX = rand_1to1( randNum ) ;
    float sampleY = rand_1to1( sampleX ) ;

    float angle = sampleX * PI2;
    float radius = sqrt(sampleY);

    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        poissonDisk[i] = vec2( radius * cos(angle) , radius * sin(angle)  );

        sampleX = rand_1to1( sampleY ) ;
        sampleY = rand_1to1( sampleX ) ;

        angle = sampleX * PI2;
        radius = sqrt(sampleY);
    }
}

float findBlocker(vec2 uv, float dReceiver){
    int blockNums=0;
    float dBlocker = 0.0;
    float filterStep = 5.0;
    float texelsize = textureSize(shadowMap,0).x;
    float filterRange = filterStep / texelsize;
    uniformDiskSamples(uv);
    for (int k=0;k<NUM_SAMPLES; k++)
    {
        if (dReceiver>0&&dReceiver<1.0){
            float closetDepth=texture(shadowMap, uv+poissonDisk[k]*filterRange).r;
            if (dReceiver-bias>closetDepth){
                dBlocker+=closetDepth;
                blockNums +=1;
            }
        }
    }
    if (blockNums == 0){
        return 0;
    } else {
        return dBlocker / blockNums;
    }

}
float pcss(vec4 lightSpace_pos){
    float shadow = 0.0;
    //透视除法
    vec3 projCoords=lightSpace_pos.xyz/lightSpace_pos.w;
    float dReceiver=projCoords.z;

    //1.Blocker Search Step
    float dBlocker=findBlocker(projCoords.xy, dReceiver);
    if (dBlocker==0){
        return 0;
    }

    //2.Penumbra Size Estimation
    float penumbraWidth=(dReceiver-dBlocker)*LIGHT_WIDTH/dBlocker;

    //3.filter
    float textureSize=textureSize(shadowMap, 0).x;
    float filterRange=penumbraWidth/textureSize;
    uniformDiskSamples(projCoords.xy);
    for (int k=0;k<NUM_SAMPLES;k++){
        if (dReceiver>0&&dReceiver<1.0){
            float closetDepth=texture(shadowMap, projCoords.xy+poissonDisk[k]*filterRange).r;
            if (dReceiver-bias>closetDepth){
                shadow+=1;
            }
        }
    }
    return shadow/float(PCF_NUM_SAMPLES);
}

float pcf(vec4 lightSpace_pos, int range){
    float shadow=0.0;
    //透视除法
    vec3 projCoords=lightSpace_pos.xyz/lightSpace_pos.w;
    float currentDepth=projCoords.z;
    vec2 texelSize=1.0/textureSize(shadowMap, 0);
    for (int x=-range;x<=range;x++){
        for (int y=-range;y<=range;y++){
            if (currentDepth > 0 && currentDepth < 1.0)
            {
                float closestDepth=texture(shadowMap, projCoords.xy+vec2(x, y)*texelSize).r;
                if (currentDepth-bias > closestDepth)
                shadow+=1;
            }

        }
    }
    float f=range+range+1;
    float nums=f*f;
    return shadow/nums;

}

vec3 Blinn_Phong(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 lightColor, float shadow){
    //ambient
    float ambientStrength=0.1;
    vec3 ambient = ambientStrength * lightColor;
    //diffuse
    float diffStrength=max(dot(normal, lightDir), 0.0);
    vec3 diffuse=diffStrength*lightColor;
    //specular
    float specularStrength = 0.5;
    vec3 reflectDir=reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    return ambient+(1.0-shadow)*(diffuse+specular);
}
vec3 NPR(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 lightColor, float shadow){
    float shadowRange=0.24;
    vec3 shadowColor=vec3(0.7, 0.7, 0.8);
    float shadowSmooth=0.4;
    //ambient
    float ambientStrength=0.1;
    vec3 ambient = ambientStrength * lightColor;
    //diffuse
    float halfLambert=dot(normal, lightDir)*0.5+0.5;
    float ramp=smoothstep(0, shadowSmooth, halfLambert-shadowRange);
    vec3 diffuse=(1-ramp)*shadowColor+ramp*lightColor;
    //specular
    float specularStrength = 0.5;
    vec3 reflectDir=reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    return ambient+(1.0-shadow)*(diffuse+specular);
}

void main(){
    vec4 baseColor =texture(texture0, fragTexCoord).rgba;

    vec3 lightDir =normalize(lightPos-FragPos);
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos-FragPos);
    //shadow
    //float shadow=pcf(lightSpace_pos, 3);
    float shadow=pcss(lightSpace_pos);
    //light
    vec3 lighting=Blinn_Phong(lightDir, normal, viewDir, lightColor, shadow);
    //vec3 lighting=NPR(lightDir,normal,viewDir,lightColor,shadow);
    outcolor=vec4(lighting, 1.0f)*baseColor;

}