// Linearize depth.

struct PixelInput {
    float2 uv : TEXCOORD0;
};

struct PixelOutput {
    float4 linearDepth : COLOR0;
};

sampler2D HardwareDepthBuffer : register(s0);

float4 DepthConstants : register(c0);

PixelOutput Main(PixelInput input) {
    PixelOutput output;

    float depth = tex2D(HardwareDepthBuffer, input.uv).r;
    
    output.linearDepth.rgb = (DepthConstants.w / (depth * DepthConstants.z + DepthConstants.x)) / DepthConstants.y;
    output.linearDepth.a = 1.0f;
    
    return output;
}
