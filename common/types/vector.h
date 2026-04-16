#pragma once

struct vekl_float2;
struct vekl_float3;
struct vekl_float4;
struct vekl_uint2;

struct vekl_float2 {
    float x, y;
    vekl_float2() : x(0), y(0) {}
    vekl_float2(float s) : x(s), y(s) {}
    vekl_float2(float x_, float y_) : x(x_), y(y_) {}
    explicit vekl_float2(vekl_uint2 v);
};

struct vekl_float3 {
    union { float x; float r; };
    union { float y; float g; };
    union { float z; float b; };
    vekl_float3() : x(0), y(0), z(0) {}
    vekl_float3(float s) : x(s), y(s), z(s) {}
    vekl_float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct vekl_float4 {
    union { float x; float r; };
    union { float y; float g; };
    union { float z; float b; };
    union { float w; float a; };
    vekl_float4() : x(0), y(0), z(0), w(0) {}
    vekl_float4(float s) : x(s), y(s), z(s), w(s) {}
    vekl_float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    vekl_float4(vekl_float3 v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
    vekl_float3 xyz() const { return vekl_float3(x, y, z); }
};

struct vekl_uint2 {
    unsigned int x, y;
    vekl_uint2() : x(0), y(0) {}
    vekl_uint2(unsigned int s) : x(s), y(s) {}
    vekl_uint2(unsigned int x_, unsigned int y_) : x(x_), y(y_) {}
    vekl_uint2(int x_, int y_) : x((unsigned int)x_), y((unsigned int)y_) {}
    vekl_uint2(float x_, float y_) : x((unsigned int)x_), y((unsigned int)y_) {}
    explicit vekl_uint2(vekl_float2 v);
};
