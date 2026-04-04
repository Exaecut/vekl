// dsl_metal.h
#pragma once

#include <metal_stdlib>
using namespace metal;

// -----------------------------------------------------------------------------
// Keywords and Address Spaces
// -----------------------------------------------------------------------------

#define kernel          kernel
#define device          device
#define constant        constant
#define threadgroup_mem threadgroup
#define thread_local    thread
#define restrict        __restrict
#define restrict_ptr    __restrict

// internal helper macros for address space
#define device_ptr(T)   device T*
#define device_cptr(T)  device const T*
#define constant_ref(T) constant T&

// -----------------------------------------------------------------------------
// Scalar Types
// -----------------------------------------------------------------------------

// Metal supports uint, uchar, ushort natively via metal namespace.
// Ensure global scope visibility if needed (though using namespace metal handles it).
typedef unsigned int uint;

// -----------------------------------------------------------------------------
// Vector Types
// -----------------------------------------------------------------------------

// Metal provides native types: float2, float3, float4, uint2, etc.
// They support .xyzw, .rgba, constructors, and math operators natively.
// No extra definition required.

// -----------------------------------------------------------------------------
// Kernel Parameter Macros
// -----------------------------------------------------------------------------

// Map macros to Metal buffer attribute syntax
#define param_dev_ro(T, name, s)   device_cptr(T) name [[buffer(s)]]
#define param_dev_rw(T, name, s)   device_ptr(T) name [[buffer(s)]]
#define param_dev_wo(T, name, s)   device_ptr(T) name [[buffer(s)]]
#define param_dev_cbuf(T, name, s) constant_ref(T) name [[buffer(s)]]

// -----------------------------------------------------------------------------
// Thread Position & Synchronization Macros
// -----------------------------------------------------------------------------

// Metal puts the thread position in the argument list
#define thread_pos_param(name) uint2 name [[thread_position_in_grid]]

// No initialization logic needed inside the body (already passed as arg)
#define thread_pos_init(name)

#define threadgroup_barrier_all() threadgroup_barrier(mem_flags::mem_threadgroup)

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// Metal overloads abs() for floats natively.