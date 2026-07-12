#ifndef CORE_H
#define CORE_H

#include <stdarg.h>

typedef enum {
    LOG_PANIC = 0,
    LOG_WARNING = 1,
    LOG_SUCCESS = 2,
    LOG_PRINT = 3
} LogLevel;

void set_log_level(char level);
void log_message(LogLevel level, const char* format, ...);

#define panic(...)   log_message(LOG_PANIC, __VA_ARGS__)
#define warning(...) log_message(LOG_WARNING, __VA_ARGS__)
#define success(...) log_message(LOG_SUCCESS, __VA_ARGS__)
#define print(...)   log_message(LOG_PRINT, __VA_ARGS__)

#endif