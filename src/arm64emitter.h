#ifndef EMITTER_H
#define EMITTER_H

#include "memory.h"
#include "armdef.h"

#define _construct_r_r_imm(op, rd, rn, imm) \
    ((op) | ((imm) << 10) | (x64_regs[rn] << 5) | x64_regs[rd])
#define _construct_r_r_r(op, rd, rn, rm) \
    ((op) | (x64_regs[rm] << 16) | (x64_regs[rn] << 5) | x64_regs[rd])

#define emit_movz(rd, imm, shift) \
    emit32(0xD2800000 | (shift << 21) | (imm << 5) | x64_regs[rd])
#define emit_sub_imm(rd, rn, imm) \
    emit32(_construct_r_r_imm(0xD1000000, rd, rn, imm))
#define emit_sub_reg(rd, rn, rm) \
    emit32(_construct_r_r_r(0xCB000000, rd, rn, rm))
#define emit_add_imm(rd, rn, imm) \
    emit32(_construct_r_r_imm(0x91000000, rd, rn, imm))
#define emit_add_reg(rd, rn, rm) \
    emit32(_construct_r_r_r(0x8B000000, rd, rn, rm))
#define emit_eor_reg(rd, rn, rm) \
    emit32(_construct_r_r_r(0xCA000000, rd, rn, rm))
#define emit_and_imm(rd, rn, imm) \
    emit32(_construct_r_r_imm(0x92000000, rd, rn, imm))
#define emit_ldr_reg(rd, rn, imm) \
    emit32(_construct_r_r_imm(0xF9400000, rd, rn, imm))
#define emit_tst_reg(rn, rm) \
    emit32(_construct_r_r_r(0xEA000000, XZR, rn, rm))
#define emit_blr_reg(rn) \
    emit32(0xD63F0000 | (x64_regs[rn] << 5))
#define emit_br_reg(rn) \
    emit32(0xD61F0000 | (x64_regs[rn] << 5))
#define emit_pop_reg(rn) \
    emit32(0xf8408780 | x64_regs[rn])
#define emit_push_reg(rn) \
    emit32(0xf81f8f80 | x64_regs[rn])
#define emit_brk(imm16) \
    emit32(0xD4200000 | (imm16 << 5))
#define emit_ret() \
    emit32(0xD65F03C0)

#endif