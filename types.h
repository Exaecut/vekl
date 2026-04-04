#pragma once

#ifdef USE_HALF_PRECISION
    #if defined(__METAL_VERSION__)
        typedef half4 pixel_format;
    #else
        #include <cuda_fp16.h>
        struct __align__(8) half4
        {
            __half x, y, z, w;
        };
        typedef half4 pixel_format;
    #endif
#else
    typedef float4 pixel_format;
    typedef float4 half4;
#endif

struct TransitionParams {
    uint out_pitch;
    uint in_pitch;
    uint dest_pitch;
    uint width;
    uint height;
    float progress;
};
