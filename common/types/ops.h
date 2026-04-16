#pragma once

#include "vector.h"

inline vekl_float2 operator+(vekl_float2 a, vekl_float2 b) { return vekl_float2(a.x + b.x, a.y + b.y); }
inline vekl_float2 operator-(vekl_float2 a, vekl_float2 b) { return vekl_float2(a.x - b.x, a.y - b.y); }
inline vekl_float2 operator*(vekl_float2 a, vekl_float2 b) { return vekl_float2(a.x * b.x, a.y * b.y); }
inline vekl_float2 operator/(vekl_float2 a, vekl_float2 b) { return vekl_float2(a.x / b.x, a.y / b.y); }
inline vekl_float2 operator+(vekl_float2 a, float s) { return vekl_float2(a.x + s, a.y + s); }
inline vekl_float2 operator-(vekl_float2 a, float s) { return vekl_float2(a.x - s, a.y - s); }
inline vekl_float2 operator*(vekl_float2 a, float s) { return vekl_float2(a.x * s, a.y * s); }
inline vekl_float2 operator/(vekl_float2 a, float s) { return vekl_float2(a.x / s, a.y / s); }
inline vekl_float2 operator+(float s, vekl_float2 a) { return vekl_float2(s + a.x, s + a.y); }
inline vekl_float2 operator-(float s, vekl_float2 a) { return vekl_float2(s - a.x, s - a.y); }
inline vekl_float2 operator*(float s, vekl_float2 a) { return vekl_float2(s * a.x, s * a.y); }
inline vekl_float2 operator/(float s, vekl_float2 a) { return vekl_float2(s / a.x, s / a.y); }
inline vekl_float2 operator-(vekl_float2 a) { return vekl_float2(-a.x, -a.y); }

inline vekl_float2& operator+=(vekl_float2& a, vekl_float2 b) { a.x += b.x; a.y += b.y; return a; }
inline vekl_float2& operator-=(vekl_float2& a, vekl_float2 b) { a.x -= b.x; a.y -= b.y; return a; }
inline vekl_float2& operator*=(vekl_float2& a, vekl_float2 b) { a.x *= b.x; a.y *= b.y; return a; }
inline vekl_float2& operator/=(vekl_float2& a, vekl_float2 b) { a.x /= b.x; a.y /= b.y; return a; }
inline vekl_float2& operator*=(vekl_float2& a, float s) { a.x *= s; a.y *= s; return a; }
inline vekl_float2& operator/=(vekl_float2& a, float s) { a.x /= s; a.y /= s; return a; }

inline vekl_float3 operator+(vekl_float3 a, vekl_float3 b) { return vekl_float3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline vekl_float3 operator-(vekl_float3 a, vekl_float3 b) { return vekl_float3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline vekl_float3 operator*(vekl_float3 a, vekl_float3 b) { return vekl_float3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline vekl_float3 operator*(vekl_float3 a, float s) { return vekl_float3(a.x * s, a.y * s, a.z * s); }
inline vekl_float3 operator*(float s, vekl_float3 a) { return vekl_float3(s * a.x, s * a.y, s * a.z); }
inline vekl_float3 operator/(vekl_float3 a, vekl_float3 b) { return vekl_float3(a.x / b.x, a.y / b.y, a.z / b.z); }
inline vekl_float3 operator/(vekl_float3 a, float s) { return vekl_float3(a.x / s, a.y / s, a.z / s); }
inline vekl_float3 operator-(vekl_float3 a) { return vekl_float3(-a.x, -a.y, -a.z); }
inline vekl_float3 operator+(vekl_float3 a, float s) { return vekl_float3(a.x + s, a.y + s, a.z + s); }
inline vekl_float3 operator+(float s, vekl_float3 a) { return vekl_float3(s + a.x, s + a.y, s + a.z); }
inline vekl_float3 operator-(vekl_float3 a, float s) { return vekl_float3(a.x - s, a.y - s, a.z - s); }

inline vekl_float3& operator+=(vekl_float3& a, vekl_float3 b) { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
inline vekl_float3& operator+=(vekl_float3& a, float s) { a.x += s; a.y += s; a.z += s; return a; }
inline vekl_float3& operator-=(vekl_float3& a, vekl_float3 b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
inline vekl_float3& operator*=(vekl_float3& a, float s) { a.x *= s; a.y *= s; a.z *= s; return a; }
inline vekl_float3& operator/=(vekl_float3& a, float s) { a.x /= s; a.y /= s; a.z /= s; return a; }

inline vekl_float4 operator+(vekl_float4 a, vekl_float4 b) { return vekl_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline vekl_float4 operator-(vekl_float4 a, vekl_float4 b) { return vekl_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
inline vekl_float4 operator*(vekl_float4 a, vekl_float4 b) { return vekl_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
inline vekl_float4 operator/(vekl_float4 a, vekl_float4 b) { return vekl_float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
inline vekl_float4 operator+(vekl_float4 a, float s) { return vekl_float4(a.x + s, a.y + s, a.z + s, a.w + s); }
inline vekl_float4 operator-(vekl_float4 a, float s) { return vekl_float4(a.x - s, a.y - s, a.z - s, a.w - s); }
inline vekl_float4 operator*(vekl_float4 a, float s) { return vekl_float4(a.x * s, a.y * s, a.z * s, a.w * s); }
inline vekl_float4 operator/(vekl_float4 a, float s) { return vekl_float4(a.x / s, a.y / s, a.z / s, a.w / s); }
inline vekl_float4 operator*(float s, vekl_float4 a) { return vekl_float4(s * a.x, s * a.y, s * a.z, s * a.w); }
inline vekl_float4 operator/(float s, vekl_float4 a) { return vekl_float4(s / a.x, s / a.y, s / a.z, s / a.w); }
inline vekl_float4 operator-(vekl_float4 a) { return vekl_float4(-a.x, -a.y, -a.z, -a.w); }

inline vekl_float4& operator+=(vekl_float4& a, vekl_float4 b) { a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w; return a; }
inline vekl_float4& operator-=(vekl_float4& a, vekl_float4 b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w; return a; }
inline vekl_float4& operator*=(vekl_float4& a, float s) { a.x *= s; a.y *= s; a.z *= s; a.w *= s; return a; }
inline vekl_float4& operator/=(vekl_float4& a, float s) { a.x /= s; a.y /= s; a.z /= s; a.w /= s; return a; }
inline vekl_float4& operator/=(vekl_float4& a, vekl_float4 b) { a.x /= b.x; a.y /= b.y; a.z /= b.z; a.w /= b.w; return a; }

inline vekl_uint2 operator+(vekl_uint2 a, vekl_uint2 b) { return vekl_uint2(a.x + b.x, a.y + b.y); }
inline vekl_uint2 operator-(vekl_uint2 a, vekl_uint2 b) { return vekl_uint2(a.x - b.x, a.y - b.y); }
inline vekl_uint2 operator*(vekl_uint2 a, vekl_uint2 b) { return vekl_uint2(a.x * b.x, a.y * b.y); }
inline vekl_uint2 operator/(vekl_uint2 a, vekl_uint2 b) { return vekl_uint2(a.x / b.x, a.y / b.y); }
