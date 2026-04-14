#pragma once

#ifndef VEKL_KERNEL_NAME
#define VEKL_KERNEL_NAME "unknown"
#endif

#ifndef VEKL_BACKEND_NAME
#define VEKL_BACKEND_NAME "cuda"
#endif

// __vekl_bind_log_buffer is always defined because prepare_cuda_source()
// injects a call to it into every kernel prologue regardless of logging state.
#if defined(DISABLE_LOGGING) || !defined(DEBUG)

typedef noop_logging_channel logging_channel;

inline __device__ void __vekl_bind_log_buffer(VeklLogBuffer* buffer) {
	(void)buffer; // no-op when logging is disabled
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
// Use a constant string literal for the noop channel — avoids a .global device variable
#define logging (noop_logging_channel{"default"})
#else
static __device__ logging_channel __vekl_default_log = {"default"};
#define logging (__vekl_default_log)
#endif
