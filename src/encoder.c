#include "encoder.h"
#include "armdef.h"
#include "core.h"
#include "decoder.h"
#include "memory.h"
#include "cache.h"
#include "dlmanager.h"
#include "arm64emitter.h"
#include <stdint.h>

/*
TODO:
- encoding for 8, 16 and 32 bit instructions
- clean emitter
*/
void emit_rip(uint8_t rd, int32_t offset) {
    uint64_t full = (uint64_t)(get_guest() + offset);
    int64_t target = full & ~0xFFF;
    int64_t current = (uint64_t)(get_host() + get_hp()) & ~0xFFF;
    int32_t delta = (target - current) / 4096;
    if (delta < -524288 || delta > 524287) {
        panic("ENCODER::TOO_LARGE_DISTANCE");
    }
    emit_adrp(SC1, delta);
    emit_add_imm(rd, SC1, full & 0xFFF);
}
void emit_branch(X64_instruction* buf, uint32_t code, uint8_t type) {
    if (buf->op0.type == REG) {
        emit32(code | (x64_regs[buf->op0.reg] << 5));
    } else if (buf->op0.type == IMM) {
        emit_brk(cache_patch_point(type, 0, buf->op0.imm));
    } else if (buf->op0.type == (MEM|IMM)) {
        int32_t offset = get_gp() + buf->op0.imm;
        if (is_external_offset(offset)) {
            emit_rip(SC1, offset);
            emit_ldr_reg(SC1, SC1, 0);
            emit32(code | (x64_regs[SC1] << 5));
            // wrapper
        } else {
            warning("ENCODER::ILLEGAL_RIP");
            emit_brk(0);
        }
    }
}
void emit_add_signed(uint8_t r0, uint8_t r1, int64_t imm) {
    if (imm > 0)
        emit_add_imm(r0, r1, imm);
    else emit_sub_imm(r0, r1, -imm);
}
void emit_address_decode(Operand* op) {
    uint8_t t = op->type;
    if (t == (MEM|IMM)) {
        emit_rip(SC1, get_gp() + op->imm);
        return;
    }
    if (t&IDX) {
        emit32(_construct_r_r_imm(SF|ADD_IMM, SC1, op->idx, 0));
        if (op->scale != 0) {
            emit_lsl_imm(SC1, SC1, op->scale);
        }
        if (t&REG) {
            emit32(_construct_r_r_r(SF|ADD_REG, SC1, SC1, op->reg));
        }
        if ((t&IMM) && op->imm != 0) {
            emit_add_signed(SC1, SC1, op->imm);
        }
    }else {
        if (t&IMM) {
            emit_add_signed(SC1, op->reg, op->imm);
        } else {
            emit32(_construct_r_r_imm(SF|ADD_IMM, SC1, op->reg, 0));
        }
    }
}
void emit_imm(Operand* op) {
    if (op->imm >= 0) {
        if (op->imm <= INT16_MAX) emit_movz(SC2, op->imm, 0);
        else {
            emit_mov32(SC2, op->imm);
        }
    } else {
        if (~op->imm <= INT16_MAX) emit_movn(SC2, ~op->imm, 0);
        else {
            emit_mov32(SC2, op->imm);
            emit32(SXTW_REG | (x64_regs[SC2] << 5) | x64_regs[SC2]);
        }
    }
}
void emit_neon(X64_instruction* buf, int opcode) {
    uint8_t r0 = buf->op0.reg;
    uint8_t r1 = buf->op1.reg;
    uint8_t t0 = buf->op0.type;
    uint8_t t1 = buf->op1.type;
    uint32_t osf = (buf->prefix == REPN) * FT;

    uint32_t msf = (buf->prefix == REPN) * MFT;
    if (t0 == (REG|XMM) && t1 == (REG|XMM)) {
        emit32(osf|DIV_NEON|(r0)|(r0<<5)|(r1<<16));
    } else if (t0 & MEM) {
        emit_address_decode(&buf->op0);
        emit32(msf|LDR_NEON | (x64_regs[SC1]<<5) | 16);
        emit32(osf|opcode|(16)|(16<<5)|(r1<<16));
        emit32(msf|STR_NEON | (x64_regs[SC1]<<5) | 16);
    } else if (t1 & MEM) {
        emit_address_decode(&buf->op1);
        emit32(msf|LDR_NEON | (x64_regs[SC1]<<5) | 16);
        emit32(osf|opcode|(r0)|(r0<<5)|(16<<16));
    } else panic("ENCODER::UNHANDLED_NEON");
}
void encode8bit(X64_instruction* buf) {
    uint8_t r0 = buf->op0.reg;
    uint8_t r1 = buf->op1.reg;
    uint8_t t0 = buf->op0.type;
    uint8_t t1 = buf->op1.type;
    switch (buf->type) {
        case MOV: {
            if (t0&MEM) {
                emit_address_decode(&buf->op0);
                if (t1 == REG){
                    emit32(_construct_r_r_imm(STR8_REG, r1, SC1, 0));
                } else {
                    emit_movz(SC2, buf->op1.imm, 0);
                    emit32(_construct_r_r_imm(STR8_REG, SC2, SC1, 0));
                }
            } else panic("ENCODER::UNHANDLED_MOV");
        } break;
        case TST:{
            if (t0 == REG && t1 == REG) {
                emit32(0x12001c00 | (x64_regs[r0]<<5) | (x64_regs[r1])); 
                emit32(_construct_r_r_r(ANDS_REG, XZR, r0, r1));
            } else panic("ENCODER::UNHANDLED_TST");
        } break;
        case CMP:{
            if (t0 == REG) {
                emit32(_construct_r_r_r(SUB_IMM|S, XZR, r0, buf->op1.imm));
            } else if (t0&MEM) {
                emit_address_decode(&buf->op0);
                emit32(_construct_r_r_imm(LDR8_REG, SC1, SC1, 0));
                emit32(_construct_r_r_imm(SUB_IMM|S, XZR, SC1, buf->op1.imm));
            } else panic("ENCODER::UNHANDLED_CMP");
        } break;
        default:
            panic("ENCODER::UNKNOWN_8BIT_INSTRUCTION: %x", buf->type);
    }
}
void encode(X64_instruction* buf) {
    if (buf->size == 8) {
        encode8bit(buf);
        return;
    }
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
                emit_add_signed(r0, r0, buf->op1.imm);
            else if (t0&MEM) {
                emit_address_decode(&buf->op0);
                emit_ldr_reg(SC2, SC1, 0);
                if (t1 == REG) {
                    emit32(sf|_construct_r_r_r(ADD_REG|S, SC2, SC2, r1));
                } else {
                    emit_add_signed(SC2, SC2, buf->op1.imm);
                }
                if (sf) emit32(sf|_construct_r_r_imm(STR64_REG, SC2, SC1, 0));
                else emit32(sf|_construct_r_r_imm(STR32_REG, SC2, SC1, 0));
            } else panic("ENCODER::UNHANDLED_ADD");
        } break;
        case SHL:{
            if (t0 == REG && t1 == IMM)
                emit_lsl_imm(r0, r1, buf->op1.imm);
            else panic("ENCODER::UNHANDLED_SHL");
            break;
        }
        case SHR:{
            if (t0 == REG && t1 == IMM)
                emit_lsr_imm(r0, r1, buf->op1.imm);
            else panic("ENCODER::UNHANDLED_SHR");
            break;
        }
        case SAR:{
            if (t0 == REG && t1 == IMM)
                emit_asr_imm(r0, r1, buf->op1.imm);
            else panic("ENCODER::UNHANDLED_SAR");
            break;
        }
        case MOVSLQ: {
            if (t0 == REG && t1 == REG) {
                emit32(0x93407c00 | (x64_regs[r0]<<5) | (x64_regs[r1]));
            }else panic("ENCODER::UNHANDLED_MOVSLQ");
        } break;
        case MOVZX:
        case MOV:{
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_imm(ADD_IMM, r0, r1, 0));
            }else if (t0 == REG && t1 == IMM){
                int64_t imm = buf->op1.imm;
                if (imm < 0 || imm > UINT16_MAX) {
                    emit_imm(&buf->op1);
                    emit32(sf|_construct_r_r_imm(ADD_IMM, r0, SC2, 0));
                } else emit32(sf | MOVZ_IMM | (imm << 5) | x64_regs[r0]);
            } else if (t1&MEM) {
                emit_address_decode(&buf->op1);
                if (sf) emit32(_construct_r_r_imm(LDR64_REG, r0, SC1, 0));
                else emit32(_construct_r_r_imm(LDR32_REG, r0, SC1, 0));
            } else if (t0&MEM) {
                emit_address_decode(&buf->op0);
                if (t1 == REG){
                    if (sf) emit32(sf|_construct_r_r_imm(STR64_REG, r1, SC1, 0));
                    else emit32(sf|_construct_r_r_imm(STR32_REG, r1, SC1, 0));
                } else {
                    emit_imm(&buf->op1);
                    if (sf) emit32(sf|_construct_r_r_imm(STR64_REG, SC2, SC1, 0));
                    else emit32(sf|_construct_r_r_imm(STR32_REG, SC2, SC1, 0));
                }
            } else panic("ENCODER::UNHANDLED_MOV");
        } break;
        case LEA:{
            if (t1 == (MEM|IMM)) {
                int32_t offset = get_gp() + buf->op1.imm;
                if (is_external_offset(offset)) {
                    emit_rip(r0, offset);
                } else {
                    if (buf->op1.imm > INT16_MAX || buf->op1.imm < INT16_MIN) panic("ENCODER::ILLEGAL_OFFSET");
                    emit_brk(cache_patch_point(LEA, r0, buf->op1.imm));
                    warning("ENCODER::ILLEGAL_RIP");
                }
            } else if (t1&MEM) {
                emit_address_decode(&buf->op1);
                emit_add_imm(r0, SC1, 0);
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
            } else if (t1&MEM) {
                emit_address_decode(&buf->op1);
                emit_ldr_reg(SC1, SC1, 0);
                emit32(sf|_construct_r_r_r(SUB_REG|S, XZR, r0, SC1));
            } else if (t0&MEM) {
                emit_address_decode(&buf->op0);
                emit_ldr_reg(SC1, SC1, 0);
                if (t1 == IMM) {
                    emit_movz(SC2, buf->op1.imm, 0);
                    emit32(sf|_construct_r_r_r(SUB_REG|S, XZR, SC1, SC2));
                } else emit32(sf|_construct_r_r_r(SUB_REG|S, XZR, SC1, r1));
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
                    emit_and_imm(r0, r1, 7995);
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
            } else if (t0 == IMM) {
                emit_movz(SC1, buf->op0.imm, 0);
                emit_push_reg(SC1);
            } else if (t0&MEM) {
                emit_address_decode(&buf->op0);
                emit_ldr_reg(SC1, SC1, 0);
                emit_push_reg(SC1);
            } else panic("ENCODER::UNHANDLED_PUSH");
        } break;
        case LEAVE: {
            emit_add_imm(RSP, RBP, 0);
            emit_pop_reg(RBP);
        } break;
        case CLTQ: {
            emit32(SXTW_REG | (x64_regs[r0] << 5) | x64_regs[r0]);
        } break;
        case JG:
        case JGE:
        case JL:
        case JLE:
        case JNE:
        case JE:{
            emit_brk(cache_patch_point(buf->type, 0, buf->op0.imm));
        } break;
        case JMP:{
            emit_branch(buf, BR_REG, JMP);
        } break;
        case CALL:{
            emit32(0xf81f8f9e);
            emit_branch(buf, BLR_REG, CALL);
            emit_add_imm(RAX, RDI, 0);
            emit32(0xf840879e);
        } break;
        case RET: emit_ret(); break;
        case EBR: emit_bti(); break;
        case NOP: break;
        case MOVS: {
            sf = (buf->prefix == REPN) * MFT;
            if (t0 & MEM) {
                emit_address_decode(&buf->op0);
                emit32(sf|STR_NEON | (x64_regs[SC1]<<5) | r1);
            }else if (t1 & MEM) {
                emit_address_decode(&buf->op1);
                emit32(sf|LDR_NEON | (x64_regs[SC1]<<5) | r0);
            } else panic("ENCODER::UNHANDLED_MOVSS");
        } break;
        case MULS:
            emit_neon(buf, MUL_NEON);
            break;
        case DIVS:
            emit_neon(buf, DIV_NEON);
            break;
        case ADDS:
            emit_neon(buf, ADD_NEON);
            break;
        case SUBS:
            emit_neon(buf, SUB_NEON);
            break;
        case PXOR:
            if (t0 == (REG|XMM) && t1 == (REG|XMM)) {
                emit32(EOR_NEON|(r0)|(r0<<5)|(r1<<16));
            } else panic("ENCODER::UNHANDLED_PXOR");
            break;
        case CVTSD2SI:
            if (t0 == REG && t1 == (REG|XMM)) {
                emit32(FCVTZS_NEON | (x64_regs[r0]) | (r1 << 5));
            } else panic("ENCODER::UNHANDLED_CVTSD2SI");
            break;
        case CVTSS2SS:
            if (t0 == (REG|XMM) && t1 == (REG|XMM)) {
                emit32(FCVTD_NEON | (r0) | (r1 << 5));
            } else panic("ENCODER::UNHANDLED_CVTSS2SS");
            break;
        case CVTSS2SD:
            if (t1 & MEM) {
                emit_address_decode(&buf->op1);
                emit32(LDR_NEON | (16) | (x64_regs[SC1]<<5));
                emit32(FCVTU_NEON | (r0) | (16 << 5));
            } else panic("ENCODER::UNHANDLED_CVTSS2SD");
            break;
        case MOVQ:
            if (t1&XMM) emit32(FMOV_NEON | (x64_regs[r0]) | (r1 << 5));
            else emit32(FMOVR_NEON | (r0) | (x64_regs[r1] << 5));
            break;
        case MOVAPD:
            emit32(sf|MOV_NEON | (r0) | (r1 << 5));
            break;
        default:
            panic("ENCODER::UNKNOWN_INSTRUCTION: %x", buf->type);
    }
}