#include "stack.h"
#include "core.h"
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>

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

void push_argv(const char* arg) {
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

void* get_sp() {
    if ((uint64_t)sp % 16 != 0) {
        *(--sp) = 0;
    }
    *(--sp) = argc;
    return sp;
}