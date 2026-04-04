#pragma once

#ifdef USE_HALF_PRECISION
    struct __device_builtin__ __builtin_align__(8) half4
    {
        half x, y, z, w;
    }

    typedef half4 pixel_format;
#else
    typedef float4 pixel_format;
    typedef float4 half4;
#endif