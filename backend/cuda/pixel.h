#pragma once

#ifdef USE_HALF_PRECISION
#include <cuda_fp16.h>
struct __align__(8) half4 {
    __half x, y, z, w;
    __device__ __forceinline__ half4() = default;
    __device__ __forceinline__ half4(__half x_, __half y_, __half z_, __half w_) : x(x_), y(y_), z(z_), w(w_) {}
    __device__ __forceinline__ half4(float x_, float y_, float z_, float w_) : x(__float2half(x_)), y(__float2half(y_)), z(__float2half(z_)), w(__float2half(w_)) {}
    __device__ __forceinline__ explicit half4(float4 v) : x(__float2half(v.x)), y(__float2half(v.y)), z(__float2half(v.z)), w(__float2half(v.w)) {}
    __device__ __forceinline__ operator float4() const {
        float4 r;
        r.x = __half2float(x); r.y = __half2float(y); r.z = __half2float(z); r.w = __half2float(w);
        return r;
    }
};
typedef half4 pixel;
#else
typedef float4 pixel;
typedef float4 half4;
#endif

struct layout_rgba {
    inline float4 to_rgba(float4 c) const { return c; }
    inline float4 from_rgba(float4 c) const { return c; }
};

struct layout_bgra {
    inline float4 to_rgba(float4 c) const { return float4(c.b, c.g, c.r, c.a); }
    inline float4 from_rgba(float4 c) const { return float4(c.z, c.y, c.x, c.w); }
};

struct layout_vuya {

    inline float4 to_rgba(float4 c) const {
        float v = c.x - 0.5f;
        float u = c.y - 0.5f;
        float y = c.z;
        return float4(
            y + 1.402f * v,
            y - 0.344136f * u - 0.714136f * v,
            y + 1.772f * u,
            c.w
        );
    }

    inline float4 from_rgba(float4 c) const {
        float y = 0.299f * c.x + 0.587f * c.y + 0.114f * c.z;
        return float4(
            (c.x - y) / 1.402f + 0.5f,
            (c.z - y) / 1.772f + 0.5f,
            y,
            c.w
        );
    }
};

struct layout_vuya709 {

    inline float4 to_rgba(float4 c) const {
        float v = c.x - 0.5f;
        float u = c.y - 0.5f;
        float y = c.z;
        return float4(
            y + 1.5748f * v,
            y - 0.1873f * u - 0.4681f * v,
            y + 1.8556f * u,
            c.w
        );
    }

    inline float4 from_rgba(float4 c) const {
        float y = 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z;
        return float4(
            (c.x - y) / 1.5748f + 0.5f,
            (c.z - y) / 1.8556f + 0.5f,
            y,
            c.w
        );
    }
};

struct layout_auto {
    uint layout_type;

    inline float4 to_rgba(float4 c) const {
        switch (layout_type) {
            case 2: return layout_vuya().to_rgba(c);
            case 3: return layout_vuya709().to_rgba(c);
            case 1: return layout_bgra().to_rgba(c);
            default: return c;
        }
    }

    inline float4 from_rgba(float4 c) const {
        switch (layout_type) {
            case 2: return layout_vuya().from_rgba(c);
            case 3: return layout_vuya709().from_rgba(c);
            case 1: return layout_bgra().from_rgba(c);
            default: return c;
        }
    }
};

inline float4 pixel_load(device const pixel *data, uint pitch_px, uint2 xy) {
    return (float4)data[xy.y * pitch_px + xy.x];
}

inline void pixel_store(device pixel *data, uint pitch_px, uint2 xy, float4 c) {
    data[xy.y * pitch_px + xy.x] = (pixel)c;
}
