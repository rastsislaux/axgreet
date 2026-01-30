#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#if DEBUG_LOG_ENABLED
#define DEBUG_LOG(fmt, ...) fprintf(stderr, "[DEBUG] " fmt, ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...)
#endif

#if TRACE_LOG_ENABLED
#define TRACE_LOG(fmt, ...) fprintf(stderr, "[TRACE] " fmt, ##__VA_ARGS__)
#else
#define TRACE_LOG(fmt, ...)
#endif

#endif