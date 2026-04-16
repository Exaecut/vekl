#pragma once

#if !defined(__INTELLISENSE__) && !defined(VEKL_INTELLISENSE_COMPAT)
#error "This file must only be used by IntelliSense. Never include it in real compilation."
#endif

#define VEKL_CPU 1
#define VEKL_INTELLISENSE_COMPAT 1

#define constant    const
#define device
#define threadgroup_mem static
#define thread
#define restrict_ptr
#define restrict
#define kernel      static inline

#define __device__
#define __host__
#define __global__
#define __shared__
#define __constant__

typedef unsigned int atomic_uint;
#define memory_order_relaxed 0
#define memory_order_release 1
#define memory_scope_device  2

#ifndef nullptr
#define nullptr 0
#endif
