#ifndef PATCHER_H
#define PATCHER_H

#include <signal.h>
#include <ucontext.h>

void print_cpu();
void print_native_cpu();
void patcher_init();

#endif