#pragma once

#include "vector.h"

inline vekl_float2::vekl_float2(vekl_uint2 v) : x((float)v.x), y((float)v.y) {}
inline vekl_uint2::vekl_uint2(vekl_float2 v) : x((unsigned int)v.x), y((unsigned int)v.y) {}
