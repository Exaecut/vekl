#pragma once

// # VEKL IntelliSense Compatibility Header
//
// Maps all GPU-backend-specific address space qualifiers and function attributes
// to their C++ equivalents (mostly empty / const) so that any VEKL header or
// .vekl shader file is fully analyzable by a C++ IntelliSense engine.
//
// THIS FILE IS NEVER INCLUDED IN ACTUAL COMPILATION.  It is referenced only by
// .vscode/c_cpp_properties.json via forcedInclude so that the IDE sees valid
// C++ for all code paths.

#if !defined(__CUDACC__) && !defined(__METAL_VERSION__)

// Backend selector
#define VEKL_CPU    1

// Metal Shading Language address-space qualifiers 
#define constant    const
#define device
#define threadgroup
#define thread

// Metal atomic types that IntelliSense doesn't know 
typedef unsigned int atomic_uint;
#define memory_order_relaxed 0
#define memory_order_release 1
#define memory_scope_device  2

// CUDA device / host qualifiers 
#define __device__
#define __host__
#define __global__
#define __shared__
#define __constant__

// Suppress nullptr-on-unsupported-GPU warnings ─
#ifndef nullptr
#define nullptr 0
#endif

#endif // !__CUDACC__ && !__METAL_VERSION__
