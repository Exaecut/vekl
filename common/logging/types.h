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

// Cross-platform string builder for log messages.
// Works on all backends including CUDA/Metal where snprintf is unavailable.
// Usage: LogFormat().str("w=").u(width).str(" uv=(").f(uv.x).c(',').f(uv.y).c(')')
//        then pass fmt.c_str() to log.info() etc.
struct LogFormat {
	char buf[VEKL_LOG_MESSAGE_CAPACITY];
	unsigned int pos;

	forceinline LogFormat() : pos(0) { buf[0] = '\0'; }

	forceinline const char* c_str() const { return buf; }

	forceinline LogFormat& str(const char* s) {
		if (s == nullptr) return *this;
		while (*s && pos < VEKL_LOG_MESSAGE_CAPACITY - 1u) {
			buf[pos++] = *s++;
		}
		buf[pos] = '\0';
		return *this;
	}

	forceinline LogFormat& c(char ch) {
		if (pos < VEKL_LOG_MESSAGE_CAPACITY - 1u) {
			buf[pos++] = ch;
			buf[pos] = '\0';
		}
		return *this;
	}

	forceinline LogFormat& u(unsigned int v) {
		if (v == 0u) { c('0'); return *this; }
		char digits[12];
		int i = 0;
		while (v > 0u && i < 12) {
			digits[i++] = '0' + (char)(v % 10u);
			v /= 10u;
		}
		for (int j = i - 1; j >= 0; j--) {
			c(digits[j]);
		}
		return *this;
	}

	forceinline LogFormat& d(int v) {
		if (v < 0) {
			c('-');
			u((unsigned int)(-(v + 1)) + 1u);
			return *this;
		}
		u((unsigned int)v);
		return *this;
	}

	forceinline LogFormat& f(float v) {
		if (v != v) { str("nan"); return *this; }
		if (v > 1e30f) { str("inf"); return *this; }
		if (v < -1e30f) { str("-inf"); return *this; }
		if (v < 0.0f) { c('-'); v = -v; }
		unsigned int intPart = (unsigned int)v;
		u(intPart);
		c('.');
		float frac = v - (float)intPart;
		for (int i = 0; i < 4; i++) {
			frac *= 10.0f;
			unsigned int digit = (unsigned int)frac;
			if (digit > 9u) digit = 9u;
			c('0' + (char)digit);
			frac -= (float)digit;
		}
		return *this;
	}
};

struct noop_logging_channel {
	const char* channel_name;
	inline void trace(const char*) const {}
	inline void debug(const char*) const {}
	inline void info(const char*) const {}
	inline void warn(const char*) const {}
	inline void error(const char*) const {}
};
