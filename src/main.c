#include "core.h"
#include "memory.h"
#include "cache.h"
#include "dlmanager.h"
#include "decoder.h"
#include "patcher.h"
#include <stdint.h>

void execute(int block) {
    CacheUnit* unit = cache_get_block(block);
    void* code = get_host() + unit->hp;
    uint32_t size = unit->offsets[unit->offsetssz-1].hoff+4;
    uint64_t gp = (uint64_t)get_guest();
    void* end = code + size;
    __builtin___clear_cache(code, end);
    void(*exec)(void) = code;
    #if defined(__aarch64__) || defined(_M_ARM64)
    __asm__ volatile(
        "mov x21, %0\n"
        : : "r" (gp)
        : "x21", "memory"
    );
    exec();
    #endif
    success("execution");
}
int main(int argc, char** argv) {
    patcher_init();
    cahce_init();
    ExeMeta* exe = load_object(argv[argc-1]);
    
    decode(exe->init);
    success("decode init");
    execute(0);
    print("STAT: memory: %i, cache: %i", get_hp(), cache_usage());
    cache_print();
    cache_clear();
    decode(exe->elf->header.e_entry);
    success("decode _start");
    execute(0);
    print("STAT: memory: %i, cache: %i", get_hp(), cache_usage());
    cache_print();
    cache_clear();

    loader_close_elf(exe);
    loader_close_exe(exe);
    cahce_fini();
    memory_fini();
    success("anythink");
}