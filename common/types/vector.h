#pragma once

struct vekl_float2;
struct vekl_float3;
struct vekl_float4;
struct vekl_uint2;

#include "swizzle.h"

struct vekl_float2 {
    union {
        struct { float x, y; };
        struct { float r, g; };
        float vekl_storage[2];
        VEKL_FLOAT2_SWIZZLES(VEKL_DECLARE_SWIZZLE)
    };

    host device forceinline vekl_float2() : x(0), y(0) {}
    host device forceinline vekl_float2(float s) : x(s), y(s) {}
    host device forceinline vekl_float2(float x_, float y_) : x(x_), y(y_) {}
    host device forceinline explicit vekl_float2(vekl_uint2 v);
};

struct vekl_float3 {
    union {
        struct { float x, y, z; };
        struct { float r, g, b; };
        float vekl_storage[3];
        VEKL_FLOAT3_SWIZZLES(VEKL_DECLARE_SWIZZLE)
    };

    host device forceinline vekl_float3() : x(0), y(0), z(0) {}
    host device forceinline vekl_float3(float s) : x(s), y(s), z(s) {}
    host device forceinline vekl_float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct vekl_float4 {
    union {
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
        float vekl_storage[4];
        VEKL_FLOAT4_SWIZZLES(VEKL_DECLARE_SWIZZLE)
    };

    host device forceinline vekl_float4() : x(0), y(0), z(0), w(0) {}
    host device forceinline vekl_float4(float s) : x(s), y(s), z(s), w(s) {}
    host device forceinline vekl_float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    host device forceinline vekl_float4(vekl_float3 v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
};

template<int Lanes, int A, int B>
host device forceinline vekl_swizzle2<Lanes, A, B>::operator vekl_float2() const {
    return vekl_float2(vekl_storage[A], vekl_storage[B]);
}

template<int Lanes, int A, int B>
host device forceinline vekl_swizzle2<Lanes, A, B>& vekl_swizzle2<Lanes, A, B>::operator=(vekl_float2 value) {
    static_assert(A != B, "VEKL duplicate-component swizzles are read-only");
    vekl_storage[A] = value.x;
    vekl_storage[B] = value.y;
    return *this;
}

template<int Lanes, int A, int B, int C>
host device forceinline vekl_swizzle3<Lanes, A, B, C>::operator vekl_float3() const {
    return vekl_float3(vekl_storage[A], vekl_storage[B], vekl_storage[C]);
}

template<int Lanes, int A, int B, int C>
host device forceinline vekl_swizzle3<Lanes, A, B, C>& vekl_swizzle3<Lanes, A, B, C>::operator=(vekl_float3 value) {
    static_assert(A != B && A != C && B != C, "VEKL duplicate-component swizzles are read-only");
    vekl_storage[A] = value.x;
    vekl_storage[B] = value.y;
    vekl_storage[C] = value.z;
    return *this;
}

template<int Lanes, int A, int B, int C, int D>
host device forceinline vekl_swizzle4<Lanes, A, B, C, D>::operator vekl_float4() const {
    return vekl_float4(vekl_storage[A], vekl_storage[B], vekl_storage[C], vekl_storage[D]);
}

template<int Lanes, int A, int B, int C, int D>
host device forceinline vekl_swizzle4<Lanes, A, B, C, D>& vekl_swizzle4<Lanes, A, B, C, D>::operator=(vekl_float4 value) {
    static_assert(A != B && A != C && A != D && B != C && B != D && C != D,
                  "VEKL duplicate-component swizzles are read-only");
    vekl_storage[A] = value.x;
    vekl_storage[B] = value.y;
    vekl_storage[C] = value.z;
    vekl_storage[D] = value.w;
    return *this;
}

static_assert(sizeof(vekl_float2) == sizeof(float) * 2, "vekl_float2 must remain compact");
static_assert(sizeof(vekl_float3) == sizeof(float) * 3, "vekl_float3 must remain compact");
static_assert(sizeof(vekl_float4) == sizeof(float) * 4, "vekl_float4 must remain compact");

struct vekl_uint2 {
    unsigned int x, y;
    host device forceinline vekl_uint2() : x(0), y(0) {}
    host device forceinline vekl_uint2(unsigned int s) : x(s), y(s) {}
    host device forceinline vekl_uint2(unsigned int x_, unsigned int y_) : x(x_), y(y_) {}
    host device forceinline vekl_uint2(int x_, int y_) : x((unsigned int)x_), y((unsigned int)y_) {}
    host device forceinline vekl_uint2(float x_, float y_) : x((unsigned int)x_), y((unsigned int)y_) {}
    host device forceinline explicit vekl_uint2(vekl_float2 v);
};

#undef VEKL_DECLARE_SWIZZLE
#undef VEKL_DECLARE_SWIZZLE_IMPL
#undef VEKL_DECLARE_SWIZZLE_2
#undef VEKL_DECLARE_SWIZZLE_3
#undef VEKL_DECLARE_SWIZZLE_4
