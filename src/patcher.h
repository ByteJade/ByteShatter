#ifndef PATCHER_H
#define PATCHER_H

#include <signal.h>
#include <ucontext.h>

void print_cpu(void);
void print_native_cpu(void);
void patcher_init(void);

#endif