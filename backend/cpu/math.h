#pragma once

inline float abs(float a) { return fabsf(a); }
inline float min(float a, float b) { return fminf(a, b); }
inline float max(float a, float b) { return fmaxf(a, b); }

inline float clamp(float v, float lo, float hi) { return fminf(fmaxf(v, lo), hi); }
inline int   clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

inline float saturate(float v) { return clamp(v, 0.0f, 1.0f); }
inline float pow(float x, float y) { return powf(x, y); }
inline float exp2(float x) { return expf(x * 0.6931471805599453f); }

inline float mix(float a, float b, float t) { return a + (b - a) * t; }
inline float fract(float v) { return v - floorf(v); }
inline float smoothstep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline float2 floor(float2 v) { return float2(floorf(v.x), floorf(v.y)); }
inline float2 fract(float2 v) { return float2(fract(v.x), fract(v.y)); }
inline float2 abs(float2 v) { return float2(fabsf(v.x), fabsf(v.y)); }
inline float2 clamp(float2 v, float lo, float hi) { return float2(clamp(v.x, lo, hi), clamp(v.y, lo, hi)); }
inline float2 min(float2 a, float2 b) { return float2(fminf(a.x, b.x), fminf(a.y, b.y)); }
inline float2 max(float2 a, float2 b) { return float2(fmaxf(a.x, b.x), fmaxf(a.y, b.y)); }
inline float2 mix(float2 a, float2 b, float t) { return a + (b - a) * t; }

inline float dot(float2 a, float2 b) { return a.x * b.x + a.y * b.y; }
inline float dot(float3 a, float3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float length(float2 v) { return sqrtf(v.x * v.x + v.y * v.y); }
inline float2 normalize(float2 v) { float l = length(v); return l != 0.0f ? v / l : float2(0.0f); }

inline float3 floor(float3 v) { return float3(floorf(v.x), floorf(v.y), floorf(v.z)); }
inline float3 fract(float3 v) { return float3(fract(v.x), fract(v.y), fract(v.z)); }
inline float3 abs(float3 v) { return float3(fabsf(v.x), fabsf(v.y), fabsf(v.z)); }
inline float3 clamp(float3 v, float lo, float hi) { return float3(clamp(v.x, lo, hi), clamp(v.y, lo, hi), clamp(v.z, lo, hi)); }
inline float3 mix(float3 a, float3 b, float t) { return a + (b - a) * t; }

inline float4 clamp(float4 v, float lo, float hi) {
    return float4(clamp(v.x, lo, hi), clamp(v.y, lo, hi), clamp(v.z, lo, hi), clamp(v.w, lo, hi));
}
inline float4 mix(float4 a, float4 b, float t) { return a + (b - a) * t; }
inline float4 abs(float4 v) { return float4(fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w)); }

inline float3 saturate(float3 v) { return clamp(v, 0.0f, 1.0f); }
inline float4 saturate(float4 v) { return clamp(v, 0.0f, 1.0f); }
