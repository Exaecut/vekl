#pragma once

#ifndef VEKL_METAL
#define VEKL_METAL
#endif

#include <metal_stdlib>
using namespace metal;

#define kernel          kernel
#define device          device
#define constant        constant
#define threadgroup_mem threadgroup
#define thread          thread
#define restrict_ptr    __restrict
#define restrict        __restrict

#define param_dev_ro(T, name, s)   device const T* name [[buffer(s)]]
#define param_dev_rw(T, name, s)   device T* name [[buffer(s)]]
#define param_dev_wo(T, name, s)   device T* name [[buffer(s)]]
#define param_dev_cbuf(T, name, s) constant T& name [[buffer(s)]]

#define thread_pos_param(name)
#define thread_pos_init(name)   uint2 name = dispatch_id()

#define threadgroup_barrier_all() threadgroup_barrier(mem_flags::mem_threadgroup)

#include "types.h"
#include "dispatch.h"
#include "math.h"
#include "pixel.h"
#include "logging.h"
