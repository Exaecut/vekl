#pragma once

#ifndef VEKL_CPU
#define VEKL_CPU
#endif

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define kernel              static inline
#define constant            const
#define device
#define threadgroup_mem     static
#define thread
#define restrict_ptr
#define restrict

#define param_dev_ro(T, name, s)   const T * __restrict name
#define param_dev_rw(T, name, s)   T * __restrict name
#define param_dev_wo(T, name, s)   T * __restrict name
#define param_dev_cbuf(T, name, s) const T name

#define thread_pos_param(name)
#define thread_pos_init(name) uint2 name = dispatch_id()

#define threadgroup_barrier_all()

// __cpu_gid_x/y and dispatch_w/h are NOT volatile: their reads are via
// dispatch_id() / dispatch_size() which are __declspec(noinline) / noinline,
// ensuring a real function-call boundary exists.  The compiler cannot
// constant-fold reads across a non-inline call boundary.
//
// __cpu_format IS volatile: it is read inside pixel_load which is a deeply
// inlined helper.  Without volatile the compiler can constant-fold the read
// to 0 (the TLS initial value) or to the stale value from a previous render,
// causing pixel_load to hit the default case (returns float4(0)) or the wrong
// case (e.g. case 16 with a bpp=4 buffer → out-of-bounds read → crash).
thread_local static unsigned int __cpu_gid_x;
thread_local static unsigned int __cpu_gid_y;
thread_local static unsigned int __cpu_dispatch_w;
thread_local static unsigned int __cpu_dispatch_h;
thread_local static volatile unsigned int __cpu_format;

#define VEKL_FORMAT (__cpu_format)
#define VEKL_EXP2F exp2f
#define VEKL_BACKEND_NAME "cpu"

#include "common/types.h"
#include "common/math.h"
#include "common/pixel.h"
#include "common/logging.h"
#include "common/frame/params.h"

static noinline uint2 dispatch_id()   { return uint2(__cpu_gid_x, __cpu_gid_y); }
static noinline uint2 dispatch_size() { return uint2(__cpu_dispatch_w, __cpu_dispatch_h); }

static inline float rsqrt(float x) { return 1.0f / sqrtf(x); }

typedef void pixel;

// format parameter replaces the volatile TLS __cpu_format in the inner loop.
// The caller (image_2d) captures VEKL_FORMAT once at construction time and
// passes it explicitly, so pixel_load/store never touch TLS.  This prevents
// stale-value crashes when two concurrent renders share rayon worker threads:
// a worker that switches between a bpp=16 job and a bpp=4 job would see the
// wrong __cpu_format if the dispatch wrapper's volatile write races with the
// read inside the inlined kernel body.
inline float4 pixel_load(device const pixel *data, uint pitch_px, uint2 xy, uint format) {
	uint idx = xy.y * pitch_px + xy.x;
	switch (format) {
		case 4: {
			const uint8_t *p = reinterpret_cast<const uint8_t *>(data) + idx * 4;
			return float4(p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f, p[3] / 255.0f);
		}
		case 8: {
			const uint16_t *p = reinterpret_cast<const uint16_t *>(reinterpret_cast<const uint8_t *>(data) + idx * 8);
			return float4(p[0] / 65535.0f, p[1] / 65535.0f, p[2] / 65535.0f, p[3] / 65535.0f);
		}
		case 16: {
			const float *p = reinterpret_cast<const float *>(data) + idx * 4;
			return float4(p[0], p[1], p[2], p[3]);
		}
		default: return float4(0.0f);
	}
}

inline void pixel_store(device pixel *data, uint pitch_px, uint2 xy, float4 c, uint format) {
	uint idx = xy.y * pitch_px + xy.x;
	switch (format) {
		case 4: {
			uint8_t *p = reinterpret_cast<uint8_t *>(data) + idx * 4;
			p[0] = (uint8_t)clamp(c.x * 255.0f, 0.0f, 255.0f);
			p[1] = (uint8_t)clamp(c.y * 255.0f, 0.0f, 255.0f);
			p[2] = (uint8_t)clamp(c.z * 255.0f, 0.0f, 255.0f);
			p[3] = (uint8_t)clamp(c.w * 255.0f, 0.0f, 255.0f);
			break;
		}
		case 8: {
			uint16_t *p = reinterpret_cast<uint16_t *>(reinterpret_cast<uint8_t *>(data) + idx * 8);
			p[0] = (uint16_t)clamp(c.x * 65535.0f, 0.0f, 65535.0f);
			p[1] = (uint16_t)clamp(c.y * 65535.0f, 0.0f, 65535.0f);
			p[2] = (uint16_t)clamp(c.z * 65535.0f, 0.0f, 65535.0f);
			p[3] = (uint16_t)clamp(c.w * 65535.0f, 0.0f, 65535.0f);
			break;
		}
		case 16: {
			float *p = reinterpret_cast<float *>(data) + idx * 4;
			p[0] = c.x;
			p[1] = c.y;
			p[2] = c.z;
			p[3] = c.w;
			break;
		}
	}
}

#ifdef __cplusplus
extern "C" {
#endif
void host_log(unsigned int level, const char* message);
#ifdef __cplusplus
}
#endif

#if defined(DISABLE_LOGGING) || !defined(DEBUG)
typedef noop_logging_channel logging_channel;
#else
struct logging_channel {
	const char* channel_name;
	inline void trace(const char* message) const { __vekl_emit(VEKL_LOG_LEVEL_TRACE, message); }
	inline void debug(const char* message) const { __vekl_emit(VEKL_LOG_LEVEL_DEBUG, message); }
	inline void info(const char* message) const { __vekl_emit(VEKL_LOG_LEVEL_INFO, message); }
	inline void warn(const char* message) const { __vekl_emit(VEKL_LOG_LEVEL_WARN, message); }
	inline void error(const char* message) const { __vekl_emit(VEKL_LOG_LEVEL_ERROR, message); }
private:
	inline void __vekl_emit(unsigned int level, const char* message) const {
		char formatted[256] = {0};
		const char* channel = (channel_name != nullptr) ? channel_name : "default";
		const char* text = (message != nullptr) ? message : "";
		snprintf(formatted, sizeof(formatted), "[%s(%s)/%s] - %s",
				 VEKL_KERNEL_NAME, VEKL_BACKEND_NAME, channel, text);
		host_log(level, formatted);
	}
};
#endif

static logging_channel __vekl_default_log = {"default"};
#define log (__vekl_default_log)
