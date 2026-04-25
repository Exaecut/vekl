#pragma once

#include "vector.h"

host device forceinline vekl_float2::vekl_float2(vekl_uint2 v) : x((float)v.x), y((float)v.y) {}
host device forceinline vekl_uint2::vekl_uint2(vekl_float2 v) : x((unsigned int)v.x), y((unsigned int)v.y) {}
