#pragma once

#ifndef VEKL_CUDA
#define VEKL_CUDA
#endif

#define kernel          extern "C" __global__
#define constant        const
#define device
#define threadgroup_mem __shared__
#define thread          /* CUDA: no address-space qualifier for thread-local */
#define restrict_ptr    __restrict__
#define restrict        __restrict__

#define param_dev_ro(T, name, s)   const T * __restrict__ name
#define param_dev_rw(T, name, s)   T * __restrict__ name
#define param_dev_wo(T, name, s)   T * __restrict__ name
#define param_dev_cbuf(T, name, s) const T name

#define thread_pos_param(name)
#define thread_pos_init(name) uint2 name = dispatch_id()

#define threadgroup_barrier_all() __syncthreads()

#include "types.h"
#include "dispatch.h"
#include "math.h"
#include "pixel.h"
