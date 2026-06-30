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
void emit_branch(X64_instruction* buf, uint32_t code, uint8_t type) {
    if (buf->op0.type == REG) {
        emit32(code | (x64_regs[buf->op0.reg] << 5));
    } else if (buf->op0.type == IMM) {
        emit_brk(cache_patch_point(type, 0, buf->op0.imm));
    } else if (buf->op0.type == (MEM|IMM)) {
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
        emit32(code | (x64_regs[SC1] << 5));
    }
}
void encode(X64_instruction* buf) {
    uint8_t r0 = buf->op0.reg;
    uint8_t r1 = buf->op1.reg;
    uint8_t t0 = buf->op0.type;
    uint8_t t1 = buf->op1.type;
    uint32_t sf = (buf->size == 64) * SF;
    switch (buf->type) {
        case SUB:{
            if (t0 == REG && t1 == REG)
                emit32(sf|_construct_r_r_r(SUB_REG|S, r0, r0, r1));
            else if (t0 == REG && t1 == IMM)
                emit32(sf|_construct_r_r_imm(SUB_IMM|S, r0, r0, buf->op1.imm&IMM12));
            else panic("ENCODER::UNHANDLED_SUB");
        } break;
        case ADD:{
            if (t0 == REG && t1 == REG)
                emit32(sf|_construct_r_r_r(ADD_REG|S, r0, r0, r1));
            else if (t0 == REG && t1 == IMM)
                emit32(sf|_construct_r_r_imm(ADD_IMM|S, r0, r0, buf->op1.imm&IMM12));
            else panic("ENCODER::UNHANDLED_ADD");
        } break;
        case MOV:{
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_r(ADD_IMM, r0, r1, 0));
            }else if (t0 == REG && t1 == IMM){
                int64_t imm = buf->op1.imm;
                if (imm > INT16_MAX || imm < INT16_MIN) panic("ENCODER::ILLEGAL_IMM");
                emit_movz(r0, imm & IMM12, 0);
            } else if (t0 == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op0.imm;
                if (offset > INT16_MAX || offset < INT16_MIN) panic("ENCODER::ILLEGAL_OFFSET");
                emit_movz(SC1, offset, 0);
                emit_add_reg(SC1, SC1, RIP);
                if (t1 == REG)
                    emit32(sf|_construct_r_r_imm(STR_REG, r1, SC1, 0));
                else {
                    emit_movz(SC2, buf->op1.imm, 0);
                    emit32(sf|_construct_r_r_imm(STR_REG, SC2, SC1, 0));
                }
            } else if (t1 == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op1.imm;
                if (offset > INT16_MAX || offset < INT16_MIN) panic("ENCODER::ILLEGAL_OFFSET");
                emit_movz(r0, offset, 0);
                emit_add_reg(r0, r0, RIP);
                emit_ldr_reg(r0, r0, 0);
            } else if (t1&MEM) {
                if (t1 == (MEM|REG)) {
                    emit_ldr_reg(r0, r1, 0);
                } else if (t1 == (MEM|REG|IMM)) {
                    if (buf->op1.imm > 0)
                        emit_add_imm(SC1, r1, buf->op1.imm);
                    else emit_sub_imm(SC1, r1, -buf->op1.imm);
                    emit_ldr_reg(r0, SC1, 0);
                } else panic("ENCODER::UNHANDLED_MOV");
            } else if (t0&MEM) {
                if (t0 == (MEM|REG)) {
                    emit32(sf|_construct_r_r_imm(STR_REG, r1, r0, 0));
                } else if (t0 == (MEM|REG|IMM)) {
                    if (buf->op0.imm > 0)
                        emit_add_imm(SC1, r0, buf->op0.imm);
                    else emit_sub_imm(SC1, r0, -buf->op0.imm);
                    emit32(sf|_construct_r_r_imm(STR_REG, r1, SC1, 0));
                } else panic("ENCODER::UNHANDLED_MOV");
            } else panic("ENCODER::UNHANDLED_MOV");
        } break;
        case LEA:{
            if (t1 == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op1.imm;
                if (offset > INT16_MAX || offset < INT16_MIN) panic("ENCODER::ILLEGAL_OFFSET");
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
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_r(ANDS_REG, XZR, r0, r1));
            } else panic("ENCODER::UNHANDLED_TST");
        } break;
        case CMP:{
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_r(SUB_REG|S, XZR, r0, r1));
            } else if (t1 == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op1.imm;
                if (offset > INT16_MAX || offset < INT16_MIN) panic("ENCODER::ILLEGAL_OFFSET");
                emit_movz(SC1, offset, 0);
                emit_add_reg(SC1, SC1, RIP);
                emit_ldr_reg(SC1, SC1, 0);
                emit32(sf|_construct_r_r_r(SUB_REG|S, XZR, r0, SC1));
            } else panic("ENCODER::UNHANDLED_CMP");
        } break;
        case XOR:{
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_r(EOR_REG, r0, r0, r1));
            } else panic("ENCODER::UNHANDLED_XOR");
        } break;
        case AND:{
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_r(AND_REG, r0, r0, r1));
            } else if (t0 == REG && t1 == IMM) {
                uint64_t imm = buf->op1.imm;
                if (imm == 0xFFFFFFFFFFFFFFF0) {
                    emit_and_imm(r0, r1, 0b01111100111011);
                } else {
                    emit_and_imm(r0, r1, imm);
                }
            } else panic("ENCODER::UNHANDLED_AND");
        } break;
        case POP:{
            if (t0 == REG) {
                emit_pop_reg(r0);
            } else panic("ENCODER::UNHANDLED_POP");
        } break;
        case PUSH:{
            if (t0 == REG) {
                emit_push_reg(r0);
            } else panic("ENCODER::UNHANDLED_PUSH");
        } break;
        case JE:{
            emit_brk(cache_patch_point(JE, 0, buf->op0.imm));
        } break;
        case JMP:{
            emit_branch(buf, BR_REG, JMP);
        } break;
        case CALL:{
            emit32(0xA9BF7BFD);
            emit_branch(buf, BLR_REG, CALL);
            emit32(0xA8C17BFD);
        } break;
        case RET: emit_ret(); break;
        case EBR: /*emit_bti();*/ break;
        default:
            panic("ENCODER::UNKNOWN_INSTRUCTION: %x", buf->type);
    }
}