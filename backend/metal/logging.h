#pragma once

#ifndef VEKL_KERNEL_NAME
#define VEKL_KERNEL_NAME "unknown"
#endif

#ifndef VEKL_BACKEND_NAME
#define VEKL_BACKEND_NAME "metal"
#endif

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

constant logging_channel __vekl_default_log {"default"};
#define logging (__vekl_default_log)
