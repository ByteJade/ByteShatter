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
    uint8_t rt = (buf >> 10) & 0x1F;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    char sign = ' ';
    if (buf&(1<<29)) sign = 'S';
    sprintf(out, "%s%c %c%i, %c%i, %c%i",
        name, sign, size, rd, size, rn, size, rt);
}
void sprint_x_x_imm(char* out, char* name, uint32_t buf) {
    uint8_t rd = buf & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    uint16_t imm = (buf >> 10) & 0xFFF;
    uint8_t shift = (buf >> 21) & 0x3;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    char sign = ' ';
    if (buf&(1<<29)) sign = 'S';
    out += sprintf(out, "%s%c, %c%i, %c%i, #%i",
        name, sign,  size, rd, size, rn, imm);
    if (shift) out += sprintf(out, "{%i}", shift);
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
    if (comp("-0-01011--0---------------------", buf))
        sprint_x_x_x(out, "add", buf); return;
    if (comp("-1-01011--0---------------------", buf))
        sprint_x_x_x(out, "sub", buf); return;
    if (comp("-0-10001--0---------------------", buf))
        sprint_x_x_imm(out, "add", buf); return;
    if (comp("-1-10001--0---------------------", buf))
        sprint_x_x_imm(out, "sub", buf); return;
}