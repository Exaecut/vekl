#pragma once

#ifndef VEKL_KERNEL_NAME
#define VEKL_KERNEL_NAME "unknown"
#endif

#ifndef VEKL_BACKEND_NAME
#define VEKL_BACKEND_NAME "cpu"
#endif

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
void host_log(unsigned int level, const char* message);
#ifdef __cplusplus
}
#endif

// logging_channel (debug only) - calls host_log directly.
// Disabled when DISABLE_LOGGING is defined or DEBUG is not set.
#if defined(DISABLE_LOGGING) || !defined(DEBUG)
typedef noop_logging_channel logging_channel;
#else

// Aggregate struct — no user-defined constructor.
// Custom channels: logging_channel net_log = {"network"};  net_log.info("...");
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

#endif // DISABLE_LOGGING || !DEBUG

// Default logging instance
//   logging.info("...");
//
// Custom named channels:
//   logging_channel blur_pass_log = {"blur"};
//   blur_pass_log.warn("...");
static logging_channel __vekl_default_log = {"default"};
#define logging (__vekl_default_log)
