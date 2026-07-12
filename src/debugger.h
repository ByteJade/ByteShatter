#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>

int debug_break(void);
void set_break_point();
void debug_enable(void);
void debug_wait(void);

#endif