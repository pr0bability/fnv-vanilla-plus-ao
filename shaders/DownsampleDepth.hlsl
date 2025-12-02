// Downsample depth creating the next level of a mipchain.

struct PixelInput {
    float4 position : VPOS;
    float2 uv : TEXCOORD0;
};

struct PixelOutput {
    float4 depth : COLOR0;
};

sampler2D LinearDepthBuffer : register(s0);

float4 ScreenData : register(c0);

static const float2 PreviousTexelSize = ScreenData.xy;
static const float PreviousMip = ScreenData.z;

PixelOutput Main(PixelInput input) {
    PixelOutput output;
    
    // Checkerboard pattern.
    int2 pixelOffset = int2(1 - input.position.y % 2, 1 - input.position.x % 2);

    output.depth.rgb = tex2Dlod(LinearDepthBuffer, float4((input.position.xy * 2.0f + pixelOffset) * PreviousTexelSize, 0.0f, PreviousMip)).r;
    output.depth.a = 1.0f;
    
    return output;
}
