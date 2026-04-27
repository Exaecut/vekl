#pragma once

#if !defined(VEKL_CPU) && !defined(VEKL_CUDA) && !defined(VEKL_METAL) && !defined(VEKL_OPENCL)
    #if defined(__CUDACC__) || defined(__CUDA_ARCH__)
        #define VEKL_CUDA
    #elif defined(__METAL_VERSION__)
        #define VEKL_METAL
    #elif defined(__OPENCL_VERSION__)
        #define VEKL_OPENCL
    #else
        #define VEKL_CPU
    #endif
#endif

#if defined(VEKL_CPU)
    #include "backend/cpu.h"
#elif defined(VEKL_CUDA)
    #include "backend/cuda.h"
#elif defined(VEKL_METAL)
    #include "backend/metal.h"
#elif defined(VEKL_OPENCL)
    #include "backend/opencl.h"
#else
    #error "Unknown VEKL backend. Define one of: VEKL_CPU, VEKL_CUDA, VEKL_METAL, VEKL_OPENCL"
#endif

#include "maths/transform.h"
#include "maths/trigonometry.h"
#include "maths/easing.h"
#include "maths/color.h"

#include "image/coords.h"
#include "image/2d.h"
#include "image/tonemapping.h"

#include "sdf/operators.h"
#include "sdf/shapes.h"

#include "filters/blur.h"
#include "filters/radial_blur.h"
#include "filters/envelope.h"
