#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "armdef.h"

void sprint_instr(char* out, X64_instruction* buf);
int decode_instr(X64_instruction* buf);
void decode(uint32_t gp);

#endif