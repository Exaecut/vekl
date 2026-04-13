#pragma once

#ifdef USE_HALF_PRECISION
#define format 8u
#else
#define format 16u
#endif

inline uint2 dispatch_id() {
    return dsl_make_uint2(blockIdx.x * blockDim.x + threadIdx.x,
                          blockIdx.y * blockDim.y + threadIdx.y);
}
inline uint2 dispatch_size() {
    return dsl_make_uint2(gridDim.x * blockDim.x, gridDim.y * blockDim.y);
}
