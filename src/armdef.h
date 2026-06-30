#ifndef DEFS_H
#define DEFS_H

#include "stdint.h"

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
    uint8_t opcount;

    Operand op0;
    Operand op1;
} X64_instruction;

static const char* types[] = {
    "mov", "add", "sub", "test",
    "je", "call", "ret", "xor",
    "pop", "push", "and", "lea",
    "jmp", "cmp", "endbr64", "jl",
    "leave", "cltq"
};
static const char* regs[] = {
    "ax", "cx", "dx", "bx",
    "sp", "bp", "si", "di",
    "8", "9", "10", "11",
    "12", "13", "14", "15",
};
typedef enum {
    REG = 0,
    MEM = 1,
    IDX = 2,
    IMM = 4,
} OpTypes;

typedef enum {
    MOV, ADD, SUB, TST,
    JE, CALL, RET, XOR,
    POP, PUSH, AND, LEA,
    JMP, CMP, EBR, JL,
    LEAVE, CLTQ,
} InstrTypes;

enum mapped_registers {
    RAX,RCX,RDX,RBX,
    RSP,RBP,RSI,RDI,
    R8, R9, R10,R11,
    R12,R13,R14,R15,
// tech
    SC1,SC2,RIP,X30,
    AR7,AR8,XZR
};
static uint8_t x64_regs[] = {
    0,3,2,14,
    28,29,1,9,
    4,5,10,11,
    16,17,18,19,
    12, 13, 21, 30,
    6, 7, 31
};

#endif
