// dsl_cuda.h
#pragma once

#include <cuda_runtime.h>
#include <cmath>
#include <cstdint>
#include <algorithm> // for max/min if needed

// -----------------------------------------------------------------------------
// Keywords and Address Spaces
// -----------------------------------------------------------------------------

#define kernel          extern "C" __global__
#define device          __device__ __forceinline__
#define constant        const
#define threadgroup_mem __shared__
#define thread_local    /* matches local variables in generic C++ */
#define restrict_ptr    __restrict__
#define restrict        __restrict__

// -----------------------------------------------------------------------------
// scalar types
// -----------------------------------------------------------------------------

typedef unsigned int   uint;
typedef unsigned char  uchar;
typedef unsigned short ushort;

// -----------------------------------------------------------------------------
// Vector Types Implementation
// -----------------------------------------------------------------------------
// We define wrapper structs that match CUDA built-in layout but add:
// 1. Constructors (splat, component-wise, from-builtin)
// 2. Component aliases (.r, .g, .b, .a) via unions
// 3. Implicit conversion to/from CUDA built-ins for compatibility

#define DSL_DEFINE_VEC2(TYPE, BUILTIN, USER_NAME) \
    struct USER_NAME { \
        union { \
            struct { TYPE x, y; }; \
            struct { TYPE r, g; }; \
        }; \
        __host__ __device__ USER_NAME() {} \
        __host__ __device__ USER_NAME(TYPE v) : x(v), y(v) {} \
        __host__ __device__ USER_NAME(TYPE _x, TYPE _y) : x(_x), y(_y) {} \
        __host__ __device__ USER_NAME(const BUILTIN& v) : x(v.x), y(v.y) {} \
        __host__ __device__ operator BUILTIN() const { \
            BUILTIN v; v.x = x; v.y = y; return v; \
        } \
    };

#define DSL_DEFINE_VEC3(TYPE, BUILTIN, USER_NAME) \
    struct USER_NAME { \
        union { \
            struct { TYPE x, y, z; }; \
            struct { TYPE r, g, b; }; \
        }; \
        __host__ __device__ USER_NAME() {} \
        __host__ __device__ USER_NAME(TYPE v) : x(v), y(v), z(v) {} \
        __host__ __device__ USER_NAME(TYPE _x, TYPE _y, TYPE _z) : x(_x), y(_y), z(_z) {} \
        __host__ __device__ USER_NAME(const BUILTIN& v) : x(v.x), y(v.y), z(v.z) {} \
        __host__ __device__ operator BUILTIN() const { \
            BUILTIN v; v.x = x; v.y = y; v.z = z; return v; \
        } \
    };

#define DSL_DEFINE_VEC4(TYPE, BUILTIN, USER_NAME) \
    struct USER_NAME { \
        union { \
            struct { TYPE x, y, z, w; }; \
            struct { TYPE r, g, b, a; }; \
        }; \
        __host__ __device__ USER_NAME() {} \
        __host__ __device__ USER_NAME(TYPE v) : x(v), y(v), z(v), w(v) {} \
        __host__ __device__ USER_NAME(TYPE _x, TYPE _y, TYPE _z, TYPE _w) : x(_x), y(_y), z(_z), w(_w) {} \
        __host__ __device__ USER_NAME(const BUILTIN& v) : x(v.x), y(v.y), z(v.z), w(v.w) {} \
        __host__ __device__ operator BUILTIN() const { \
            BUILTIN v; v.x = x; v.y = y; v.z = z; v.w = w; return v; \
        } \
    };

// Define DSL types to separate names first
DSL_DEFINE_VEC2(float, ::float2, dsl_float2)
DSL_DEFINE_VEC3(float, ::float3, dsl_float3)
DSL_DEFINE_VEC4(float, ::float4, dsl_float4)

DSL_DEFINE_VEC2(int, ::int2, dsl_int2)
DSL_DEFINE_VEC3(int, ::int3, dsl_int3)
DSL_DEFINE_VEC4(int, ::int4, dsl_int4)

DSL_DEFINE_VEC2(uint, ::uint2, dsl_uint2)
DSL_DEFINE_VEC3(uint, ::uint3, dsl_uint3)
DSL_DEFINE_VEC4(uint, ::uint4, dsl_uint4)

// Macro-replace standard names to use our DSL types
#define float2 dsl_float2
#define float3 dsl_float3
#define float4 dsl_float4
#define int2   dsl_int2
#define int3   dsl_int3
#define int4   dsl_int4
#define uint2  dsl_uint2
#define uint3  dsl_uint3
#define uint4  dsl_uint4

// -----------------------------------------------------------------------------
// Kernel Parameter Macros
// -----------------------------------------------------------------------------

// CUDA ignores the slot index (s) used by Metal
#define param_dev_ro(T, name, s)   const T * __restrict__ name
#define param_dev_rw(T, name, s)   T * __restrict__ name
#define param_dev_wo(T, name, s)   T * __restrict__ name

// For Constant Buffers, we pass by value (copy to kernel param memory/constant bank)
// Reference (const T &) is not strictly valid for __global__ entry points in CUDA.
#define param_dev_cbuf(T, name, s) const T name

// -----------------------------------------------------------------------------
// Thread Position & Synchronization Macros
// -----------------------------------------------------------------------------

// No argument needed in the parameter list for CUDA (uses built-ins)
#define thread_pos_param(name)

// Calculate GID inside the kernel; utilizes our dsl_uint2 constructor over ::make_uint2 result
#define thread_pos_init(name) \
    uint2 name = make_uint2(blockIdx.x * blockDim.x + threadIdx.x, \
                            blockIdx.y * blockDim.y + threadIdx.y)

#define threadgroup_barrier_all() __syncthreads()

// -----------------------------------------------------------------------------
// Helpers / Math Overloads
// -----------------------------------------------------------------------------

// Ensure abs(float) works (CUDA standard is fabsf for floats)
__device__ inline float abs(float a) { return ::fabsf(a); }