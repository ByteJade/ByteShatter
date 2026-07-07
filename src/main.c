#include "core.h"
#include "memory.h"
#include "cache.h"
#include "dlmanager.h"
#include "decoder.h"
#include "patcher.h"
#include "stack.h"
#include "executer.h"
#include <stdint.h>
#include <sys/auxv.h>
#include <unistd.h>

void init_stack(ExeMeta* exe, int argc, char** argv) {
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
        //{AT_SYSINFO,   0}, Android doesn't have this?
        {AT_NULL,      0}
    };
    set_auxv(auxv, 19);
    push_argv(0);
    for (int i = argc-1; i > 0; i--) {
        push_argv(argv[i]);
    }
    push_argc();
}
int main(int argc, char** argv, const char** envp) {
    patcher_init();
    cahce_init();
    stack_init();
    set_envp(envp);
    ExeMeta* exe = load_object(argv[1]);
    init_stack(exe, argc, argv);
    
    execute(exe->init);
    if (exe->init_array) {
        size_t count = exe->init_arraysz / sizeof(Elf64_Addr);
        uint64_t* init_funcs = (uint64_t*)(exe->base + exe->init_array);
        
        for (size_t i = 0; i < count; i++) {
            if (init_funcs[i]) {
                uint64_t pos = init_funcs[i] - (uint64_t)exe->base;
                print("Calling INIT_ARRAY[%zu] at %lx\n", i, pos);
                execute(pos);
            }
        }
    }
    execute(exe->elf->header.e_entry);

    loader_close_elf(exe);
    loader_close_exe(exe);
    cahce_fini();
    memory_fini();
    stack_fini();
    success("anythink");
}