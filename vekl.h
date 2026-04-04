#pragma once

#if defined(__CUDACC__) || defined(__cuda_cuda_h__)
    #include "vekl_cuda.h"
#elif defined(__METAL_VERSION__)
    #include "vekl_metal.h"
#else
    #include "vekl_cpu.h"
#endif

#ifndef UTILS_COMMON
#define UTILS_COMMON

#include "types.h"

// Maths
#include "maths/transform.h"
#include "maths/trigonometry.h"
#include "maths/easing.h"

// Image manipulation
#include "image/coords.h"
#include "image/2d.h"

#endif // UTILS_COMMON
