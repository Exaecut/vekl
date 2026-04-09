#pragma once

#ifdef VEKL_CPU
    // pixel is void on CPU — the actual storage format is determined at runtime
    // via __cpu_format. image_2d handles format-aware read/write transparently.
    // typedef already provided by vekl_cpu.h
#else
    #ifdef USE_HALF_PRECISION
        #if defined(__METAL_VERSION__)
            typedef half4 pixel;
        #else
            #include <cuda_fp16.h>
            struct __align__(8) half4
            {
                __half x, y, z, w;

                __device__ __forceinline__ half4() = default;

                __device__ __forceinline__ half4(__half x_, __half y_, __half z_, __half w_)
                    : x(x_), y(y_), z(z_), w(w_) {}

                __device__ __forceinline__ half4(float x_, float y_, float z_, float w_)
                    : x(__float2half(x_)), y(__float2half(y_)),
                  z(__float2half(z_)), w(__float2half(w_)) {}

                __device__ __forceinline__ explicit half4(float4 v)
                    : x(__float2half(v.x)), y(__float2half(v.y)),
                  z(__float2half(v.z)), w(__float2half(v.w)) {}

                __device__ __forceinline__ operator float4() const
                {
                    float4 r;
                    r.x = __half2float(x);
                    r.y = __half2float(y);
                    r.z = __half2float(z);
                    r.w = __half2float(w);
                    return r;
                }
            };

            __device__ __forceinline__ float4 make_float4(half4 h)
            {
                float4 r;
                r.x = __half2float(h.x);
                r.y = __half2float(h.y);
                r.z = __half2float(h.z);
                r.w = __half2float(h.w);
                return r;
            }

            typedef half4 pixel;
        #endif
    #else
        typedef float4 pixel;
        typedef float4 half4;
    #endif
#endif

struct FrameParams {
    uint out_pitch;
    uint in_pitch;
    uint dest_pitch;
    uint width;
    uint height;
    float progress;
    uint bpp;
};
