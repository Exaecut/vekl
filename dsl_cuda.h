#pragma once

#include <cuda_runtime.h>

#define kernel          extern "C" __global__
#define device          __device__ __forceinline__
#define constant        const
#define threadgroup_mem __shared__
#define thread_local    /* matches local variables in generic C++ */
#define restrict_ptr    __restrict__
#define restrict        __restrict__
#define min fminf
#define max fmaxf

typedef unsigned int   uint;
typedef unsigned char  uchar;
typedef unsigned short ushort;

// CUDA ignores the slot index (s) used by Metal
#define param_dev_ro(T, name, s)   const T * __restrict__ name
#define param_dev_rw(T, name, s)   T * __restrict__ name
#define param_dev_wo(T, name, s)   T * __restrict__ name

// For Constant Buffers, we pass by value (copy to kernel param memory/constant bank)
// Reference (const T &) is not strictly valid for __global__ entry points in CUDA.
#define param_dev_cbuf(T, name, s) const T name

// No argument needed in the parameter list for CUDA (uses built-ins)
#define thread_pos_param(name)

// Calculate GID inside the kernel; utilizes our dsl_uint2 constructor over ::make_uint2 result
#define thread_pos_init(name) \
    uint2 name = ::make_uint2(blockIdx.x * blockDim.x + threadIdx.x, \
                            blockIdx.y * blockDim.y + threadIdx.y)

#define threadgroup_barrier_all() __syncthreads()

// Ensure abs(float) works (CUDA standard is fabsf for floats)
__device__ inline float abs(float a) { return ::fabsf(a); }