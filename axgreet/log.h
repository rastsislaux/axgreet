#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define ERROR_LOG(fmt, ...) fprintf(stderr, "[ERROR] [" __FILE__ ":%d] " fmt, __LINE__, ##__VA_ARGS__)
#define DEBUG_LOG(fmt, ...) fprintf(stderr, "[DEBUG] [" __FILE__ ":%d] " fmt, __LINE__, ##__VA_ARGS__)
#define TRACE_LOG(fmt, ...) fprintf(stderr, "[TRACE] [" __FILE__ ":%d] " fmt, __LINE__, ##__VA_ARGS__)

#endif
