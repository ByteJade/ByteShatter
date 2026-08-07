#include "wrapper.h"
#include <stdlib.h>

// just call main and return
int my___libc_start_main(
    int (*main)(int, char **, char **),
    int argc, char** argv,
    void (*init)(void), void (*fini)(void),
    void (*rtld_fini)(void), void* stack_end)
{
    if (init) init();
    int out = main(argc, argv, NULL);
    if (fini) fini();
    exit(0);
}
void my___isoc23_strtol() {
    asm volatile(
        "mov x27, x30\n"
        "bl strtol\n"
        "mov x30, x27\n"
        "mov x9, x0\n"
    );
}
void my___isoc23_sscanf() {
    asm volatile(
        "mov x27, x30\n"
        "bl sscanf\n"
        "mov x30, x27\n"
        "mov x9, x0\n"
    );
}

WRAP_BIG_FUNC(printf)
WRAP_BIG_FUNC(vsnprintf)
WRAP_BIG_FUNC(fprintf)
WRAP_BIG_FUNC(sprintf)
WRAP_FUNC_VOID(exit)
WRAP_FUNC(puts)
WRAP_FUNC(malloc)
WRAP_FUNC(memcpy)
WRAP_FUNC(memset)
WRAP_FUNC(memcmp)
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
WRAP_FUNC(dcgettext)
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