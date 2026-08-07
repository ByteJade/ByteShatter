#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>

enum Prefixes {
    FS = 0x64,
    GS = 0x65,
    OS = 0x66,
    //LOCK = 0xF0,
    REPN = 0xF2,
    REPE = 0xF3,
};
enum OpType {
    NONE = 0,
    REG = 1<<0,
    MEM = 1<<1,
    IDX = 1<<2,
    IMM = 1<<3,
    XMM = 1<<4,
};
enum Registers {
    RAX,RCX,RDX,RBX,
    RSP,RBP,RSI,RDI,
    R8, R9, R10,R11,
    R12,R13,R14,R15,
// tech
    SC1,SC2,X30,
    AR7,AR8,XZR
};
enum InstrTypes {
    ADD, OR, ADC, SBB,
    AND, SUB, XOR, CMP,

    ROL, ROR, RCL, RCR,
    SHL, SHR, SAL, SAR,

    PUSH, POP, MOVSX, MOV,

    JO, JNO, JB, JAE,
    JE, JNE, JBE, JA,
    JS, JNS, JP, JPO,
    JL, JGE, JLE, JG,

    TEST, LEA, NOP, EBR,
    RET, LEAVE, CALL, JMP,

    PXOR, ADDS, MULS,
    CVTSS2SD, CVTSD2SS,
    SUBS, DIVS,
    COMIS, MOVS,
    MOVQ, MOVAPD,
    CVTSD2SI, CVTSI2S,
    CMOVA, MOVZX, IDIV,
    CLTQ, CLTD
};
typedef struct {
    uint8_t type;
    uint8_t reg;
    uint8_t idx;
    uint8_t scale;
    int64_t imm;
} Operand;
typedef struct {
    uint8_t type;
    uint8_t size;
    uint8_t prefix;
    uint8_t reverse;
    
    Operand a;
    Operand b;
} Instruction;

int64_t fetch_imm8(void);
int64_t fetch_imm32(void);
void decode_r_rm(Instruction* buf);
void decode_rm_r(Instruction* buf);
void decode_0F(Instruction* buf);
int decode_instr(Instruction* buf);
void decode(uint32_t gp);

#endif