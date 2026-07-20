#ifndef DEFS_H
#define DEFS_H

#include "stdint.h"

static const char* types[] = {
    // std
    "mov", "add", "sub", "test",
    "call", "ret", "xor",
    "pop", "push", "and", "lea",
    "jmp", "cmp", "endbr64", 
    "leave", "cltq", "cltd",
    "nop", "shl", "shr", "sar",
    "movzx", "movslq", 
    "idiv",
    // jumps
    "jb", "jae", "je",
    "jne", "jbe", "ja",
    "js", "jns", "jp",
    "jnp","jl", "jge",
    "jle","jg",
    // avx
    "cvtss2sd", "cvtsd2ss",
    "pxor", "comis", "movq", "movapd",
    "cvtsd2si", "cvtsi2s",
    "movs", "divs", "muls", "adds", "subs",

};
static const char* regs[] = {
    "ax", "cx", "dx", "bx",
    "sp", "bp", "si", "di",
    "8", "9", "10", "11",
    "12", "13", "14", "15",
};
typedef enum {
    REG = 1<<0,
    MEM = 1<<2,
    IDX = 1<<3,
    IMM = 1<<4,
    XMM = 1<<5,
} OpTypes;

typedef enum {
    MOV, ADD, SUB, TST,
    CALL, RET, XOR,
    POP, PUSH, AND, LEA,
    JMP, CMP, EBR,
    LEAVE, CLTQ, CLTD,
    NOP, SHL, SHR, SAR,
    MOVZX, MOVSLQ,
    IDIV,

    JB, JAE, JE, JNE,
    JBE, JA, JS, JNS,
    JP, JNP, JL, JGE,
    JLE, JG,
    
    CVTSS2SD, CVTSD2SS, PXOR, COMIS,
    MOVQ, MOVAPD, CVTSD2SI, CVTSI2S,
    MOVS, DIVS, MULS, ADDS, SUBS,

} InstrTypes;

enum mapped_registers {
    RAX,RCX,RDX,RBX,
    RSP,RBP,RSI,RDI,
    R8, R9, R10,R11,
    R12,R13,R14,R15,
// tech
    SC1,SC2,X30,
    AR7,AR8,XZR
};
static uint8_t x64_regs[] = {
    9,3,2,14,
    28,29,1,0,
    4,5,10,11,
    16,17,18,19,
    12, 13, 30,
    6, 7, 31
};

#endif
