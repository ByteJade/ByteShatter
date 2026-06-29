#include "encoder.h"
#include "armdef.h"
#include "core.h"
#include "memory.h"
#include "cache.h"
#include "dlmanager.h"
#include "arm64emitter.h"
#include <stdint.h>

/*
TODO:
- encoding for 8, 16 and 32 bit instructions
- encoding NZCV flags for math ops
- memory ops, like "add [rax], 0x2"
- better "unhandled"
- clean emitter
*/
void encode(X64_instruction* buf) {
    uint8_t r0 = buf->op0.reg;
    uint8_t r1 = buf->op1.reg;
    uint32_t sf = (buf->size == 64) * SF;
    switch (buf->type) {
        case SUB:{
            if (buf->op0.type == REG && buf->op1.type == REG)
                emit32(sf|_construct_r_r_r(SUB_REG|S, r0, r0, r1));
            else if (buf->op0.type == REG && buf->op1.type == IMM)
                emit32(sf|_construct_r_r_imm(SUB_IMM|S, r0, r0, buf->op1.imm));
            else panic("ENCODER::UNHANDLED_SUB");
        } break;
        case ADD:{
            if (buf->op0.type == REG && buf->op1.type == REG)
                emit32(sf|_construct_r_r_r(ADD_REG|S, r0, r0, r1));
            else if (buf->op0.type == REG && buf->op1.type == IMM)
                emit32(sf|_construct_r_r_imm(ADD_IMM|S, r0, r0, buf->op1.imm));
            else panic("ENCODER::UNHANDLED_ADD");
        } break;
        case MOV:{
            if (buf->op0.type == REG && buf->op1.type == REG) {
                emit_add_imm(r0, r1, 0);
            }else if (buf->op0.type == REG && buf->op1.type == IMM){
                if (buf->op1.imm == 0) {
                    emit_eor_reg(r0, r0, r0);
                } else if (buf->op1.imm < INT16_MAX && buf->op1.imm > INT16_MIN) {
                    emit_movz(r0, buf->op1.imm & 0xFFFF, 0);
                } else if (buf->op1.imm < INT32_MAX && buf->op1.imm > INT32_MIN) {
                    emit_movz(r0, buf->op1.imm & 0xFFFF, 16);
                }
            }else if (buf->op1.type == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op1.imm;
                if (offset > UINT16_MAX || offset < 0) panic("ENCODER::ILLEGAL_OFFSET");
                if (is_external_offset(offset)) {
                    emit_movz(r0, offset, 0);
                    emit_add_reg(r0, r0, RIP);
                    emit_ldr_reg(r0, r0, 0);
                } else {
                    warning("ENCODER::ILLEGAL_RIP");
                    emit_brk(0);
                }
            } else panic("ENCODER::UNHANDLED_MOV");
        } break;
        case LEA:{
            if (buf->op1.type == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op1.imm;
                if (offset > UINT16_MAX || offset < 0) panic("ENCODER::ILLEGAL_OFFSET");
                if (is_external_offset(offset)) {
                    emit_movz(SC1, offset, 0);
                    emit_add_reg(r0, SC1, RIP);
                } else {
                    emit_brk(cache_patch_point(LEA, r0, buf->op1.imm));
                    warning("ENCODER::ILLEGAL_RIP");
                }
            } else panic("ENCODER::UNHANDLED_LEA");
        } break;
        case TST:{
            if (buf->op0.type == REG && buf->op1.type == REG) {
                emit_tst_reg(r0, r1);
            } else panic("ENCODER::UNHANDLED_TST");
        } break;
        case XOR:{
            if (buf->op0.type == REG && buf->op1.type == REG) {
                emit32(sf|_construct_r_r_r(EOR_REG, r0, r0, r1));
            } else panic("ENCODER::UNHANDLED_XOR");
        } break;
        case AND:{
            if (buf->op0.type == REG && buf->op1.type == IMM) {
                uint64_t imm = buf->op1.imm;
                if (imm == 0xFFFFFFFFFFFFFFF0) {
                    emit_and_imm(r0, r1, 0b01111100111011);
                } else {
                    emit_and_imm(r0, r1, imm);
                }
            } else panic("ENCODER::UNHANDLED_AND");
        } break;
        case POP:{
            if (buf->op0.type == REG) {
                emit_pop_reg(r0);
            } else panic("ENCODER::UNHANDLED_POP");
        } break;
        case PUSH:{
            if (buf->op0.type == REG) {
                emit_push_reg(r0);
            } else panic("ENCODER::UNHANDLED_PUSH");
        } break;
        case JE:{
            emit_brk(cache_patch_point(JE, 0, buf->op0.imm));
        } break;
        case JMP:{
            if (buf->op0.type == REG) {
                emit_br_reg(r0);
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
                    emit_add_imm(RAX, RDI, 0);
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
                emit_blr_reg(r0);
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
                    emit_add_imm(RAX, RDI, 0);
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
        case RET: emit_ret(); break;
        case EBR: /*emit_bti();*/ break;
        default:
            panic("ENCODER::UNKNOWN_INSTRUCTION: %x", buf->type);
    }
}