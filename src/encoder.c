#include "encoder.h"
#include "armdef.h"
#include "core.h"
#include "memory.h"
#include "cache.h"
#include "dlmanager.h"
#include "arm64emitter.h"
#include <stdint.h>

void encode(X64_instruction* buf) {
    switch (buf->type) {
        case SUB:{
            if (buf->op0.type == REG && buf->op1.type == REG)
                emit_sub_reg(buf->op0.reg, buf->op0.reg, buf->op1.reg);
            else if (buf->op0.type == REG && buf->op1.type == IMM)
                emit_sub_imm(buf->op0.reg, buf->op0.reg, buf->op1.imm);
            else panic("unhandled subtraction type");
        } break;
        case ADD:{
            if (buf->op0.type == REG && buf->op1.type == REG)
                emit_add_reg(buf->op0.reg, buf->op0.reg, buf->op1.reg);
            else if (buf->op0.type == REG && buf->op1.type == IMM)
                emit_add_imm(buf->op0.reg, buf->op0.reg, buf->op1.imm);
            else panic("unhandled addition type");
        } break;
        case MOV:{
            if (buf->op0.type == REG && buf->op1.type == REG) {
                emit_mov_reg(buf->op0.reg, buf->op1.reg);
            }else if (buf->op0.type == REG && buf->op1.type == IMM){
                if (buf->op1.imm == 0) {
                    emit_eor_reg(buf->op0.reg, buf->op0.reg, buf->op0.reg);
                } else if (buf->op1.imm < INT16_MAX && buf->op1.imm > INT16_MIN) {
                    emit_movz(buf->op0.reg, buf->op1.imm & 0xFFFF, 0);
                } else if (buf->op1.imm < INT32_MAX && buf->op1.imm > INT32_MIN) {
                    emit_movz(buf->op0.reg, buf->op1.imm & 0xFFFF, 16);
                }
            }else if (buf->op1.type == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op1.imm;
                if (offset > UINT16_MAX || offset < 0) panic("ENCODER::ILLEGAL_OFFSET");
                if (is_external_offset(offset)) {
                    emit_movz(buf->op0.reg, offset, 0);
                    emit_add_reg(buf->op0.reg, buf->op0.reg, RIP);
                    emit_ldr_reg(buf->op0.reg, buf->op0.reg, 0);
                } else {
                    warning("ENCODER::ILLEGAL_RIP");
                    emit_brk(0);
                }
            }
        } break;
        case LEA:{
            if (buf->op1.type == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op1.imm;
                if (offset > UINT16_MAX || offset < 0) panic("ENCODER::ILLEGAL_OFFSET");
                if (is_external_offset(offset)) {
                    emit_movz(SC1, offset, 0);
                    emit_add_reg(buf->op0.reg, SC1, RIP);
                } else {
                    emit_brk(cache_patch_point(LEA, buf->op0.reg, buf->op1.imm));
                    warning("ENCODER::ILLEGAL_RIP");
                }
            }
        } break;
        case TST:{
            if (buf->op0.type == REG && buf->op1.type == REG) {
                emit_tst_reg(buf->op0.reg, buf->op1.reg);
            }
        } break;
        case XOR:{
            if (buf->op0.type == REG && buf->op1.type == REG) {
                emit_eor_reg(buf->op0.reg, buf->op0.reg, buf->op1.reg);
            }
        } break;
        case AND:{
            if (buf->op0.type == REG && buf->op1.type == IMM) {
                uint64_t imm = buf->op1.imm;
                if (imm == 0xFFFFFFFFFFFFFFF0) {
                    emit_and_imm(buf->op0.reg, buf->op1.reg, 0b01111100111011);
                } else {
                    emit_and_imm(buf->op0.reg, buf->op1.reg, imm);
                }
            }
        } break;
        case POP:{
            if (buf->op0.type == REG) {
                emit_pop_reg(buf->op0.reg);
            }
        } break;
        case PUSH:{
            if (buf->op0.type == REG) {
                emit_push_reg(buf->op0.reg);
            }
        } break;
        case JE:{
            emit_brk(cache_patch_point(JE, 0, buf->op0.imm));
        } break;
        case JMP:{
            if (buf->op0.type == REG) {
                emit_br_reg(buf->op0.reg);
            } else if (buf->op0.type == IMM) {
                emit_brk(cache_patch_point(JMP, 0, buf->op0.imm));
            } else {
                int32_t offset = get_gp() + buf->op0.imm;
                if (offset > INT16_MAX || offset < INT16_MIN) panic("ENCODER::ILLEGAL_OFFSET");
                if (is_external_offset(offset)) {
                    emit_movz(SC1, offset, 0);
                    emit_add_reg(SC1, SC1, RIP);
                    emit_ldr_reg(SC1, SC1, 0);
                    // wrapper
                    emit_mov_reg(RAX, RDI);
                } else {
                    warning("ENCODER::ILLEGAL_RIP");
                    emit_brk(0);
                }
                emit_br_reg(SC1);
            }
            
        } break;
        case CALL:{
            emit32(0xA9BF7BFD);
            if (buf->op0.type == REG) {
                // stp x29, x30, [sp, #-16]!
                emit_blr_reg(buf->op0.reg);
            } else if (buf->op0.type == IMM) {
                emit_brk(cache_patch_point(CALL, 0, buf->op0.imm));
            } else {
                int32_t offset = get_gp() + buf->op0.imm;
                if (offset > INT16_MAX || offset < INT16_MIN) panic("ENCODER::ILLEGAL_OFFSET");
                if (is_external_offset(offset)) {
                    emit_movz(SC1, offset, 0);
                    emit_add_reg(SC1, SC1, RIP);
                    emit_ldr_reg(SC1, SC1, 0);
                    // wrapper
                    emit_mov_reg(RAX, RDI);
                    // for testing purposes only!
                    emit_pop_reg(AR7);
                } else {
                    warning("ENCODER::ILLEGAL_RIP");
                    emit_brk(0);
                }
                emit_blr_reg(SC1);
            }
                emit32(0xA8C17BFD);
        } break;
        case RET:{
            emit_ret();
        } break;
        default:
            emit_brk(0);
    }
}