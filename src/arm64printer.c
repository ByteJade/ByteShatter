#include "arm64printer.h"
#include <stdint.h>
#include <stdio.h>
int comp(const char* cond, uint64_t opcode) {
    uint32_t mask = 0;
    uint32_t check = 0;
    for (int i = 0; i < 32; i++) {
        char c = *cond++;
        if (c == '-') continue;
        uint32_t addent = 1 << (31 - i);
        mask |= addent;
        if (c == '1') check |= addent;
    }
    return ((opcode&mask) == check);
}
void sprint_x_x_x(char* out, char* name, uint32_t buf) {
    uint8_t rd = (buf >> 0) & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    uint8_t rm = (buf >> 16) & 0x1F;
    uint8_t shift = (buf >> 10) & 0x3;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    char sign = ' ';
    if (buf&(1<<29)) sign = 'S';
    sprintf(out, "%s%c %c%i, %c%i, %c%i",
        name, sign, size, rd, size, rn, size, rm);
    if (shift) out += sprintf(out, "{%i}", shift);
}
void sprint_x_x_imm(char* out, char* name, uint32_t buf) {
    uint8_t rd = buf & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    uint16_t imm = (buf >> 10) & 0xFFF;
    uint8_t shift = (buf >> 22) & 0x3;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    char sign = ' ';
    if (buf&(1<<29)) sign = 'S';
    out += sprintf(out, "%s%c %c%i, %c%i, #%x",
        name, sign,  size, rd, size, rn, imm);
    if (shift) out += sprintf(out, "{%i}", shift);
}
void sprint_x_mem(char* out, char* name, uint32_t buf) {
    uint8_t rd = buf & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    uint16_t imm = (buf >> 12) & 0x1FF;
    int W = (buf >> 10) & 1;  // Write-back?
    int P = (buf >> 11) & 1;  // Pre-indexed?
    char sign = '+';
    if (imm & 0x100) {
        imm |= 0xFE00;
        sign = '-';
    }
    char size = 'W';
    if (buf&(1<<30)) size = 'X';
    out += sprintf(out, "%s %c%i, [X%i",
        name, size, rd, rn);
    if (P) {
        // Pre-indexed: [Xn, #offset]
        if (W && imm) {
            out += sprintf(out, ", #%c%x]!", sign, imm);
        } else {
            out += sprintf(out, "]");
        }
    } else {
        // Post-indexed: [Xn], #offset
        out += sprintf(out, "]");
        if (W && imm) {
            out += sprintf(out, ", #%c%x", sign, imm);
        }
    }
}
void sprint_x_imm(char** out, uint32_t buf) {
    char* ptr = *out;
    uint8_t rd = buf & 0x1F;
    uint16_t imm = (buf >> 5) & 0xFFFF;
    uint8_t shift = (buf >> 21) & 0x3;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    ptr += sprintf(ptr, "%c%i, #%i", size, rd, imm);
    if (shift) ptr += sprintf(ptr, "{%i}", shift);
    *out = ptr;
}
void sprint_arm(char* out, uint32_t buf) {
    if (comp("-0-01011------------------------", buf))
        {sprint_x_x_x(out, "add", buf); return;}
    if (comp("-1-01011------------------------", buf))
        {sprint_x_x_x(out, "sub", buf); return;}
    if (comp("-0-10001------------------------", buf))
        {sprint_x_x_imm(out, "add", buf); return;}
    if (comp("-1-10001------------------------", buf))
        {sprint_x_x_imm(out, "sub", buf); return;}
    if (comp("-1-01010------------------------", buf))
        {sprint_x_x_x(out, "eor", buf); return;}
    if (comp("1-11100-01----------------------", buf))
        {sprint_x_mem(out, "ldr", buf); return;}
    if (comp("1-11100-00----------------------", buf))
        {sprint_x_mem(out, "str", buf); return;}
    if (comp("11010100001---------------------", buf))
        {sprintf(out, "brk %x", (buf>>5)&0xFFFF); return;}
    if (comp("1101011001011111----------------", buf))
        {sprintf(out, "ret X%i", (buf>>5)&0x1F); return;}
    if (comp("1101011000111111000000-----00000", buf))
        {sprintf(out, "blr X%i", (buf>>5)&0x1F); return;}
    if (comp("0--10000------------------------", buf))
        {sprintf(out, "adr X%i, #%x", buf&0x1F, ((buf >> 29) & 0x3) | (((buf >> 5) & 0x7FFFF) << 2)); return;}
    if (comp("1--10000------------------------", buf))
        {sprintf(out, "adrp X%i, #%x", buf&0x1F, ((buf >> 29) & 0x3) | (((buf >> 5) & 0x7FFFF) << 2)); return;}
    sprintf(out, "und");
}