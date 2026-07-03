#include "executer.h"
#include "memory.h"
#include "cache.h"
#include "decoder.h"
#include "stack.h"
#include "core.h"

void execute(uint32_t address) {
    decode(address);
    uint64_t gp = (uint64_t)get_guest();
    uint32_t offset = cache_get_block(0)->hp;
    void(*exec)(void) = (void*)get_host() + offset;
    uint64_t* sp = get_sp();
    #if defined(__aarch64__) || defined(_M_ARM64)
    __asm__ volatile(
        "mov x21, %0\n"
        "mov x28, %1\n"
        : : "r" (gp), "r" (sp)
        : "x21", "x28"
    );
    exec();
    #endif
    print("STAT: memory: %i, cache: %i", get_hp(), cache_usage());
    cache_clear();
    success("execution");
}