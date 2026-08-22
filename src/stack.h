#ifndef STACK_H
#define STACK_H

#include <elf.h>
#include "elf_manager.h"

#define STACK_SIZE 2*1024*1024

void stack_init(void);
void stack_fini(void);

void push_argc(void);
void push_arg(const char* arg);
void push_envp(const char* env);

void set_envp(const char** envp);
void set_auxv(Elf64_auxv_t* auxv, int auxc);

void finish_stack(Elf* elf);
void* get_sp(void);

#endif