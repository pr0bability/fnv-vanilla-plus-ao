// Bilateral upsample and apply for SSAO.


#ifdef __INTELLISENSE__
    #define PS
#endif

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
    float4 color : COLOR0;
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

sampler2D RenderedBuffer : register(s0);
sampler2D AOBuffer : register(s1);

float4 DepthParams : register(c0);
float4 SAOParams : register(c1);
float4 FogParams : register(c2);

static const float FarPlane = DepthParams.y;
static const float Strength = SAOParams.x;
static const float LuminanceThreshold = SAOParams.y;
static const float MinAmbient = SAOParams.z;
static const float FogNear = FogParams.x;
static const float FogFar = FogParams.y;
static const float FogPower = FogParams.z;

static const float3 LuminanceFactors = float3(0.2126f, 0.7152f, 0.0722f);

PixelOutput Main(PixelInput input) {
    PixelOutput output;
    
    float4 renderedColor = tex2D(RenderedBuffer, input.uv);
   
    float2 AO = tex2D(AOBuffer, input.uv).rg;
    
    // Convert to projection space for fog calculation.
    float2 ndc = (input.uv * 2.0f - 1.0f) * float2(1.0f, -1.0f);
    float linearDepth = AO.g * FarPlane;
    float ndcDepth = (DepthParams.w / linearDepth - DepthParams.x) / DepthParams.z;
    
    float4 positionPS = float4(ndc.xy, ndcDepth, 1.0f);
    positionPS *= linearDepth;
    
    positionPS.z = positionPS.w - positionPS.z;
    
    // Fog influence.
    float fogStrength = 1.0f - saturate((FogFar - length(positionPS.xyz)) / FogNear);
    float fogAlpha = pow(fogStrength, FogPower);
    
    // Luminance influence.
    float luminance = dot(renderedColor.rgb, LuminanceFactors);
    float luminanceInfluence = 1.0f - saturate(luminance * LuminanceThreshold);
    
    // Scale AO by strength and luminance influence.
    float finalAO = saturate(lerp(1.0f, AO.r, luminanceInfluence * Strength));
    
    // Rescale by min ambient.
    finalAO = (finalAO + MinAmbient) / (1.0f + MinAmbient);

    // Apply fog.
    finalAO = lerp(finalAO, 1.0f, fogAlpha);
    
    // Delinearize the AO value, since we are working with "sRGB" rendered color.
    finalAO = pow(abs(finalAO), 0.45f);
    
    output.color.rgb = renderedColor.rgb * finalAO;
    output.color.a = renderedColor.a;
    
    return output;
}

#endif