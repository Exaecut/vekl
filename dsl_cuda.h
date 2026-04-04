#pragma once

// DSL CUDA Backend — NVRTC compatible
// Compiled with --device-as-default-execution-space (all code is implicitly __device__)
//
// NVRTC pre-defines float2/float3/float4/uint2 as plain C structs (no constructors, no operators).
// We define our own dsl_* types with full Metal-like semantics, then shadow the built-in names
// via macros at the end of this header.

#define kernel          extern "C" __global__
#define constant        const
#define threadgroup_mem __shared__
#define thread_local
#define restrict_ptr    __restrict__
#define restrict        __restrict__

// Metal 'device' is an address-space qualifier for pointers.
// CUDA device pointers have no special qualifier.
#define device

typedef unsigned int   uint;
typedef unsigned char  uchar;
typedef unsigned short ushort;

#ifndef M_PI_F
#define M_PI_F 3.14159265358979323846f
#endif

#define param_dev_ro(T, name, s)   const T * __restrict__ name
#define param_dev_rw(T, name, s)   T * __restrict__ name
#define param_dev_wo(T, name, s)   T * __restrict__ name
#define param_dev_cbuf(T, name, s) const T name

#define thread_pos_param(name)
#define thread_pos_init(name) uint2 name = dispatch_id()

#define threadgroup_barrier_all() __syncthreads()

struct dsl_float2;
struct dsl_float3;
struct dsl_float4;
struct dsl_uint2;

struct dsl_float2 {
    float x, y;
    inline dsl_float2() = default;
    inline dsl_float2(float s) : x(s), y(s) {}
    inline dsl_float2(float x_, float y_) : x(x_), y(y_) {}
    inline explicit dsl_float2(dsl_uint2 v);
};

