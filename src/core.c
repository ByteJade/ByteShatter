#include "core.h"
#include <stdio.h>
#include <stdlib.h>

static const char* const prefixes[] = {
    "\033[1;31mPANIC::",
    "\033[1;33mWARN::",
    "\033[1;32mSuccess: ",
    ""
};

void log_message(LogLevel level, const char* format, ...) {
    if (prefixes[level][0]) printf("%s", prefixes[level]);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\033[0m\n");
    
    if (level == LOG_PANIC) exit(0);
}