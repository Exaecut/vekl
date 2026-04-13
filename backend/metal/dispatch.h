#pragma once

#ifdef USE_HALF_PRECISION
#define format 8u
#else
#define format 16u
#endif

#define dispatch_id()   __vekl_dispatch_id
#define dispatch_size() __vekl_dispatch_size
