#ifndef EMITTER_H
#define EMITTER_H

#include <stdint.h>

void emit_movz(uint8_t rd, uint16_t imm16, uint8_t shift);
void emit_sub_imm(uint8_t rd, uint8_t rn, uint16_t imm);
void emit_sub_reg(uint8_t rd, uint8_t rn, uint8_t rm);
void emit_add_imm(uint8_t rd, uint8_t rn, uint16_t imm);
void emit_add_reg(uint8_t rd, uint8_t rn, uint8_t rm);
void emit_eor_reg(uint8_t rd, uint8_t rn, uint8_t rm);
void emit_and_imm(uint8_t rd, uint8_t rn, uint16_t imm);
void emit_ldr_reg(uint8_t rt, uint8_t rn, int16_t offset);
void emit_mov_reg(uint8_t rd, uint8_t rn);
void emit_tst_reg(uint8_t rn, uint8_t rm);
void emit_blr_reg(uint8_t rn);
void emit_br_reg(uint8_t rn);
void emit_pop_reg(uint8_t rn);
void emit_push_reg(uint8_t rn);
void emit_brk(uint16_t imm16);
void emit_ret();

#endif