#include "stack.h"
#include "core.h"
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <sys/auxv.h>
#include <unistd.h>

#define STACK_SIZE 2*1024*1024

void* stack = NULL;
char* dp = NULL;
uint64_t* sp = NULL;
int argc = 0;

void stack_init() {
    stack = mmap(
        NULL, STACK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_STACK | MAP_ANON | MAP_PRIVATE,
        -1, 0
    );
    if (stack == MAP_FAILED) {
        panic("STACK::MAP_FAIL");
    }
    dp = stack;
    sp = stack + STACK_SIZE;
}
void stack_fini() {
    munmap(stack, STACK_SIZE);
}

void push_argc() {
    *(--sp) = argc;
}
void push_arg(const char* arg) {
    if (arg) {
        *(--sp) = (uint64_t)dp;
        strcpy(dp, arg);
        dp += strlen(arg) + 1;
        argc++;
    } else {
        *(--sp) = 0;
    }
}
void push_envp(const char* env) {
    if (env) {
        *(--sp) = (uint64_t)dp;
        strcpy(dp, env);
        dp += strlen(env) + 1;
    } else {
        *(--sp) = 0;
    }
}
void set_auxv(Elf64_auxv_t* auxv, int auxc) {
    for (int i = auxc - 1; i >= 0; i--) {
        *(--sp) = auxv[i].a_un.a_val;
        *(--sp) = auxv[i].a_type;
    }
}
void set_envp(const char** envp) {
    push_envp(0);
    int envc = 0;
    while (envp[envc]) {
        push_envp(envp[envc]);
        envc++;
    }
}
void finish_stack(ExeMeta* exe) {
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
}

void* get_sp() {
    return sp;
}