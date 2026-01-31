#pragma once

#if defined(__CUDACC__)
    #include "dsl_cuda.h"
#else
    #include "dsl_metal.h"
#endif
