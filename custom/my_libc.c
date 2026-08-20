#include "wrapper.h"
#include <sys/unistd.h>

// just call main and return
__asm__(
    ".global my___libc_start_main\n"
    ".type my___libc_start_main, @function\n"
    "my___libc_start_main:\n"
    "mov sp, x28\n"
    "mov x7, x0\n"
    "mov x0, x1\n"
    "mov x1, x2\n"
    "bl x7\n"
    "mov x0, #0\n"
    "b _exit\n"
);
WRAP_FUNC(__isoc23_strtol)
WRAP_FUNC(__isoc23_sscanf)
WRAP_FUNC(__errno_location)
WRAP_BIG_FUNC(printf)
WRAP_BIG_FUNC(vsnprintf)
WRAP_BIG_FUNC(fprintf)
WRAP_BIG_FUNC(sprintf)
WRAP_FUNC_VOID(exit)
WRAP_FUNC_VOID(__stack_chk_fail)
WRAP_FUNC(puts)
WRAP_FUNC(malloc)
WRAP_FUNC(memcpy)
WRAP_FUNC(memset)
WRAP_FUNC(memcmp)
WRAP_FUNC(memchr)
WRAP_FUNC(memmove)
WRAP_FUNC(realloc)
WRAP_FUNC(calloc)
WRAP_FUNC_VOID(free)
WRAP_FUNC(strlen)
WRAP_FUNC(strstr)
WRAP_FUNC(strcmp)
WRAP_FUNC(strncmp)
WRAP_FUNC(strncpy)
WRAP_FUNC(strchr)
WRAP_FUNC(strcat)
WRAP_FUNC(strcspn)
WRAP_FUNC(strerror)
WRAP_FUNC(strrchr)
WRAP_FUNC(fflush)
WRAP_FUNC(strtod)
WRAP_FUNC(nanosleep)
WRAP_FUNC(gettimeofday)
WRAP_FUNC(localtime)
WRAP_FUNC(strftime)
WRAP_FUNC(gmtime)
WRAP_FUNC(mktime)
WRAP_FUNC(utime)
WRAP_FUNC(time)
WRAP_FUNC(setlocale)
WRAP_FUNC(ftell)
WRAP_FUNC(fputs)
WRAP_FUNC(qsort)
WRAP_FUNC(rand)
WRAP_FUNC_VOID(srand)

WRAP_FUNC(feof)
WRAP_FUNC(fseek)
WRAP_FUNC(fseeko)
WRAP_FUNC(fseeko64)
WRAP_FUNC(ftello64)
WRAP_FUNC(remove)
WRAP_FUNC(rename)
WRAP_FUNC(opendir)
WRAP_FUNC(closedir)
WRAP_FUNC(readdir)
WRAP_FUNC(mkdir)
WRAP_FUNC(fopen)
WRAP_FUNC(fopen64)
WRAP_FUNC(fwrite)
WRAP_FUNC(fread)
WRAP_FUNC(fclose)
WRAP_FUNC(access)

WRAP_FUNC(setenv)
WRAP_FUNC(getenv)
WRAP_FUNC(unsetenv)
WRAP_FUNC(longjmp)
WRAP_FUNC(setjmp)

WRAP_FUNC(getopt)
WRAP_FUNC(getopt_long)

WRAP_FUNC(dlopen)
WRAP_FUNC(dlclose)
WRAP_FUNC(dlsym)
WRAP_FUNC(dlerror)
WRAP_FUNC(sysconf)

WRAP_FUNC(isatty)
WRAP_FUNC(getrlimit)
WRAP_FUNC(getrusage)
WRAP_FUNC(setrlimit)

WRAP_FUNC_VOID(__cxa_finalize)