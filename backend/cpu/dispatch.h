#pragma once

thread_local static unsigned int __cpu_gid_x;
thread_local static unsigned int __cpu_gid_y;
thread_local static unsigned int __cpu_dispatch_w;
thread_local static unsigned int __cpu_dispatch_h;
thread_local static unsigned int __cpu_format;

#define format (__cpu_format)

inline uint2 dispatch_id()   { return uint2(__cpu_gid_x, __cpu_gid_y); }
inline uint2 dispatch_size() { return uint2(__cpu_dispatch_w, __cpu_dispatch_h); }
