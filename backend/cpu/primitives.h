#pragma once

#ifndef VEKL_CPU
#define VEKL_CPU
#endif

#include <math.h>
#include <stdint.h>
#include <string.h>

#define kernel              static inline
#define constant            const
#define device
#define threadgroup_mem     static
#define thread              /* CPU: no address-space qualifier */
#define restrict_ptr
#define restrict

#define param_dev_ro(T, name, s)   const T * __restrict name
#define param_dev_rw(T, name, s)   T * __restrict name
#define param_dev_wo(T, name, s)   T * __restrict name
#define param_dev_cbuf(T, name, s) const T name

#define thread_pos_param(name)
#define thread_pos_init(name) uint2 name = dispatch_id()

#define threadgroup_barrier_all()

#include "types.h"
#include "dispatch.h"
#include "math.h"
#include "pixel.h"
#include "logging.h"
