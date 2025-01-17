#version 450
layout(location=0) in vec2 fragTexCoord;

layout(location=0) out vec4 outcolor;

layout(set=0, binding=0) uniform sampler2D texture0;

#define FXAA_ABSOLUTE_LUMA_THRESHOLD 0.0625
#define FXAA_RELATIVE_LUMA_THRESHOLD 0.166
#define FXAA_SEARCHSTEP 10
#define FXAA_SERACHEND 8

float luma(vec2 uv){
    vec3 color=texture(texture0, uv).rgb;
    return 0.213*color.r+0.715*color.g;
}

float saturate(float value){
    return min(max(0, value), 1);
}
vec4 fxaa(){
    float minThreshold=1;

    vec2 texelSize=1.0/textureSize(texture0, 0);
    float W=luma(fragTexCoord+vec2(-1, 0)*texelSize);
    float M=luma(fragTexCoord);
    float E=luma(fragTexCoord+vec2(1, 0)*texelSize);
    float N=luma(fragTexCoord+vec2(0, 1)*texelSize);
    float S=luma(fragTexCoord+vec2(0, -1)*texelSize);
    float NW=luma(fragTexCoord+vec2(-1, 1)*texelSize);
    float NE=luma(fragTexCoord+vec2(1, 1)*texelSize);
    float SW=luma(fragTexCoord+vec2(-1, -1)*texelSize);
    float SE=luma(fragTexCoord+vec2(1, -1)*texelSize);

    float MaxLuma=max(max(N, E), max(max(W, S), M));
    float MinLuma=min(min(N, E), min(min(W, S), M));
    float Contrast=MaxLuma-MinLuma;
    float edgeThreshold = max(FXAA_ABSOLUTE_LUMA_THRESHOLD, MaxLuma * FXAA_RELATIVE_LUMA_THRESHOLD);
    if (Contrast>=edgeThreshold){
        float Filter=2*(N+E+S+W)+NE+NW+SE+SW;
        Filter=Filter/12;
        Filter=abs(Filter-M);
        Filter=saturate(Filter/Contrast);
        float PixelBlend=smoothstep(0, 1, Filter);
        PixelBlend=PixelBlend*PixelBlend;

        //blend direction
        float Vertical=abs(N + S - 2 * M) * 2+ abs(NE + SE - 2 * E) + abs(NW + SW - 2 * W);
        float Horizontal = abs(E + W - 2 * M) * 2 + abs(NE + NW - 2 * N) + abs(SE + SW - 2 * S);
        bool IsVertical = Vertical > Horizontal;
        vec2 PixelStep = IsVertical ? vec2(0, texelSize.y) : vec2(texelSize.x, 0);
        float Positive=abs((IsVertical?N:E)-M);
        float Negative=abs((IsVertical?S:W)-M);
        float Gradient, OppositeLuma;
        if (Positive>Negative){
            Gradient=Positive;
            OppositeLuma=IsVertical?N:E;
        } else {
            Gradient=Negative;
            PixelStep=-PixelStep;
            OppositeLuma=IsVertical?S:W;
        }
        //根据边界的方向，确定搜索方向，并进行搜索
        vec2 UVEdge=fragTexCoord+PixelStep*0.5;
        float EdgeStep_1=IsVertical?texelSize.x:texelSize.y;
        vec2 EdgeStep_2=IsVertical?vec2(texelSize.x, 0):vec2(0, texelSize.y);
        float EdgeLuma=(M+OppositeLuma)*0.5f;
        float GradientThreshold=EdgeLuma*0.25;
        //沿锯齿方向搜索
        float PLumaDelta, NLumaDelta, PDistance, NDistance;
        int i;
        for (i=1;i<=FXAA_SEARCHSTEP;i++){
            PLumaDelta=luma(UVEdge+i*EdgeStep_2)-EdgeLuma;
            if (abs(PLumaDelta)>GradientThreshold){
                PDistance=i*EdgeStep_1;
                break;
            }
        }
        if (i==FXAA_SEARCHSTEP+1){
            PDistance=EdgeStep_1*FXAA_SERACHEND;
        }
        //沿另一侧锯齿方向搜索
        for (i=1;i<=FXAA_SEARCHSTEP;i++){
            NLumaDelta=luma(UVEdge-i*EdgeStep_2)-EdgeLuma;
            if (abs(NLumaDelta)>GradientThreshold){
                NDistance=i*EdgeStep_1;
                break;
            }
        }
        if (i==FXAA_SEARCHSTEP+1){
            NDistance=EdgeStep_1*FXAA_SERACHEND;
        }
        float EdgeBlend;
        if(PDistance<NDistance){
            if(sign(PLumaDelta)==sign(M-EdgeLuma)){
                EdgeBlend=0;
            }else{
                EdgeBlend=0.5f-PDistance/(PDistance+NDistance);
            }
        }else{
            if(sign(NLumaDelta)==sign(M-EdgeLuma)){
                EdgeBlend=0;
            }else{
                EdgeBlend=0.5f-NDistance/(PDistance+NDistance);
            }
        }
        float FinalBlend=max(PixelBlend,EdgeBlend);

        return texture(texture0, fragTexCoord+PixelStep*FinalBlend);
    }
    return texture(texture0, fragTexCoord);
}

void main(){
    vec4 baseColor=fxaa();

    outcolor=baseColor;
}