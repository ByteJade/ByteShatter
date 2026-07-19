#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "armdef.h"

enum Prefixes {
    TLS = 0x64,
    P66 = 0x66,
    REP = 0xf3,
    REPN = 0xf2,
};
typedef struct {
    uint8_t type;
    uint8_t reg;
    uint8_t idx;
    uint8_t scale;
    int64_t imm;
} Operand;
typedef struct {
    uint8_t size;
    uint8_t type;
    uint8_t reverse;
    uint8_t opcount;
    uint8_t prefix;

    Operand op0;
    Operand op1;
} X64_instruction;

int64_t fetch_imm8(void);
int64_t fetch_imm32(void);
void decode_rm(Operand* op, uint8_t modrm);
void decode_regrm(X64_instruction* buf);
void decode_0F(X64_instruction* buf);
void sprint_instr(char* out, X64_instruction* buf);
int decode_instr(X64_instruction* buf);
void decode(uint32_t gp);

#endif