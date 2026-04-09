#pragma once

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
    union { float x; float r; };
    union { float y; float g; };
    union { float z; float b; };
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

typedef unsigned int   uint;
typedef unsigned char  uchar;
typedef unsigned short ushort;

inline dsl_float2::dsl_float2(dsl_uint2 v) : x((float)v.x), y((float)v.y) {}
inline dsl_uint2::dsl_uint2(dsl_float2 v) : x((unsigned int)v.x), y((unsigned int)v.y) {}

inline dsl_uint2 dsl_make_uint2(unsigned int x, unsigned int y) { return dsl_uint2(x, y); }

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

inline dsl_float3 operator+(dsl_float3 a, dsl_float3 b) { return dsl_float3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline dsl_float3 operator-(dsl_float3 a, dsl_float3 b) { return dsl_float3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline dsl_float3 operator*(dsl_float3 a, dsl_float3 b) { return dsl_float3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline dsl_float3 operator*(dsl_float3 a, float s) { return dsl_float3(a.x * s, a.y * s, a.z * s); }
inline dsl_float3 operator*(float s, dsl_float3 a) { return dsl_float3(s * a.x, s * a.y, s * a.z); }
inline dsl_float3 operator/(dsl_float3 a, dsl_float3 b) { return dsl_float3(a.x / b.x, a.y / b.y, a.z / b.z); }
inline dsl_float3 operator/(dsl_float3 a, float s) { return dsl_float3(a.x / s, a.y / s, a.z / s); }
inline dsl_float3 operator-(dsl_float3 a) { return dsl_float3(-a.x, -a.y, -a.z); }
inline dsl_float3 operator+(dsl_float3 a, float s) { return dsl_float3(a.x + s, a.y + s, a.z + s); }
inline dsl_float3 operator+(float s, dsl_float3 a) { return dsl_float3(s + a.x, s + a.y, s + a.z); }
inline dsl_float3 operator-(dsl_float3 a, float s) { return dsl_float3(a.x - s, a.y - s, a.z - s); }

inline dsl_float3& operator+=(dsl_float3& a, dsl_float3 b) { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
inline dsl_float3& operator+=(dsl_float3& a, float s) { a.x += s; a.y += s; a.z += s; return a; }
inline dsl_float3& operator-=(dsl_float3& a, dsl_float3 b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
inline dsl_float3& operator*=(dsl_float3& a, float s) { a.x *= s; a.y *= s; a.z *= s; return a; }
inline dsl_float3& operator/=(dsl_float3& a, float s) { a.x /= s; a.y /= s; a.z /= s; return a; }

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

inline dsl_uint2 operator+(dsl_uint2 a, dsl_uint2 b) { return dsl_uint2(a.x + b.x, a.y + b.y); }
inline dsl_uint2 operator-(dsl_uint2 a, dsl_uint2 b) { return dsl_uint2(a.x - b.x, a.y - b.y); }

#define float2 dsl_float2
#define float3 dsl_float3
#define float4 dsl_float4
#define uint2  dsl_uint2

#ifndef M_PI_F
#define M_PI_F 3.14159265358979323846f
#endif
