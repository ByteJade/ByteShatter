#ifndef PATCHER_H
#define PATCHER_H

#include <signal.h>
#include <ucontext.h>
#include <stdint.h>

uint64_t get_reg(const char* name);
void print_cpu(void);
void print_native_cpu(void);
void patcher_init(void);

#endif