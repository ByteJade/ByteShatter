#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>

int debug_break(void);
uint32_t debug_block();
uint64_t debug_pc();
void set_bp(uint32_t* instr);

void debug_enable(void);
int debug_enabled(void);
void debug_wait(void);

#endif