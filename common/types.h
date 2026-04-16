#pragma once

#ifdef _MSC_VER
#define noinline __declspec(noinline)
#else
#define noinline __attribute__((noinline))
#endif

#include "types/scalar.h"
#include "types/vector.h"
#include "types/convert.h"
#include "types/ops.h"

#ifndef VEKL_VECTOR_TYPES_PROVIDED
using float2 = vekl_float2;
using float3 = vekl_float3;
using float4 = vekl_float4;
using uint2  = vekl_uint2;
#endif
