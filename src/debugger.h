#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>

int debug_break(void);
void set_break_point(uint32_t pc);
void debug_enable(void);
void debug_wait(void);

#endif