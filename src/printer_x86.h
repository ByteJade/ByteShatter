#ifndef PRINTER_X86_64_H
#define PRINTER_X86_64_H

#include "decoder.h"

extern const char* instr_types[];
extern const char* regs64[];
extern const char* regs32[];
extern const char* regs16[];
extern const char* regs8[];

void sprint_x86_64(Instruction* buf, char* out);

#endif