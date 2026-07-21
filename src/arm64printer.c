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
void sprint_x_x_x(char** out, char* name, uint32_t buf) {
    char* ptr = *out;
    uint8_t rd = (buf >> 0) & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    uint8_t rm = (buf >> 16) & 0x1F;
    uint8_t shift = (buf >> 10) & 0x3;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    char sign = ' ';
    if (buf&(1<<29)) sign = 'S';
    ptr += sprintf(ptr, "%s%c %c%i, %c%i, %c%i",
        name, sign, size, rd, size, rn, size, rm);
    if (shift) ptr += sprintf(ptr, "{%i}", shift);
    *out = ptr;
}
void sprint_x_x_imm(char** out, char* name, uint32_t buf) {
    char* ptr = *out;
    uint8_t rd = buf & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    uint16_t imm = (buf >> 10) & 0xFFF;
    uint8_t shift = (buf >> 22) & 0x3;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    char sign = ' ';
    if (buf&(1<<29)) sign = 'S';
    ptr += sprintf(ptr, "%s%c %c%i, %c%i, #%x",
        name, sign,  size, rd, size, rn, imm);
    if (shift) ptr += sprintf(ptr, "{%i}", shift);
    *out = ptr;
}
void sprint_x_mem(char** out, char* name, uint32_t buf) {
    char* ptr = *out;
    uint8_t rd = buf & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    int16_t imm = (buf >> 12) & 0x1FF;
    int U = (buf >> 20) & 1;  // UR
    int W = (buf >> 10) & 1;  // Write-back?
    int P = (buf >> 11) & 1;  // Pre-indexed?
    if (imm & 0x100) {
        imm |= 0xFE00;
    }
    char size = 'W';
    if (buf&(1<<30)) size = 'X';
    ptr += sprintf(ptr, "%s %c%i, [X%i",
        name, size, rd, rn);
    if (P || U) {
        // Pre-indexed: [Xn, #offset]
        if (W) {
            ptr += sprintf(ptr, ", #%i]!", imm);
        } else if (U) {
            ptr += sprintf(ptr, ", #%i]", imm);
        } else {
            ptr += sprintf(ptr, "]");
        }
    } else {
        // Post-indexed: [Xn], #offset
        ptr += sprintf(ptr, "]");
        if (W && imm) {
            ptr += sprintf(ptr, ", #%i", imm);
        }
    }
    *out = ptr;
}
void sprint_x_imm(char** out, char* name, uint32_t buf) {
    char* ptr = *out;
    uint8_t rd = buf & 0x1F;
    uint16_t imm = (buf >> 5) & 0xFFFF;
    uint8_t shift = (buf >> 21) & 0x3;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    ptr += sprintf(ptr, "%s %c%i, #%i", name, size, rd, imm);
    if (shift) ptr += sprintf(ptr, "{%i}", shift);
    *out = ptr;
}
void sprint_arm(char* out, uint32_t buf) {
    out += sprintf(out, "\033[35m");
    if (comp("-0-01011------------------------", buf))
        sprint_x_x_x(&out, "add", buf);
    else if (comp("-1-01011------------------------", buf))
        sprint_x_x_x(&out, "sub", buf);
    else if (comp("-0-10001------------------------", buf))
        sprint_x_x_imm(&out, "add", buf);
    else if (comp("-1-10001------------------------", buf))
        sprint_x_x_imm(&out, "sub", buf);
    else if (comp("-1-01010------------------------", buf))
        sprint_x_x_x(&out, "eor", buf);
    else if (comp("1-11100-01----------------------", buf))
        sprint_x_mem(&out, "ldr", buf);
    else if (comp("1-11100-00----------------------", buf))
        sprint_x_mem(&out, "str", buf);
    else if (comp("-101001010----------------------", buf))
        sprint_x_imm(&out, "movz", buf);
        
    else if (comp("11010100001---------------------", buf))
        out += sprintf(out, "brk %x", (buf>>5)&0xFFFF);
    else if (comp("1101011001011111----------------", buf))
        out += sprintf(out, "ret X%i", (buf>>5)&0x1F);
    else if (comp("1101011000111111000000-----00000", buf))
        out += sprintf(out, "blr X%i", (buf>>5)&0x1F);
    else if (comp("0--10000------------------------", buf))
        out += sprintf(out, "adr X%i, #%x", buf&0x1F, ((buf >> 29) & 0x3) | (((buf >> 5) & 0x7FFFF) << 2));
    else if (comp("1--10000------------------------", buf))
        out += sprintf(out, "adrp X%i, #%x", buf&0x1F, ((buf >> 29) & 0x3) | (((buf >> 5) & 0x7FFFF) << 2));
    else out += sprintf(out, "und");
    out += sprintf(out, "\033[0m");
}
