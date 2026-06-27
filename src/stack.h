#ifndef STACK_H
#define STACK_H

#include <elf.h>

void stack_init();
void stack_fini();

void push_argv(const char* arg);
void push_envp(const char* env);

void set_envp(const char** envp);
void set_auxv(Elf64_auxv_t* auxv, int auxc);

void* get_sp();

#endif