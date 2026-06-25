#include "encoder.h"
#include "armdef.h"
#include "core.h"
#include "memory.h"
#include "cache.h"
#include "dlmanager.h"
#include "arm64emitter.h"

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
            } else if (buf->op1.type == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op1.imm;
                if (offset > UINT16_MAX || offset < 0) panic("DECODER::ILLEGAL_OFFSET");
                if (is_external_offset(offset)) {
                    emit_movz(buf->op0.reg, offset, 0);
                    emit_add_reg(buf->op0.reg, buf->op0.reg, RIP);
                    emit_ldr_reg(buf->op0.reg, buf->op0.reg, 0);
                } else {
                    warning("DECODER::ILLEGAL_RIP");
                    emit_brk(0);
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
        case POP:{
            if (buf->op0.type == REG) {
                emit_pop_reg(buf->op0.reg);
            }
        } break;
        case JE:{
            emit_brk(cache_jump_point(JE, buf->op0.imm));
        } break;
        case CALL:{
            if (buf->op0.type == REG) {
                emit_blr_reg(buf->op0.reg);
            } else {
                warning("DECODER::ILLEGAL_CALL");
                emit_brk(0);
            }
        } break;
        case RET:{
            emit_ret();
        } break;
        default:
            emit_brk(0);
    }
}