struct dsl_float3 {
    float x, y, z;
    inline dsl_float3() = default;
    inline dsl_float3(float s) : x(s), y(s), z(s) {}
    inline dsl_float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct dsl_float4 {
    union { float x; float r; };
    union { float y; float g; };
    union { float z; float b; };
    union { float w; float a; };
    inline dsl_float4() = default;
    inline dsl_float4(float s) : x(s), y(s), z(s), w(s) {}
    inline dsl_float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    inline dsl_float4(dsl_float3 v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
    inline dsl_float3 xyz() const { return dsl_float3(x, y, z); }
};

struct dsl_uint2 {
    unsigned int x, y;
    inline dsl_uint2() = default;
    inline dsl_uint2(unsigned int s) : x(s), y(s) {}
    inline dsl_uint2(unsigned int x_, unsigned int y_) : x(x_), y(y_) {}
    inline dsl_uint2(int x_, int y_) : x((unsigned int)x_), y((unsigned int)y_) {}
    inline dsl_uint2(float x_, float y_) : x((unsigned int)x_), y((unsigned int)y_) {}
    inline explicit dsl_uint2(dsl_float2 v);
};

inline dsl_float2::dsl_float2(dsl_uint2 v) : x((float)v.x), y((float)v.y) {}
inline dsl_uint2::dsl_uint2(dsl_float2 v) : x((unsigned int)v.x), y((unsigned int)v.y) {}

inline dsl_uint2 dsl_make_uint2(unsigned int x, unsigned int y) { return dsl_uint2(x, y); }

// dsl_float2 operators
inline dsl_float2 operator+(dsl_float2 a, dsl_float2 b) { return dsl_float2(a.x + b.x, a.y + b.y); }
inline dsl_float2 operator-(dsl_float2 a, dsl_float2 b) { return dsl_float2(a.x - b.x, a.y - b.y); }
inline dsl_float2 operator*(dsl_float2 a, dsl_float2 b) { return dsl_float2(a.x * b.x, a.y * b.y); }
inline dsl_float2 operator/(dsl_float2 a, dsl_float2 b) { return dsl_float2(a.x / b.x, a.y / b.y); }
inline dsl_float2 operator+(dsl_float2 a, float s) { return dsl_float2(a.x + s, a.y + s); }
inline dsl_float2 operator-(dsl_float2 a, float s) { return dsl_float2(a.x - s, a.y - s); }
inline dsl_float2 operator*(dsl_float2 a, float s) { return dsl_float2(a.x * s, a.y * s); }
inline dsl_float2 operator/(dsl_float2 a, float s) { return dsl_float2(a.x / s, a.y / s); }
inline dsl_float2 operator+(float s, dsl_float2 a) { return dsl_float2(s + a.x, s + a.y); }
inline dsl_float2 operator-(float s, dsl_float2 a) { return dsl_float2(s - a.x, s - a.y); }
inline dsl_float2 operator*(float s, dsl_float2 a) { return dsl_float2(s * a.x, s * a.y); }
inline dsl_float2 operator/(float s, dsl_float2 a) { return dsl_float2(s / a.x, s / a.y); }
inline dsl_float2 operator-(dsl_float2 a) { return dsl_float2(-a.x, -a.y); }

inline dsl_float2& operator+=(dsl_float2& a, dsl_float2 b) { a.x += b.x; a.y += b.y; return a; }
inline dsl_float2& operator-=(dsl_float2& a, dsl_float2 b) { a.x -= b.x; a.y -= b.y; return a; }
inline dsl_float2& operator*=(dsl_float2& a, dsl_float2 b) { a.x *= b.x; a.y *= b.y; return a; }
inline dsl_float2& operator/=(dsl_float2& a, dsl_float2 b) { a.x /= b.x; a.y /= b.y; return a; }
inline dsl_float2& operator*=(dsl_float2& a, float s) { a.x *= s; a.y *= s; return a; }
inline dsl_float2& operator/=(dsl_float2& a, float s) { a.x /= s; a.y /= s; return a; }

// dsl_float3 operators
inline dsl_float3 operator+(dsl_float3 a, dsl_float3 b) { return dsl_float3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline dsl_float3 operator-(dsl_float3 a, dsl_float3 b) { return dsl_float3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline dsl_float3 operator*(dsl_float3 a, dsl_float3 b) { return dsl_float3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline dsl_float3 operator*(dsl_float3 a, float s) { return dsl_float3(a.x * s, a.y * s, a.z * s); }
inline dsl_float3 operator*(float s, dsl_float3 a) { return dsl_float3(s * a.x, s * a.y, s * a.z); }
inline dsl_float3 operator/(dsl_float3 a, float s) { return dsl_float3(a.x / s, a.y / s, a.z / s); }
inline dsl_float3 operator-(dsl_float3 a) { return dsl_float3(-a.x, -a.y, -a.z); }

// dsl_float4 operators
inline dsl_float4 operator+(dsl_float4 a, dsl_float4 b) { return dsl_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline dsl_float4 operator-(dsl_float4 a, dsl_float4 b) { return dsl_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
inline dsl_float4 operator*(dsl_float4 a, dsl_float4 b) { return dsl_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
inline dsl_float4 operator/(dsl_float4 a, dsl_float4 b) { return dsl_float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
inline dsl_float4 operator+(dsl_float4 a, float s) { return dsl_float4(a.x + s, a.y + s, a.z + s, a.w + s); }
inline dsl_float4 operator-(dsl_float4 a, float s) { return dsl_float4(a.x - s, a.y - s, a.z - s, a.w - s); }
inline dsl_float4 operator*(dsl_float4 a, float s) { return dsl_float4(a.x * s, a.y * s, a.z * s, a.w * s); }
inline dsl_float4 operator/(dsl_float4 a, float s) { return dsl_float4(a.x / s, a.y / s, a.z / s, a.w / s); }
inline dsl_float4 operator*(float s, dsl_float4 a) { return dsl_float4(s * a.x, s * a.y, s * a.z, s * a.w); }
inline dsl_float4 operator/(float s, dsl_float4 a) { return dsl_float4(s / a.x, s / a.y, s / a.z, s / a.w); }
inline dsl_float4 operator-(dsl_float4 a) { return dsl_float4(-a.x, -a.y, -a.z, -a.w); }

inline dsl_float4& operator+=(dsl_float4& a, dsl_float4 b) { a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w; return a; }
inline dsl_float4& operator-=(dsl_float4& a, dsl_float4 b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w; return a; }
inline dsl_float4& operator*=(dsl_float4& a, float s) { a.x *= s; a.y *= s; a.z *= s; a.w *= s; return a; }
inline dsl_float4& operator/=(dsl_float4& a, float s) { a.x /= s; a.y /= s; a.z /= s; a.w /= s; return a; }
inline dsl_float4& operator/=(dsl_float4& a, dsl_float4 b) { a.x /= b.x; a.y /= b.y; a.z /= b.z; a.w /= b.w; return a; }

// dsl_uint2 operators
inline dsl_uint2 operator+(dsl_uint2 a, dsl_uint2 b) { return dsl_uint2(a.x + b.x, a.y + b.y); }
inline dsl_uint2 operator-(dsl_uint2 a, dsl_uint2 b) { return dsl_uint2(a.x - b.x, a.y - b.y); }

// Scalar math (NVRTC provides fabsf, fminf, fmaxf, sinf, cosf, etc. as builtins)
inline float abs(float a) { return ::fabsf(a); }
inline float min(float a, float b) { return ::fminf(a, b); }
inline float max(float a, float b) { return ::fmaxf(a, b); }

inline float clamp(float v, float lo, float hi) { return ::fminf(::fmaxf(v, lo), hi); }
inline int   clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

inline float mix(float a, float b, float t) { return a + (b - a) * t; }
inline float fract(float v) { return v - ::floorf(v); }

// Vector math
inline dsl_float2 floor(dsl_float2 v) { return dsl_float2(::floorf(v.x), ::floorf(v.y)); }
inline dsl_float2 fract(dsl_float2 v) { return dsl_float2(fract(v.x), fract(v.y)); }
inline dsl_float2 abs(dsl_float2 v) { return dsl_float2(::fabsf(v.x), ::fabsf(v.y)); }
inline dsl_float2 clamp(dsl_float2 v, float lo, float hi) { return dsl_float2(clamp(v.x, lo, hi), clamp(v.y, lo, hi)); }
inline dsl_float2 min(dsl_float2 a, dsl_float2 b) { return dsl_float2(::fminf(a.x, b.x), ::fminf(a.y, b.y)); }
inline dsl_float2 max(dsl_float2 a, dsl_float2 b) { return dsl_float2(::fmaxf(a.x, b.x), ::fmaxf(a.y, b.y)); }
inline dsl_float2 mix(dsl_float2 a, dsl_float2 b, float t) { return a + (b - a) * t; }

inline dsl_float4 clamp(dsl_float4 v, float lo, float hi) {
    return dsl_float4(clamp(v.x, lo, hi), clamp(v.y, lo, hi), clamp(v.z, lo, hi), clamp(v.w, lo, hi));
}
inline dsl_float4 mix(dsl_float4 a, dsl_float4 b, float t) { return a + (b - a) * t; }
inline dsl_float4 abs(dsl_float4 v) { return dsl_float4(::fabsf(v.x), ::fabsf(v.y), ::fabsf(v.z), ::fabsf(v.w)); }

/// Unified dispatch API — the ONLY way to access invocation coordinates.
/// GPU: maps to threadIdx/blockIdx computation.
/// CPU: reads from dispatch loop variables.
inline dsl_uint2 dispatch_id() {
    return dsl_make_uint2(blockIdx.x * blockDim.x + threadIdx.x,
                          blockIdx.y * blockDim.y + threadIdx.y);
}
inline dsl_uint2 dispatch_size() {
    return dsl_make_uint2(gridDim.x * blockDim.x, gridDim.y * blockDim.y);
}

// Shadow NVRTC built-in vector types with our DSL types.
// All subsequent code sees float2/float3/float4/uint2 as our types with constructors + operators.
#define float2 dsl_float2
#define float3 dsl_float3
#define float4 dsl_float4
#define uint2  dsl_uint2
