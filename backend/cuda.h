#pragma once

#ifndef VEKL_CUDA
#define VEKL_CUDA
#endif

#ifdef USE_HALF_PRECISION
#include <cuda_fp16.h>
#endif

#define kernel          extern "C" __global__
#define constant        __constant__
#define device          __device__
#define threadgroup_mem __shared__
#define thread
#define restrict_ptr    __restrict__
#define restrict        __restrict__
#define host            __host__
#define forceinline     __forceinline__

#define param_dev_ro(T, name, s)   const T * __restrict__ name
#define param_dev_rw(T, name, s)   T * __restrict__ name
#define param_dev_wo(T, name, s)   T * __restrict__ name
#define param_dev_cbuf(T, name, s) const T name

#define thread_pos_param(name)
#define thread_pos_init(name) uint2 name = dispatch_id()

#define threadgroup_barrier_all() __syncthreads()

#ifdef USE_HALF_PRECISION
#define VEKL_FORMAT 8u
#else
#define VEKL_FORMAT 16u
#endif

#define VEKL_EXP2F exp2f
#define VEKL_BACKEND_NAME "cuda"

#define VEKL_VECTOR_TYPES_PROVIDED
#define float2 vekl_float2
#define float3 vekl_float3
#define float4 vekl_float4
#define uint2  vekl_uint2

#include "common/types.h"
#include "common/math.h"
#include "common/pixel.h"
#include "common/logging.h"
#include "common/frame/params.h"

inline uint2 vekl_make_uint2(unsigned int x, unsigned int y) { return uint2(x, y); }

inline uint2 dispatch_id() {
	return vekl_make_uint2(blockIdx.x * blockDim.x + threadIdx.x,
						   blockIdx.y * blockDim.y + threadIdx.y);
}
inline uint2 dispatch_size() {
	return vekl_make_uint2(gridDim.x * blockDim.x, gridDim.y * blockDim.y);
}

#ifdef USE_HALF_PRECISION
struct __align__(8) half4 {
	__half x, y, z, w;
	__device__ __forceinline__ half4() = default;
	__device__ __forceinline__ half4(__half x_, __half y_, __half z_, __half w_) : x(x_), y(y_), z(z_), w(w_) {}
	__device__ __forceinline__ half4(float x_, float y_, float z_, float w_) : x(__float2half(x_)), y(__float2half(y_)), z(__float2half(z_)), w(__float2half(w_)) {}
	__device__ __forceinline__ explicit half4(float4 v) : x(__float2half(v.x)), y(__float2half(v.y)), z(__float2half(v.z)), w(__float2half(v.w)) {}
	__device__ __forceinline__ operator float4() const {
		float4 r;
		r.x = __half2float(x); r.y = __half2float(y); r.z = __half2float(z); r.w = __half2float(w);
		return r;
	}
};
typedef half4 pixel;
#else
typedef float4 pixel;
typedef float4 half4;
#endif

inline float4 pixel_load(const pixel *data, uint pitch_px, uint2 xy, uint /*format*/) {
	return (float4)data[xy.y * pitch_px + xy.x];
}

inline void pixel_store(pixel *data, uint pitch_px, uint2 xy, float4 c, uint /*format*/) {
	data[xy.y * pitch_px + xy.x] = (pixel)c;
}

#if defined(DISABLE_LOGGING) || !defined(DEBUG)

typedef noop_logging_channel logging_channel;

inline __device__ void __vekl_bind_log_buffer(VeklLogBuffer* buffer) {
	(void)buffer;
}

#else

static __device__ VeklLogBuffer* __vekl_log_buffer_ptr = nullptr;

inline __device__ void __vekl_bind_log_buffer(VeklLogBuffer* buffer) {
	__vekl_log_buffer_ptr = buffer;
}

inline __device__ VeklLogBuffer* __vekl_current_log_buffer() {
	return __vekl_log_buffer_ptr;
}

inline __device__ unsigned int __vekl_strnlen(const char* text, unsigned int max_len) {
	if (text == nullptr) {
		return 0u;
	}
	unsigned int len = 0u;
	while (len < max_len && text[len] != '\0') {
		++len;
	}
	return len;
}

inline __device__ void __vekl_copy_bytes(unsigned char* dst, unsigned int cap, const char* src, unsigned int len) {
	for (unsigned int i = 0u; i < cap; ++i) {
		dst[i] = (src != nullptr && i < len) ? (unsigned char)src[i] : (unsigned char)0u;
	}
}

inline __device__ void __vekl_append_log(unsigned int level, const char* channel_name, const char* message) {
	VeklLogBuffer* buffer = __vekl_current_log_buffer();
	if (buffer == nullptr || buffer->capacity == 0u) {
		return;
	}

	unsigned int sequence = atomicAdd(&(buffer->write_index), 1u);
	if (sequence >= buffer->capacity) {
		atomicAdd(&(buffer->overrun_count), 1u);
	}

	unsigned int slot = sequence % buffer->capacity;
	VeklLogEntry* entry = &(buffer->entries[slot]);
	unsigned int channel_len = __vekl_strnlen(channel_name, VEKL_LOG_CHANNEL_CAPACITY - 1u);
	unsigned int message_len = __vekl_strnlen(message, VEKL_LOG_MESSAGE_CAPACITY - 1u);

	entry->level = level;
	entry->channel_len = channel_len;
	entry->message_len = message_len;
	__vekl_copy_bytes(entry->channel, VEKL_LOG_CHANNEL_CAPACITY, channel_name, channel_len);
	__vekl_copy_bytes(entry->message, VEKL_LOG_MESSAGE_CAPACITY, message, message_len);
	__threadfence();
	entry->committed_sequence = sequence + 1u;
	__threadfence();
}

struct logging_channel {
	const char* channel_name;

	inline __device__ void trace(const char* message) const { __vekl_append_log(VEKL_LOG_LEVEL_TRACE, channel_name, message); }
	inline __device__ void debug(const char* message) const { __vekl_append_log(VEKL_LOG_LEVEL_DEBUG, channel_name, message); }
	inline __device__ void info(const char* message) const { __vekl_append_log(VEKL_LOG_LEVEL_INFO, channel_name, message); }
	inline __device__ void warn(const char* message) const { __vekl_append_log(VEKL_LOG_LEVEL_WARN, channel_name, message); }
	inline __device__ void error(const char* message) const { __vekl_append_log(VEKL_LOG_LEVEL_ERROR, channel_name, message); }
};

#endif

#if defined(DISABLE_LOGGING) || !defined(DEBUG)
#define logging (noop_logging_channel{"default"})
#else
static __device__ logging_channel __vekl_default_log = {"default"};
#define log (__vekl_default_log)
#endif
