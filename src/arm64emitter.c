#include "arm64emitter.h"
#include "memory.h"
#include "armdef.h"

void emit_movz(uint8_t rd, uint16_t imm16, uint8_t shift) {
    emit32(0xD2800000 | (shift << 21) | (imm16 << 5) | x64_regs[rd]);
}
void emit_sub_imm(uint8_t rd, uint8_t rn, uint16_t imm) {
    emit32(0xD1000000 | (imm << 10) | (x64_regs[rn] << 5) | x64_regs[rd]);
}
void emit_sub_reg(uint8_t rd, uint8_t rn, uint8_t rm) {
    emit32(0xCB000000 | (x64_regs[rm] << 16) | (x64_regs[rn] << 5) | x64_regs[rd]);
}
void emit_add_imm(uint8_t rd, uint8_t rn, uint16_t imm) {
    emit32(0x91000000 | (imm << 10) | (x64_regs[rn] << 5) | x64_regs[rd]);
}
void emit_add_reg(uint8_t rd, uint8_t rn, uint8_t rm) {
    emit32(0x8B000000 | (x64_regs[rm] << 16) | (x64_regs[rn] << 5) | x64_regs[rd]);
}
void emit_eor_reg(uint8_t rd, uint8_t rn, uint8_t rm) {
    emit32(0xCA000000 | (x64_regs[rm] << 16) | (x64_regs[rn] << 5) | x64_regs[rd]);
}
void emit_and_imm(uint8_t rd, uint8_t rn, uint16_t imm) {
    emit32(0x92000000 | (imm << 10) | (x64_regs[rn] << 5) | x64_regs[rd]);
}
void emit_ldr_reg(uint8_t rt, uint8_t rn, int16_t offset) {
    emit32(0xF9400000 | (offset << 10) | (x64_regs[rn] << 5) | x64_regs[rt]);
}
void emit_mov_reg(uint8_t rd, uint8_t rn) {
    emit32(0x91000000 | (x64_regs[rn] << 5) | x64_regs[rd]);
}
void emit_tst_reg(uint8_t rn, uint8_t rm) {
    emit32(0xEA000000 | (x64_regs[rm] << 16) | (x64_regs[rn] << 5) | 31);
}
void emit_blr_reg(uint8_t rn) {
    emit32(0xD63F0000 | (x64_regs[rn] << 5));
}
void emit_pop_reg(uint8_t rn) {
    emit32(0xf8408780 | x64_regs[rn]);
}
void emit_push_reg(uint8_t rn) {
    emit32(0xf81f8f80 | x64_regs[rn]);
}
void emit_brk(uint16_t imm16) {
    emit32(0xD4200000 | (imm16 << 5));
}
void emit_ret() {
    emit32(0xD65F03C0);
}
