// SAO.

#ifdef __INTELLISENSE__
    #define PS
#endif

#include "includes/Common.hlsli"

// Shader inputs and outputs.

struct VertexData {
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VertexOutput {
    float4 positionSS : POSITION;
    float2 uv : TEXCOORD0;
};

struct PixelInput {
    float4 positionPS : VPOS;
    float2 uv : TEXCOORD0;
};

struct PixelOutput {
    float4 occlusion : COLOR0;
};

#ifdef VS

float4 GeometryOffset : register(c0);
float4 TextureOffset : register(c1);

VertexOutput Main(VertexData vertex) {
    VertexOutput output;
    
    output.positionSS.xy = vertex.position.xy - (float2(2, -2) * GeometryOffset.xy);
    output.positionSS.zw = vertex.position.zw;
    output.uv = vertex.uv + TextureOffset.xy;

    return output;
}

#else

sampler2D DepthBuffer : register(s0);
sampler2D NormalsBuffer : register(s1);

float4 ScreenData : register(c0);
float4 DepthParams : register(c1);
float4 SAOSampleParams : register(c2);
float4 SAOParams : register(c3);
float4 Projection : register(c4);

static const float2 InvResolution = ScreenData.xy;
static const float2 Resolution = ScreenData.zw;
static const float FarPlane = DepthParams.y;
static const float Samples = SAOSampleParams.x;
static const float Turns = SAOSampleParams.y;
static const float RadiusScale = SAOParams.x;
static const float Radius = SAOParams.y;
static const float Radius2 = Radius * Radius;
static const float Bias = SAOParams.z;
static const float IntensityDivRadius6 = SAOParams.w;

static const float MinRadiusInPixels = 1.3f;
static const float Epsilon = 0.01f;

float InterleavedGradientNoise(float2 seed) {
    return fmod(52.9829189f * fmod(0.06711056f * seed.x + 0.00583715f * seed.y, 1.0f), 1.0f);
}

// View space position from pixel space position.
float3 ViewSpacePosition(int2 positionPS, out float depth) {
    float2 uv = (float2(positionPS) + 0.5f) * InvResolution;

    depth = tex2Dlod(DepthBuffer, float4(uv.xy, 0.0f, 0.0f)).r;
    float linearDepth = depth * FarPlane;
    
    return float3((uv.xy * Projection.xy + Projection.zw) * linearDepth, linearDepth);
}

// View space position for sample from pixel space position.
float3 ViewSpacePositionOffset(int2 positionPS, float2 unitOffset, float offsetDistance) {
    int mip = clamp((int) floor(log2(offsetDistance)) - 3, 0, 5);
    
    int2 samplePosPS = positionPS + int2(unitOffset * offsetDistance);
    
    float2 uv = (float2(samplePosPS) + 0.5f) * InvResolution;

    float linearDepth = tex2Dlod(DepthBuffer, float4(uv.xy, 0.0f, mip)).r * FarPlane;
    
    return float3((uv.xy * Projection.xy + Projection.zw) * linearDepth, linearDepth);
}

PixelOutput Main(PixelInput input) {
    PixelOutput output;
    
    int2 originPS = input.positionPS.xy;
    
    float3 originVS = ViewSpacePosition(originPS, output.occlusion.g);
 
    // Normal in view space at the origin.
    float3 normalVS = normalize(cross(ddx(originVS), ddy(originVS)));
 
    // Random offset angle for this pixel.
    float offsetAngle = InterleavedGradientNoise(originPS) * TAU;
    
    float radiusScale = originVS.z * RadiusScale;
    
    float sampleRadius = Radius / radiusScale;
    
    [branch]
    if (sampleRadius < MinRadiusInPixels) {
        output.occlusion.r = 1.0f;
        output.occlusion.b = 0.0f;
        output.occlusion.a = 1.0f;
        return output;
    }
    
    float minSampleDistance = MinRadiusInPixels / sampleRadius;
    
    float result = 0.0f;
    for (int i = 0; i < Samples; i++) {
        // Place a sample on a spiral from the central point.
        float sampleDistance = float(i + 0.5f) * (1.0f / Samples);
        float angle = TAU * sampleDistance * Turns + offsetAngle;
        float2 sampleDirection = float2(cos(angle), sin(angle));
        
        sampleDistance += minSampleDistance;
        
        sampleDistance *= sampleRadius;
        
        float3 sampleVS = ViewSpacePositionOffset(originPS, sampleDirection, sampleDistance);

        float3 sampleRay = sampleVS - originVS;
        float rayDotRay = dot(sampleRay, sampleRay);
        float rayDotNormal = dot(sampleRay, normalVS);
 
        float f = max(Radius2 - rayDotRay, 0.0f);
        result += f * f * f * max((rayDotNormal - Bias) / (rayDotRay + Epsilon), 0.0f);
    }
    
    float occlusion = max(0.0f, 1.0f - result * IntensityDivRadius6 * (5.0f / Samples));
    
    output.occlusion.r = occlusion;
    output.occlusion.b = 0.0f;
    output.occlusion.a = 1.0f;
    
    if (abs(ddx(output.occlusion.g)) < 0.02f)
        output.occlusion.r -= ddx(output.occlusion.r) * ((originPS.x % 2) - 0.5f);
    if (abs(ddy(output.occlusion.g)) < 0.02)
        output.occlusion.r -= ddy(output.occlusion.r) * ((originPS.y % 2) - 0.5f);
    
    return output;
}

#endif