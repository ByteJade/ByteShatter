#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>
#include "elf_loader.h"

int debug_break(void);
void debug_check_break(void);

void debug_enable(Elf* elf);
int debug_enabled(void);
void debug_wait(void);

#endif