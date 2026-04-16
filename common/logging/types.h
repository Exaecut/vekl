#pragma once

#ifndef VEKL_LOG_CHANNEL_CAPACITY
#define VEKL_LOG_CHANNEL_CAPACITY 32u
#endif

#ifndef VEKL_LOG_MESSAGE_CAPACITY
#define VEKL_LOG_MESSAGE_CAPACITY 160u
#endif

#ifndef VEKL_LOG_CAPACITY
#define VEKL_LOG_CAPACITY 1024u
#endif

#define VEKL_LOG_LEVEL_TRACE  0u
#define VEKL_LOG_LEVEL_DEBUG  1u
#define VEKL_LOG_LEVEL_INFO   2u
#define VEKL_LOG_LEVEL_WARN   3u
#define VEKL_LOG_LEVEL_ERROR  4u

struct VeklLogEntry {
	unsigned int  committed_sequence;
	unsigned int  level;
	unsigned int  channel_len;
	unsigned int  message_len;
	unsigned char channel[VEKL_LOG_CHANNEL_CAPACITY];
	unsigned char message[VEKL_LOG_MESSAGE_CAPACITY];
};

struct VeklLogBuffer {
	unsigned int write_index;
	unsigned int capacity;
	unsigned int overrun_count;
	unsigned int reserved;
	VeklLogEntry entries[VEKL_LOG_CAPACITY];
};

struct noop_logging_channel {
	const char* channel_name;
	inline void trace(const char*) const {}
	inline void debug(const char*) const {}
	inline void info(const char*) const {}
	inline void warn(const char*) const {}
	inline void error(const char*) const {}
};
