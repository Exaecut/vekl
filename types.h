#pragma once

#ifdef USE_HALF_PRECISION
    #if defined(__METAL_VERSION__)
        typedef half4 pixel_format;
    #else
        #include <cuda_fp16.h>
        struct __align__(8) half4
        {
            __half x, y, z, w;

            __device__ __forceinline__ half4() = default;

            __device__ __forceinline__ half4(__half x_, __half y_, __half z_, __half w_)
                : x(x_), y(y_), z(z_), w(w_) {}

            // Covers pixel_format(r, g, b, a) in kernel code.
            __device__ __forceinline__ half4(float x_, float y_, float z_, float w_)
                : x(__float2half(x_)), y(__float2half(y_)),
                  z(__float2half(z_)), w(__float2half(w_)) {}

            // Covers half4(c) / from_float4<half4>(c) / (half4)c.
            // Explicit: avoids silent narrowing in assignments.
            __device__ __forceinline__ explicit half4(float4 v)
                : x(__float2half(v.x)), y(__float2half(v.y)),
                  z(__float2half(v.z)), w(__float2half(v.w)) {}

            // Covers (float4)h in image_read / to_float4(h).
            // Member assignment is used instead of make_float4() to avoid
            // ambiguity between CUDA's ::float4 (from cuda_fp16.h) and
            // VEKL's dsl_float4 typedef. The typedef takes precedence for
            // unqualified 'float4', but make_float4() would resolve to the
            // CUDA function returning ::float4, causing a type mismatch.
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

        // make_float4 overload for half4. Uses member assignment for the
        // same reason as operator float4() above.
        __device__ __forceinline__ float4 make_float4(half4 h)
        {
            float4 r;
            r.x = __half2float(h.x);
            r.y = __half2float(h.y);
            r.z = __half2float(h.z);
            r.w = __half2float(h.w);
            return r;
        }

        typedef half4 pixel_format;
    #endif
#else
    typedef float4 pixel_format;
    typedef float4 half4;
#endif

struct FrameParams {
    uint out_pitch;
    uint in_pitch;
    uint dest_pitch;
    uint width;
    uint height;
    float progress;
};
