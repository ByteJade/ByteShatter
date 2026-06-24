#include "encoder.h"
#include "core.h"
#include "memory.h"
#include "dlmanager.h"
#include "arm64emitter.h"

void encode(X64_instruction* buf) {
    switch (buf->type) {
        case SUB:{
            if (buf->optype0 == REG && buf->optype1 == REG)
                emit_sub_reg(buf->reg0, buf->reg0, buf->reg1);
            else if (buf->optype0 == REG && buf->optype1 == IMM)
                emit_sub_imm(buf->reg0, buf->reg0, buf->imm1);
            else if (buf->reverse && buf->optype0 == IMM && buf->optype1 == REG)
                emit_sub_imm(buf->reg1, buf->reg1, buf->imm0);
            else panic("unhandled subtraction type");
        } break;
        case ADD:{
            if (buf->optype0 == REG && buf->optype1 == REG)
                emit_add_reg(buf->reg0, buf->reg0, buf->reg1);
            else if (buf->optype0 == REG && buf->optype1 == IMM)
                emit_add_imm(buf->reg0, buf->reg0, buf->imm1);
            else if (buf->reverse && buf->optype0 == IMM && buf->optype1 == REG)
                emit_add_imm(buf->reg1, buf->reg1, buf->imm0);
            else panic("unhandled addition type");
        } break;
        case MOV:{
            if (buf->optype0 == REG && buf->optype1 == REG) {
                emit_mov_reg(buf->reg0, buf->reg1);
            } else if (buf->optype1 == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->imm1;
                if (offset > UINT16_MAX || offset < 0) panic("DECODER::ILLEGAL_OFFSET");
                if (is_external_offset(offset)) {
                    emit_movz(buf->reg0, offset, 0);
                    emit_add_reg(buf->reg0, buf->reg0, RIP);
                    emit_ldr_reg(buf->reg0, buf->reg0, 0);
                } else {
                    warning("DECODER::ILLEGAL_RIP");
                    emit_brk(0);
                }
            }
        } break;
        case TST:{
            if (buf->optype0 == REG && buf->optype1 == REG) {
                emit_tst_reg(buf->reg0, buf->reg1);
            }
        } break;
        case JE:{
            emit_brk(0);
        } break;
        case CALL:{
            if (buf->optype1 == REG) {
                emit_blr_reg(buf->reg1);
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