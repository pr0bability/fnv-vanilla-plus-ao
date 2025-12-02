#ifndef COMMON_HLSLI
#define COMMON_HLSLI

// Common definitions.
static const float PI = 3.14159265359f;
static const float TAU = 2.0f * PI;
static const float RCP_TAU = 1.0f / TAU;

// Common functions.

// Expand a compressed normal vector (i.e. as sampled from a normal map) from [0, 1] to [-1, 1].
float3 ExpandVector(float3 vec) {
    return 2.0f * vec - 1.0f;
}

// Compress a normal vector from [-1, 1] to [0, 1].
float3 CompressVector(float3 vec) {
    return (vec * 0.5f) + 0.5f;
}

#endif
