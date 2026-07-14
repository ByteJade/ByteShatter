#include "arm64printer.h"
#include <stdio.h>
void sprint_x_x_x(char** out, uint32_t buf) {
    char* ptr = *out;
    uint8_t rd = (buf >> 0) & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    uint8_t rt = (buf >> 10) & 0x1F;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    ptr += sprintf(ptr, "%c%i, %c%i, %c%i", size, rd, size, rn, size, rt);
    *out = ptr;    
}
void sprint_x_x_imm(char** out, uint32_t buf) {
    char* ptr = *out;
    uint8_t rd = buf & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    uint16_t imm = (buf >> 10) & 0xFFF;
    uint8_t shift = (buf >> 21) & 0x3;
    char size = 'W';
    if (buf&(1<<31)) size = 'X';
    ptr += sprintf(ptr, "%c%i, %c%i, #%i", size, rd, size, rn, imm);
    if (shift) ptr += sprintf(ptr, "{%i}", shift);
    *out = ptr;
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
    char* ptr = out;
    uint8_t rd = (buf >> 0) & 0x1F;
    uint8_t rn = (buf >> 5) & 0x1F;
    uint8_t rm = (buf >> 16) & 0x1F;
    uint8_t rt = (buf >> 0) & 0x1F;
    uint8_t rt2 = (buf >> 10) & 0x1F;
    uint8_t cond = (buf >> 12) & 0xF;
    uint8_t option = (buf >> 13) & 0x7;
    uint8_t shift = (buf >> 22) & 0x3;
    uint8_t size = (buf >> 30) & 0x3;
    if ((buf & 0x9F000000) == 0x10000000) {
        ptr += sprintf(ptr, "adr X%i", buf & 0x1F);
        return;
    }
    if ((buf & 0x9F000000) == 0x90000000) {
        ptr += sprintf(ptr, "adrp X%i", buf & 0x1F);
        return;
    }
    switch (buf & 0x7F800000) {
        case 0x11000000:
            ptr += sprintf(ptr, "add ");
            sprint_x_x_imm(&ptr, buf);
            return;
        case 0x31000000:
            ptr += sprintf(ptr, "adds ");
            sprint_x_x_imm(&ptr, buf);
            return;
        case 0x51000000:
            ptr += sprintf(ptr, "sub");
            sprint_x_x_imm(&ptr, buf);
            return;
        case 0x71000000:
            ptr += sprintf(ptr, "subs");
            sprint_x_x_imm(&ptr, buf);
            return;
        case 0x12000000:
            ptr += sprintf(ptr, "and ");
            sprint_x_x_imm(&ptr, buf);
            return;
        case 0x32000000:
            ptr += sprintf(ptr, "ands ");
            sprint_x_x_imm(&ptr, buf);
            return;

        case 0x12800000:
            ptr += sprintf(ptr, "movn");
            sprint_x_imm(&ptr, buf);
            return;
        case 0x52800000:
            ptr += sprintf(ptr, "movz ");
            sprint_x_imm(&ptr, buf);
            return;
        case 0x72800000:
            ptr += sprintf(ptr, "movk ");
            sprint_x_imm(&ptr, buf);
            return;
    }
    ptr += sprintf(ptr, "und");
}