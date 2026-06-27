#include "core.h"
#include "memory.h"
#include "cache.h"
#include "dlmanager.h"
#include "decoder.h"
#include "patcher.h"
#include "stack.h"
#include <stdint.h>
#include <sys/auxv.h>
#include <unistd.h>

void execute(int block) {
    cache_flush(block);
    uint64_t gp = (uint64_t)get_guest();
    uint32_t offset = cache_get_block(block)->hp;
    void(*exec)(void) = (void*)get_host() + offset;
    void* sp = get_sp();
    #if defined(__aarch64__) || defined(_M_ARM64)
    __asm__ volatile(
        "mov x21, %0\n"
        "mov x28, %1\n"
        : : "r" (gp), "r" (sp)
        : "x21", "memory"
    );
    exec();
    #endif
    success("execution");
}
void execute_stack(int block) {
    cache_flush(block);
    uint64_t gp = (uint64_t)get_guest();
    uint32_t offset = cache_get_block(block)->hp;
    void* exec = get_host() + offset;
    /*#if defined(__aarch64__) || defined(_M_ARM64)
    __asm__ volatile(
        "mov x21, %0\n"
        "mov sp, %1\n"
        "br %2"
        : : "r" (gp), "r" (sp), "r" (exec)
        : "x21", "sp", "memory"
    );
    exec();
    #endif*/
    success("execution");
}
void init_stack(ExeMeta* exe, const char* name) {
    Elf64_auxv_t auxv[] = {
        {AT_SYSINFO_EHDR, 0},
        {AT_MINSIGSTKSZ, 0x5f0},
        {AT_HWCAP,     0x26},
        {AT_PAGESZ,    sysconf(_SC_PAGESIZE)},
        {AT_CLKTCK,    sysconf(_SC_CLK_TCK)},
        {AT_PHDR,      (uint64_t)exe->base + exe->elf->header.e_phoff},
        {AT_PHENT,     exe->elf->header.e_phentsize},
        {AT_PHNUM,     exe->elf->header.e_phnum},
        {AT_BASE,      0},
        {AT_FLAGS,     0},
        {AT_ENTRY,     (uint64_t)exe->base + exe->elf->header.e_entry},
        {AT_UID,       getuid()},
        {AT_EUID,      geteuid()},
        {AT_GID,       getgid()},
        {AT_EGID,      getegid()},
        {AT_SECURE,    0},
        {AT_RANDOM, getauxval(AT_RANDOM)},
        {AT_HWCAP2,    2},
        //{AT_SYSINFO,   0},
        {AT_NULL,      0}
    };
    set_auxv(auxv, 19);
    push_argv(0);
    push_argv(name);
}
int main(int argc, char** argv, const char** envp) {
    patcher_init();
    cahce_init();
    stack_init();
    set_envp(envp);
    ExeMeta* exe = load_object(argv[argc-1]);
    init_stack(exe, argv[argc-1]);
    decode(exe->init);
    success("decode init");
    execute(0);
    cache_print();
    print("STAT: memory: %i, cache: %i", get_hp(), cache_usage());
    cache_clear();
    decode(exe->elf->header.e_entry);
    success("decode _start");
    cache_print();
    execute(0);
    print("STAT: memory: %i, cache: %i", get_hp(), cache_usage());
    cache_clear();

    loader_close_elf(exe);
    loader_close_exe(exe);
    cahce_fini();
    memory_fini();
    stack_fini();
    success("anythink");
}