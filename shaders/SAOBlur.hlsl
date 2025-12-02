// Separable bilateral blur for SAO.

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
    float2 uv : TEXCOORD0;
};

struct PixelOutput {
    float4 occlusion : COLOR0;
};

#ifdef VS

float4 GeometryOffset : register(c0);
float4 TextureOffset : register(c1);

row_major float4x4 InvProjMatrix : register(c3);

VertexOutput Main(VertexData vertex) {
    VertexOutput output;
    
    output.positionSS.xy = vertex.position.xy - (float2(2, -2) * GeometryOffset.xy);
    output.positionSS.zw = vertex.position.zw;
    output.uv = vertex.uv + TextureOffset.xy;

    return output;
}

#else

sampler2D AOBuffer : register(s0);

float4 ScreenData : register(c0);
float4 Params : register(c1);

static const float2 TexelSize = ScreenData.xy;
static const float2 OffsetMask = ScreenData.zw;
static const float EdgeSharpness = Params.x;

static const float GaussianWeights[5] = {
    0.153170,
    0.144893,
    0.122649,
    0.092902,
    0.062970
};

PixelOutput Main(PixelInput input) {
    PixelOutput output;
    
    float4 sample = tex2D(AOBuffer, input.uv);
    float sum = sample.r;
    float depth = sample.g;
    
    output.occlusion.g = sample.g;
    output.occlusion.b = 0.0f;
    output.occlusion.a = 1.0f;
    
    if (depth > 0.999) {
        output.occlusion.r = sum;
        return output;
    }
    
    float totalWeight = GaussianWeights[0];
    sum *= totalWeight;
    
    [unroll]
    for (int i = -4; i <= 4; i++) {
        if (i == 0)
            continue;
            
        float2 offset = ((i * 2.0f) * OffsetMask) * TexelSize;
        
        sample = tex2D(AOBuffer, input.uv + offset);
        float sampleValue = sample.r;
        float sampleDepth = sample.g;
        
        float weight = 0.3f + GaussianWeights[abs(i)];
        
        weight *= max(0.0f, 1.0f - (2000.0f * EdgeSharpness) * abs(sampleDepth - depth));
        
        sum += sampleValue * weight;
        totalWeight += weight;
    }
    
    const float epsilon = 0.0001f;
    output.occlusion.r = sum / (totalWeight + epsilon);
    
    return output;
}

#endif