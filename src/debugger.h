#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>

int debug_break(void);
void debug_check_break();

void debug_enable(void);
int debug_enabled(void);
void debug_wait(void);

#endif