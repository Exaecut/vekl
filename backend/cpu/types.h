#pragma once

struct float2;
struct float3;
struct float4;
struct uint2;

struct float2 {
    float x, y;
    float2() : x(0), y(0) {}
    float2(float s) : x(s), y(s) {}
    float2(float x_, float y_) : x(x_), y(y_) {}
    explicit float2(uint2 v);
};

struct float3 {
    union { float x; float r; };
    union { float y; float g; };
    union { float z; float b; };
    float3() : x(0), y(0), z(0) {}
    float3(float s) : x(s), y(s), z(s) {}
    float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct float4 {
    union { float x; float r; };
    union { float y; float g; };
    union { float z; float b; };
    union { float w; float a; };
    float4() : x(0), y(0), z(0), w(0) {}
    float4(float s) : x(s), y(s), z(s), w(s) {}
    float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    float4(float3 v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
    float3 xyz() const { return float3(x, y, z); }
};

struct uint2 {
    unsigned int x, y;
    uint2() : x(0), y(0) {}
    uint2(unsigned int s) : x(s), y(s) {}
    uint2(unsigned int x_, unsigned int y_) : x(x_), y(y_) {}
    uint2(int x_, int y_) : x((unsigned int)x_), y((unsigned int)y_) {}
    uint2(float x_, float y_) : x((unsigned int)x_), y((unsigned int)y_) {}
    explicit uint2(float2 v);
};

typedef unsigned int   uint;
typedef unsigned char  uchar;
typedef unsigned short ushort;

inline float2::float2(uint2 v) : x((float)v.x), y((float)v.y) {}
inline uint2::uint2(float2 v) : x((unsigned int)v.x), y((unsigned int)v.y) {}

inline float2 operator+(float2 a, float2 b) { return float2(a.x + b.x, a.y + b.y); }
inline float2 operator-(float2 a, float2 b) { return float2(a.x - b.x, a.y - b.y); }
inline float2 operator*(float2 a, float2 b) { return float2(a.x * b.x, a.y * b.y); }
inline float2 operator/(float2 a, float2 b) { return float2(a.x / b.x, a.y / b.y); }
inline float2 operator+(float2 a, float s) { return float2(a.x + s, a.y + s); }
inline float2 operator-(float2 a, float s) { return float2(a.x - s, a.y - s); }
inline float2 operator*(float2 a, float s) { return float2(a.x * s, a.y * s); }
inline float2 operator/(float2 a, float s) { return float2(a.x / s, a.y / s); }
inline float2 operator+(float s, float2 a) { return float2(s + a.x, s + a.y); }
inline float2 operator-(float s, float2 a) { return float2(s - a.x, s - a.y); }
inline float2 operator*(float s, float2 a) { return float2(s * a.x, s * a.y); }
inline float2 operator/(float s, float2 a) { return float2(s / a.x, s / a.y); }
inline float2 operator-(float2 a) { return float2(-a.x, -a.y); }

inline float2& operator+=(float2& a, float2 b) { a.x += b.x; a.y += b.y; return a; }
inline float2& operator-=(float2& a, float2 b) { a.x -= b.x; a.y -= b.y; return a; }
inline float2& operator*=(float2& a, float2 b) { a.x *= b.x; a.y *= b.y; return a; }
inline float2& operator/=(float2& a, float2 b) { a.x /= b.x; a.y /= b.y; return a; }
inline float2& operator*=(float2& a, float s) { a.x *= s; a.y *= s; return a; }
inline float2& operator/=(float2& a, float s) { a.x /= s; a.y /= s; return a; }

inline float3 operator+(float3 a, float3 b) { return float3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline float3 operator-(float3 a, float3 b) { return float3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline float3 operator*(float3 a, float3 b) { return float3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline float3 operator*(float3 a, float s) { return float3(a.x * s, a.y * s, a.z * s); }
inline float3 operator*(float s, float3 a) { return float3(s * a.x, s * a.y, s * a.z); }
inline float3 operator/(float3 a, float3 b) { return float3(a.x / b.x, a.y / b.y, a.z / b.z); }
inline float3 operator/(float3 a, float s) { return float3(a.x / s, a.y / s, a.z / s); }
inline float3 operator-(float3 a) { return float3(-a.x, -a.y, -a.z); }
inline float3 operator+(float3 a, float s) { return float3(a.x + s, a.y + s, a.z + s); }
inline float3 operator+(float s, float3 a) { return float3(s + a.x, s + a.y, s + a.z); }
inline float3 operator-(float3 a, float s) { return float3(a.x - s, a.y - s, a.z - s); }

inline float3& operator+=(float3& a, float3 b) { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
inline float3& operator+=(float3& a, float s) { a.x += s; a.y += s; a.z += s; return a; }
inline float3& operator-=(float3& a, float3 b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
inline float3& operator*=(float3& a, float s) { a.x *= s; a.y *= s; a.z *= s; return a; }
inline float3& operator/=(float3& a, float s) { a.x /= s; a.y /= s; a.z /= s; return a; }

inline float4 operator+(float4 a, float4 b) { return float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline float4 operator-(float4 a, float4 b) { return float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
inline float4 operator*(float4 a, float4 b) { return float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
inline float4 operator/(float4 a, float4 b) { return float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
inline float4 operator+(float4 a, float s) { return float4(a.x + s, a.y + s, a.z + s, a.w + s); }
inline float4 operator-(float4 a, float s) { return float4(a.x - s, a.y - s, a.z - s, a.w - s); }
inline float4 operator*(float4 a, float s) { return float4(a.x * s, a.y * s, a.z * s, a.w * s); }
inline float4 operator/(float4 a, float s) { return float4(a.x / s, a.y / s, a.z / s, a.w / s); }
inline float4 operator*(float s, float4 a) { return float4(s * a.x, s * a.y, s * a.z, s * a.w); }
inline float4 operator/(float s, float4 a) { return float4(s / a.x, s / a.y, s / a.z, s / a.w); }
inline float4 operator-(float4 a) { return float4(-a.x, -a.y, -a.z, -a.w); }

inline float4& operator+=(float4& a, float4 b) { a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w; return a; }
inline float4& operator-=(float4& a, float4 b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w; return a; }
inline float4& operator*=(float4& a, float s) { a.x *= s; a.y *= s; a.z *= s; a.w *= s; return a; }
inline float4& operator/=(float4& a, float s) { a.x /= s; a.y /= s; a.z /= s; a.w /= s; return a; }
inline float4& operator/=(float4& a, float4 b) { a.x /= b.x; a.y /= b.y; a.z /= b.z; a.w /= b.w; return a; }

inline uint2 operator+(uint2 a, uint2 b) { return uint2(a.x + b.x, a.y + b.y); }
inline uint2 operator-(uint2 a, uint2 b) { return uint2(a.x - b.x, a.y - b.y); }

#ifndef M_PI_F
#define M_PI_F 3.14159265358979323846f
#endif
