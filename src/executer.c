#include "executer.h"
#include "stack.h"

void execute(uint64_t address) {
    void* sp = get_sp();
    #if defined(__aarch64__) || defined(_M_ARM64)
    asm volatile(
        "mov x28, %0\n"
        "mov sp, %0\n"
        : : "r" (sp)
        :
    );
    #endif
    ((void(*)(void))address)();
    __builtin_unreachable();
}