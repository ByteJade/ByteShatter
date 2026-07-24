#include "armdef.h"

const char* types[] = {
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
    "pxor", "adds", "muls",
    "cvtss2sd", "cvtsd2ss",
    "subs", "divs",
    "comis", "movs",
    "movq", "movapd",
    "cvtsd2si", "cvtsi2s",
};
const char* regs[] = {
    "ax", "cx", "dx", "bx",
    "sp", "bp", "si", "di",
    "8", "9", "10", "11",
    "12", "13", "14", "15",
};

uint8_t x64_regs[] = {
    9,3,2,14,
    28,29,1,0,
    4,5,10,11,
    16,17,18,19,
    12, 13, 30,
    6, 7, 31
};