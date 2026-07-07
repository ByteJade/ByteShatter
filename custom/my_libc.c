#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "wrapper.h"

static void* exit_addr;

int my___libc_start_main(
    int (*main)(int, char**, char**),
    int argc,
    char** argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void* stack_end
) {
    asm volatile("str x30, %0" : "=m"(exit_addr));
    if (init) init();
    int result = main(argc, argv, NULL);
    if (fini) fini();
    return result;
}

int my_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    const char* tmp = format;
    int argc = 0;
    long argv[6] = {0};
    argv[0] = (long)format;
    char c;
    while ((c = *tmp++))
        if (c == '%') argc++;
    for (int i = 1; i <= argc && i < 5; i++) {
        argv[i] = va_arg(args, long);
    }
    va_end(args);
    if (argc < 5) goto _print;
    if (argc < 7) {
        POP8;
    }
    int result;
_print:
    asm volatile(
        "mov x0, %1\n"
        "mov x1, %2\n"
        "mov x2, %3\n"
        "mov x3, %4\n"
        "mov x4, %5\n"
        "mov x5, %6\n"
        "bl printf\n"
        "mov %w0, x0\n"
        : "=r" (result)
        : "r"(argv[0]), "r"(argv[1]), "r"(argv[2]), "r"(argv[3]),
            "r"(argv[4]), "r"(argv[5])
    );
    return result;
}
void my_exit(int status) {
    asm volatile("br %0" :: "r"(exit_addr));
    __builtin_unreachable();
}
WRAP_NORETURN(int, puts, const char* s);
WRAP_NORETURN(void*, malloc, size_t size);
WRAP_NORETURN(void, free, void* ptr);
WRAP_NORETURN(size_t, strlen, const char* s);
WRAP_NORETURN(int, strcmp, const char* s1, const char* s2);
WRAP_NORETURN(char*, strstr, const char* haystack, const char* needle);