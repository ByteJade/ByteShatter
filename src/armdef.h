#ifndef DEFS_H
#define DEFS_H

#include "stdint.h"

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
    
    PXOR, ADDS, MULS,
    CVTSS2SD, CVTSD2SS,
    SUBS, DIVS,
    COMIS, MOVS,
    MOVQ, MOVAPD,
    CVTSD2SI, CVTSI2S,

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
extern const char* types[];
extern const char* regs[];
extern uint8_t x64_regs[];

#endif
