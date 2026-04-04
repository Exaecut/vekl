#pragma once

#if defined(__CUDACC__) || defined(__cuda_cuda_h__)
    #include "dsl_cuda.h"
#elif defined(__METAL_VERSION__)
    #include "dsl_metal.h"
#else
    // Fallback to CUDA DSL if no known backend is specified
    #include "dsl_cuda.h"
#endif