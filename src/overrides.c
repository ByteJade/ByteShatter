#include "overrides.h"
#include <string.h>

int syscall_override[] = {
    [0] = 63,
    [1] = 64,
    [9] = 222,
    [11] = 215,
};

void my_syscall(long num, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
    long ret;
    num = syscall_override[num];
    
    #ifdef __aarch64__
    asm volatile (
        "mov x8, %1\n"
        "mov x0, %2\n"
        "mov x1, %3\n"
        "mov x2, %4\n"
        "mov x3, %5\n"
        "mov x4, %6\n"
        "mov x5, %7\n"
        "svc #0\n"
        "mov x9, x0\n"
        "ret x30\n"
        : : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
        : "memory"
    );
    #endif
}

void* sym_override(const char* symname) {
    if (strcmp(symname, "syscall") == 0) {
        return my_syscall;
    }
    return NULL;
}