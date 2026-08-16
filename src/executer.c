#include "executer.h"
#include "memory.h"
#include "cache.h"
#include "decoder.h"
#include "stack.h"

void execute(uint64_t address) {
    decode(address);
    uint32_t offset = cache_get_block(0)->hp;
    void(*exec)(void) = (void(*)(void))(get_host() + offset);
    uint64_t* sp = get_sp();
    #if defined(__aarch64__) || defined(_M_ARM64)
    __asm__ volatile(
        "mov sp, %0\n"
        : : "r" (sp)
        :
    );
    #endif
    exec();
    __builtin_unreachable();
}