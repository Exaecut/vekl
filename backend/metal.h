#pragma once

#ifndef VEKL_METAL
#define VEKL_METAL
#endif

#include <metal_stdlib>
using namespace metal;

#define kernel          kernel
#define device          device
#define constant        constant
#define threadgroup_mem threadgroup
#define thread          thread
#define restrict_ptr    __restrict
#define restrict        __restrict
#define host
#define forceinline     __attribute__((always_inline)) inline

#define param_dev_ro(T, name, s)   device const T* name [[buffer(s)]]
#define param_dev_rw(T, name, s)   device T* name [[buffer(s)]]
#define param_dev_wo(T, name, s)   device T* name [[buffer(s)]]
#define param_dev_cbuf(T, name, s) constant T& name [[buffer(s)]]

#define thread_pos_param(name)
#define thread_pos_init(name)   uint2 name = dispatch_id()

#define threadgroup_barrier_all() threadgroup_barrier(mem_flags::mem_threadgroup)

typedef unsigned int   uint;
typedef unsigned char  uchar;
typedef unsigned short ushort;

#ifdef USE_HALF_PRECISION
#define VEKL_FORMAT 8u
#else
#define VEKL_FORMAT 16u
#endif

#define sqrtf(x)    sqrt(x)
#define expf(x)     exp(x)
#define exp2f(x)    exp2(x)
#define sinf(x)     sin(x)
#define cosf(x)     cos(x)
#define atan2f(y,x) atan2(y,x)
#define floorf(x)   floor(x)
#define fabsf(x)    fabs(x)
#define fmodf(x, y) fmod(x, y)
#define log2f(x)    log2(x)
#define powf(x, y)  pow(x, y)
#define fminf(x, y) fmin(x, y)
#define fmaxf(x, y) fmax(x, y)
#define fmaf(a,b,c) fma(a,b,c)

#define VEKL_EXP2F exp2f
#define VEKL_BACKEND_NAME "metal"

#define dispatch_id()   __vekl_dispatch_id
#define dispatch_size() __vekl_dispatch_size

#define VEKL_VECTOR_TYPES_PROVIDED
#define VEKL_NATIVE_VECTOR_TYPES_PROVIDED
#define VEKL_MATH_PROVIDED

#include "common/types.h"
#include "common/math.h"
#include "common/pixel.h"
#include "common/logging.h"
#include "common/frame/params.h"

#ifdef USE_HALF_PRECISION
typedef half4 pixel;
#else
typedef float4 pixel;
#endif

inline float4 pixel_load(device const pixel *data, uint pitch_px, uint2 xy, uint /*format*/) {
	return float4(data[xy.y * pitch_px + xy.x]);
}

inline void pixel_store(device pixel *data, uint pitch_px, uint2 xy, float4 c, uint /*format*/) {
	data[xy.y * pitch_px + xy.x] = pixel(c);
}

#if defined(DISABLE_LOGGING) || !defined(DEBUG)

typedef noop_logging_channel logging_channel;

#else

static thread device VeklLogBuffer* __vekl_log_buffer_ptr = nullptr;

inline void __vekl_bind_log_buffer(device VeklLogBuffer* buffer) {
	__vekl_log_buffer_ptr = buffer;
}

inline device VeklLogBuffer* __vekl_current_log_buffer() {
	return __vekl_log_buffer_ptr;
}

inline uint __vekl_strnlen(const constant char* text, uint max_len) {
	if (text == nullptr) {
		return 0u;
	}
	uint len = 0u;
	while (len < max_len && text[len] != '\0') {
		++len;
	}
	return len;
}

inline void __vekl_append_log(uint level, const constant char* channel_name, const constant char* message) {
	device VeklLogBuffer* buffer = __vekl_current_log_buffer();
	if (buffer == nullptr || buffer->capacity == 0u) {
		return;
	}

	device atomic_uint* write_index = (device atomic_uint*)&buffer->write_index;
	device atomic_uint* overrun_count_ptr = (device atomic_uint*)&buffer->overrun_count;
	uint sequence = atomic_fetch_add_explicit(write_index, 1u, memory_order_relaxed);
	if (sequence >= buffer->capacity) {
		atomic_fetch_add_explicit(overrun_count_ptr, 1u, memory_order_relaxed);
	}

	uint slot = sequence % buffer->capacity;
	device VeklLogEntry* entry = &(buffer->entries[slot]);
	uint channel_len = __vekl_strnlen(channel_name, VEKL_LOG_CHANNEL_CAPACITY - 1u);
	uint message_len = __vekl_strnlen(message, VEKL_LOG_MESSAGE_CAPACITY - 1u);

	entry->level = level;
	entry->channel_len = channel_len;
	entry->message_len = message_len;
	for (uint i = 0u; i < VEKL_LOG_CHANNEL_CAPACITY; ++i) {
		entry->channel[i] = (channel_name != nullptr && i < channel_len) ? (uchar)channel_name[i] : (uchar)0u;
	}
	for (uint i = 0u; i < VEKL_LOG_MESSAGE_CAPACITY; ++i) {
		entry->message[i] = (message != nullptr && i < message_len) ? (uchar)message[i] : (uchar)0u;
	}
	atomic_thread_fence(memory_order_release, memory_scope_device);
	entry->committed_sequence = sequence + 1u;
	atomic_thread_fence(memory_order_release, memory_scope_device);
}

struct logging_channel {
	const constant char* channel_name;

	inline logging_channel(const constant char* channel = "default") : channel_name(channel) {}
	inline void trace(const constant char* message) const { __vekl_append_log(VEKL_LOG_LEVEL_TRACE, channel_name, message); }
	inline void debug(const constant char* message) const { __vekl_append_log(VEKL_LOG_LEVEL_DEBUG, channel_name, message); }
	inline void info(const constant char* message)  const { __vekl_append_log(VEKL_LOG_LEVEL_INFO,  channel_name, message); }
	inline void warn(const constant char* message)  const { __vekl_append_log(VEKL_LOG_LEVEL_WARN,  channel_name, message); }
	inline void error(const constant char* message) const { __vekl_append_log(VEKL_LOG_LEVEL_ERROR, channel_name, message); }
};

#endif

#if defined(DISABLE_LOGGING) || !defined(DEBUG)
#define logging (noop_logging_channel{"default"})
#else
constant logging_channel __vekl_default_log {"default"};
#define logging (__vekl_default_log)
#endif
