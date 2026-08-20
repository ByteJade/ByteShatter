#include "encoder.h"
#include "core.h"
#include "decoder.h"
#include "cache.h"
#include "arm64emitter.h"
#include <stdint.h>

#define SC1R 12
#define SC2R 13
/*
TODO:
- encoding for 8, 16 and 32 bit instructions
- clean emitter
*/
void emit_add_signed(Context* context, uint8_t r0, uint8_t r1, int64_t imm) {
    if (imm > 0)
        emit32(context, SF|ADD_IMM | (r0) | (r1<<5) | (imm<<10));
    else emit32(context, SF|SUB_IMM | (r0) | (r1<<5) | (-imm<<10));
}
void emit_sub_signed(Context* context, uint8_t r0, uint8_t r1, int64_t imm) {
    if (imm > 0)
        emit32(context, SF|SUB_IMM | (r0) | (r1<<5) | (imm<<10));
    else emit32(context, SF|ADD_IMM | (r0) | (r1<<5) | (-imm<<10));
}
void emit_address_decode(Context* context, Operand* op, uint8_t reg, uint8_t prefix) {
    uint8_t t = op->type;
    if (prefix==FS) {
        emit32(context, GET_FS | reg);
        emit_add_signed(context, reg, reg, op->imm);
        return;
    }
    if (t == (MEM|IMM)) {
        uint64_t full = (uint64_t)(context->guest + context->gp + op->imm);
        int64_t target = full & ~0xFFF;
        int64_t current = (uint64_t)(context->host + context->hp) & ~0xFFF;
        int64_t delta = (target - current) >> 12;
        if (delta < -4294967296LL || delta > 4294967296LL) {
            panic("ENCODER::TOO_LARGE_DISTANCE");
        }
        emit32(context, 0x90000000 | ((delta & 0x3) << 29) | (((delta >> 2) & 0x7FFFF) << 5) | reg);
        emit32(context, ((0x91000000) | ((full & 0xFFF) << 10) | (reg << 5) | reg));
        return;
    }
    if (t&IDX) {
        if (op->scale != 0) {
            emit32(context, 0xD3400000 | ((-(op->scale) & 0x3F) << 16) |
            (((63 - op->scale) & 0x3F) << 10) | (x64_regs[op->idx] << 5) | reg);
        } else emit32(context, SF|ADD_IMM | (reg) | (x64_regs[op->idx]<<5));
        if (t&REG) {
            emit32(context, SF|ADD_REG | (reg) | (reg<<5) | (x64_regs[op->reg]<<16));
        }
        if ((t&IMM) && op->imm != 0) {
            emit_add_signed(context, reg, reg, op->imm);
        }
    }else {
        if (t&IMM) {
            emit_add_signed(context, reg, x64_regs[op->reg], op->imm);
        } else {
            emit32(context, SF|ADD_IMM | (reg) | (x64_regs[op->reg]<<5));
        }
    }
}
void emit_branch(Context* context, Instruction* buf, uint32_t code, uint8_t type) {
    if (buf->a.type == REG) {
        emit32(context, code | (x64_regs[buf->a.reg] << 5));
    } else if (buf->a.type == IMM) {
        emit32(context, 0xD4200000 | (cache_patch_point(context, type, buf->a.imm) << 5));
    } else if (buf->a.type&MEM) {
        emit_address_decode(context, &buf->a, SC1R, 0);
        emit32(context, (LDR64_REG | (SC1R << 5) | SC1R));
        emit32(context, code | (SC1R << 5));
    }
}
void emit_imm(Context* context, int64_t imm, uint8_t rd) {
    if (imm >= 0 && imm <= INT16_MAX) {
        emit32(context, MOVZ_IMM | (imm << 5) | rd);
    } else if (imm < 0 && ~imm <= INT16_MAX) {
        emit32(context, 0x92800000 | (~imm << 5) | rd);
    }else if (imm >= INT16_MIN && imm <= INT32_MAX) {
        emit32(context, MOVZ_IMM | ((imm & 0xFFFF) << 5) | rd);
        emit32(context, MOVK_IMM | (1 << 21) | (((imm >> 16) & 0xFFFF) << 5) | rd);
        if (imm < 0) {
            emit32(context, SXTW_REG | (rd << 5) | rd);
        }
    } else {
        emit32(context, MOVZ_IMM | ((imm & 0xFFFF) << 5) | rd);
        emit32(context, MOVK_IMM | (1 << 21) | (((imm >> 16) & 0xFFFF) << 5) | rd);
        emit32(context, MOVK_IMM | (2 << 21) | (((imm >> 32) & 0xFFFF) << 5) | rd);
        emit32(context, MOVK_IMM | (3 << 21) | (((imm >> 48) & 0xFFFF) << 5) | rd);
    }
}
void emit_neon(Context* context, Instruction* buf, int opcode) {
    uint8_t r0 = buf->a.reg;
    uint8_t r1 = buf->b.reg;
    uint32_t osf = (buf->prefix == REPN) * FT;
    uint32_t msf = (buf->prefix == REPN) * MFT;
    if (buf->a.type & MEM) {
        emit_address_decode(context, &buf->a, SC1R, buf->prefix);
        emit32(context, msf|LDR_NEON | (SC1R<<5) | 16);
        r0 = 16;
    } else if (buf->b.type & MEM) {
        emit_address_decode(context, &buf->b,  SC1R, buf->prefix);
        emit32(context, msf|LDR_NEON | (SC1R<<5) | 16);
        r1 = 16;
    }
    emit32(context, osf|opcode|(r0)|(r0<<5)|(r1<<16));
    if (buf->a.type & MEM) emit32(context, msf|STR_NEON | (SC1R<<5) | 16);
}
int emit_load(Context* context, uint8_t rd, Operand* op, uint32_t sf, uint8_t prefix) {
    sf >>= 1;
    if (op->type == (MEM|REG|IMM) &&
        op->imm > -256 &&
        op->imm < 255) {
        emit32(context, sf|LDUR|((op->imm&0x1FF)<<12)|(x64_regs[op->reg]<<5)|(rd));
        return 1;
    } else if (op->type == (MEM|REG)) {
        emit32(context, sf|LDR32_REG|(x64_regs[op->reg]<<5)|rd);
        return 1;
    } else {
        emit_address_decode(context, op, SC1R, prefix);
        emit32(context, sf|LDR32_REG|(SC1R<<5)|rd);
        return 0;
    }
}
int emit_store(Context* context, uint8_t rd, Operand* op, uint32_t sf, uint8_t prefix, uint8_t address) {
    sf >>= 1;
    if (op->type == (MEM|REG|IMM) &&
        op->imm > -256 &&
        op->imm < 255) {
        emit32(context, sf|STUR|((op->imm&0x1FF)<<12)|(x64_regs[op->reg]<<5)|(rd));
        return 1;
    } else {
        if (address) emit_address_decode(context, op, SC1R, prefix);
        emit32(context, sf|STR32_REG|(SC1R<<5)|rd);
        return 0;
    }
}
void encode8bit(Context* context, Instruction* buf) {
    uint8_t r0 = buf->a.reg;
    uint8_t r1 = buf->b.reg;
    uint8_t t0 = buf->a.type;
    uint8_t t1 = buf->b.type;
    switch (buf->type) {
        case MOV: {
            if (t0&MEM) {
                emit_address_decode(context, &buf->a, SC1R, buf->prefix);
                if (t1 == REG){
                    emit32(context, _construct_r_r_imm(STR8_REG, r1, SC1, 0));
                } else {
                    emit_movz(SC2, buf->b.imm, 0);
                    emit32(context, _construct_r_r_imm(STR8_REG, SC2, SC1, 0));
                }
            } else panic("ENCODER::UNHANDLED_MOV");
        } break;
        case TEST:{
            if (t0 == REG && t1 == REG) {
                if (r0 == r1) {
                    emit32(context, 0x12001c00 | (x64_regs[r0]<<5) | (SC1R)); 
                    emit32(context, _construct_r_r_r(ANDS_REG, XZR, SC1, SC1));
                } else {
                    emit32(context, 0x12001c00 | (x64_regs[r0]<<5) | (SC1R)); 
                    emit32(context, 0x12001c00 | (x64_regs[r1]<<5) | (SC2R)); 
                    emit32(context, _construct_r_r_r(ANDS_REG, XZR, SC1, SC2));
                }
            } else if (t0 == REG && t1 == IMM) {
                emit32(context, 0x12001c00 | (x64_regs[r0]<<5) | (SC1R)); 
                emit_movz(SC2, buf->b.imm, 0);
                emit32(context, _construct_r_r_r(ANDS_REG, XZR, SC1, SC2));
            } else panic("ENCODER::UNHANDLED_TST");
        } break;
        case CMP:{
            if (t0 == REG) {
                emit32(context, _construct_r_r_r(SUB_IMM|S, XZR, r0, buf->b.imm));
            } else if (t0&MEM) {
                emit_address_decode(context, &buf->a, SC1R, buf->prefix);
                emit32(context, _construct_r_r_imm(LDR8_REG, SC1, SC1, 0));
                emit32(context, _construct_r_r_imm(SUB_IMM|S, XZR, SC1, buf->b.imm));
            } else panic("ENCODER::UNHANDLED_CMP");
        } break;
        case SETNE: {
            if (t0 == REG) {
                emit32(context, CSETNE | x64_regs[r0]);
            }  else panic("ENCODER::UNHANDLED_SETNE");
        } break;
        case SETAE: {
            if (t0 == REG) {
                emit32(context, CSETHS | x64_regs[r0]);
            }  else panic("ENCODER::UNHANDLED_SETE");
        } break;
        case SETE: {
            if (t0 == REG) {
                emit32(context, CSETEQ | x64_regs[r0]);
            }  else panic("ENCODER::UNHANDLED_SETE");
        } break;
        case SETB: {
            if (t0 == REG) {
                emit32(context, CSETLO | x64_regs[r0]);
            }  else panic("ENCODER::UNHANDLED_SETB");
        } break;
        default:
            panic("ENCODER::UNKNOWN_8BIT_INSTRUCTION: %x", buf->type);
    }
}
static int prev_instruction = NOP;
static int prev_register = NOP;

void encode(Context* context, Instruction* buf) {
    if (buf->size == 8) {
        encode8bit(context, buf);
        return;
    }
    uint8_t r0 = buf->a.reg;
    uint8_t r1 = buf->b.reg;
    uint8_t t0 = buf->a.type;
    uint8_t t1 = buf->b.type;
    uint32_t sf = (buf->size == 64) * SF;
    if (buf->type != PUSH && buf->type != POP) {
        prev_instruction = NOP;
    }
    switch (buf->type) {
        case SUB:{
            if (t1&MEM) {
                emit_load(context, SC2R, &buf->b, sf, buf->prefix);
                r1 = SC2;
                t1 = REG;
            }else if (t0&MEM) {
                emit_load(context, SC2R, &buf->a, sf, buf->prefix);
                r0 = SC2;
            }
            if (t1 == REG) {
                emit32(context, sf|_construct_r_r_r(SUB_REG|S, r0, r0, r1));
            } else {
                emit_sub_signed(context, x64_regs[r0], x64_regs[r0], buf->b.imm);
            }
            if (t0&MEM) {
                emit_store(context, SC2R, &buf->a, sf, buf->prefix, 0);
            }
        } break;
        case ADD:{
            if (t1&MEM) {
                emit_load(context, SC2R, &buf->b, sf, buf->prefix);
                r1 = SC2;
                t1 = REG;
            }else if (t0&MEM) {
                emit_load(context, SC2R, &buf->a, sf, buf->prefix);
                r0 = SC2;
            }
            r0 = x64_regs[r0];
            if (t1 == REG) {
                emit32(context, sf|ADD_REG|S | (r0) | (r0<<5) | (x64_regs[r1]<<16));
            } else {
                emit_add_signed(context, r0, r0, buf->b.imm);
            }
            if (t0&MEM) {
                emit_store(context, SC2R, &buf->a, sf, buf->prefix, 0);
            }
        } break;
        case SHL:{
            if (t0 == REG && t1 == IMM)
                emit_lsl_imm(r0, r0, buf->b.imm);
            else panic("ENCODER::UNHANDLED_SHL");  
        } break;
        case SHR:{
            if (t0 == REG && t1 == IMM)
                emit_lsr_imm(r0, r0, buf->b.imm);
            else panic("ENCODER::UNHANDLED_SHR");
        } break;
        case SAR:{
            if (t0 == REG && t1 == IMM)
                emit_asr_imm(r0, r0, buf->b.imm);
            else panic("ENCODER::UNHANDLED_SAR");
        } break;
        case MOVSX: {
            if (t0 == REG && t1 == REG) {
                emit32(context, 0x93407c00 | (x64_regs[r1]<<5) | (x64_regs[r0]));
            }else panic("ENCODER::UNHANDLED_MOVSLQ");
        } break;
        case MOVZX:
        case MOV:{
            if (t0 == REG && t1 == REG) {
                emit32(context, sf|ADD_IMM | x64_regs[r0] | (x64_regs[r1] << 5));
            }else if (t0 == REG && t1 == IMM){
                emit_imm(context, buf->b.imm, x64_regs[r0]);
            } else if (t1&MEM) {
                emit_load(context, x64_regs[r0], &buf->b, sf, buf->prefix);
            } else if (t0&MEM) {
                if (t1 == IMM){
                    if (buf->b.imm == 0) {
                        r1 = XZR;
                    } else {
                        emit_imm(context, buf->b.imm, SC2R);
                        r1 = SC2;
                    }
                }
                emit_store(context, x64_regs[r1], &buf->a, sf, buf->prefix, 1);
            } else panic("ENCODER::UNHANDLED_MOV");
        } break;
        case LEA:{
            if (t1&MEM) {
                emit_address_decode(context, &buf->b, x64_regs[r0], buf->prefix);
            } else panic("ENCODER::UNHANDLED_LEA");
        } break;
        case TEST:{
            if (t0 == REG && t1 == REG) {
                emit32(context, sf|_construct_r_r_r(ANDS_REG, XZR, r0, r1));
            } else panic("ENCODER::UNHANDLED_TST");
        } break;
        case CMP:{
            if (t0 == REG && t1 == REG) {
                emit32(context, sf|_construct_r_r_r(SUB_REG|S, XZR, r0, r1));
            } else if (t0 == REG && t1 == IMM) {
                emit_movz(SC2, buf->b.imm, 0);
                emit32(context, sf|_construct_r_r_r(SUB_REG|S, XZR, r0, SC2));
            } else if (t1&MEM) {
                emit_load(context, SC1R, &buf->b, sf, buf->prefix);
                emit32(context, sf|_construct_r_r_r(SUB_REG|S, XZR, r0, SC1));
            } else if (t0&MEM) {
                emit_load(context, SC1R, &buf->a, sf, buf->prefix);
                if (t1 == IMM) {
                    emit_movz(SC2, buf->b.imm, 0);
                    emit32(context, sf|_construct_r_r_r(SUB_REG|S, XZR, SC1, SC2));
                } else emit32(context, sf|_construct_r_r_r(SUB_REG|S, XZR, SC1, r1));
            } else panic("ENCODER::UNHANDLED_CMP");
        } break;
        case XOR:{
            if (t1 == IMM) {
                emit_imm(context, buf->b.imm, SC2R);
                r1 = SC2;
                t1 = REG;
            }
            if (t0 == REG && t1 == REG) {
                emit32(context, sf|_construct_r_r_r(EOR_REG, r0, r0, r1));
            } else panic("ENCODER::UNHANDLED_XOR");
        } break;
        case AND:{
            if (t1 == IMM) {
                emit_imm(context, buf->b.imm, SC2R);
                r1 = SC2;
                t1 = REG;
            }
            if (t0 == REG && t1 == REG) {
                emit32(context, sf|_construct_r_r_r(AND_REG, r0, r0, r1));
            } else panic("ENCODER::UNHANDLED_AND");
        } break;
        case POP:{
            if (t0 == REG) {
                if (r0 == RBP) {
                    emit32(context, POPP | (29<<10) | 30);
                    break;
                }
                if (prev_instruction == POP) {
                    prev_instruction = NOP;
                    patch(context, 3);
                    emit32(context, PUSHP | (x64_regs[r0]<<10) | (prev_register));
                } else {
                    prev_instruction = POP;
                    prev_register = x64_regs[r0];
                    emit32(context, POPP | x64_regs[r0]);
                }
            } else panic("ENCODER::UNHANDLED_POP");
        } break;
        case PUSH:{
            if (prev_instruction == PUSH) {
                patch(context, 3);
            }
            if (t0 == IMM) {
                emit_imm(context, buf->a.imm, SC1R);
                r0 = SC1;
            } else if (t0&MEM) {
                emit_load(context, SC1R,&buf->a, sf, buf->prefix);
                r0 = SC1;
            }else if (r0 == RBP) {
                emit32(context, PUSHP | (29<<10) | 30);
                emit32(context,SF|ADD_IMM | (31 << 5) | x64_regs[RSP]);
                break;
            }
            if (prev_instruction == PUSH) {
                prev_instruction = NOP;
                emit32(context, PUSHP | (prev_register<<10) | (x64_regs[r0]));
            } else {
                prev_instruction = PUSH;
                prev_register = x64_regs[r0];
                emit32(context, PUSHR | prev_register);
            }
        } break;
        case LEAVE: {
            emit_add_imm(RSP, RBP, 0);
            emit32(context, POPP | (29<<10) | 30);
        } break;
        case CLTQ: {
            emit32(context, SXTW_REG | (x64_regs[RAX] << 5) | x64_regs[RAX]);
        } break;
        case CLTD: {
            emit32(context, SXTW_REG | (x64_regs[RAX] << 5) | x64_regs[RAX]);
            emit32(context, 0x937ffd22); // asr x2, x9, #63
        } break;
        case IDIV: {
            emit_add_imm(SC1R, x64_regs[RAX], 0);
            emit32(context, 0x9ac00d89 | (x64_regs[r0]<<16)); // sdiv	x9, x12, r0
            emit32(context, 0x9b00b122 | (x64_regs[r0]<<16)); // msub	x2, x9, r0, x12
        } break;
        case IMUL: {
            emit32(context, SMUL_REG | (x64_regs[r0]<<16) | (x64_regs[r0]<<5) | (x64_regs[r1]));
        } break;
        case JG:
        case JGE:
        case JL:
        case JLE:
        case JNE:
        case JBE:
        case JAE:
        case JE:{
            emit_brk(cache_patch_point(context, buf->type, buf->a.imm));
        } break;
        case JMP:{
            emit_branch(context, buf, BR_REG, JMP);
        } break;
        case CALL:{
            emit_branch(context, buf, BLR_REG, CALL);
        } break;
        case RET: emit_ret(); break;
        case EBR: case NOP: break;
        case MOVS: {
            sf = (buf->prefix == REPN) * MFT;
            if (t0 & MEM) {
                emit_address_decode(context, &buf->a, SC1R, buf->prefix);
                emit32(context, sf|STR_NEON | (SC1R<<5) | r1);
            }else if (t1 & MEM) {
                emit_address_decode(context, &buf->b, SC1R, buf->prefix);
                emit32(context, sf|LDR_NEON | (SC1R<<5) | r0);
            } else panic("ENCODER::UNHANDLED_MOVSS");
        } break;
        case CMOVA: {
            if (t0 == REG && t1 == REG) {
                emit32(context, _construct_r_r_r(CSELHI, r0, r1, r0));
            }  else panic("ENCODER::UNHANDLED_CMOVA");
        } break;
        case MULS:
            emit_neon(context, buf, MUL_NEON);
            break;
        case DIVS:
            emit_neon(context, buf, DIV_NEON);
            break;
        case ADDS:
            emit_neon(context, buf, ADD_NEON);
            break;
        case SUBS:
            emit_neon(context, buf, SUB_NEON);
            break;
        case COMIS:
            emit_neon(context, buf, CMP_NEON);
            break;
        case PXOR:
            if (t0 == (REG|XMM) && t1 == (REG|XMM)) {
                emit32(context, EOR_NEON|(r0)|(r0<<5)|(r1<<16));
            } else panic("ENCODER::UNHANDLED_PXOR");
            break;
        case CVTSD2SI:
            if (t0 == REG && t1 == (REG|XMM)) {
                emit32(context, FCVTZS_NEON | (x64_regs[r0]) | (r1 << 5));
            } else panic("ENCODER::UNHANDLED_CVTSD2SI");
            break;
        case CVTSI2S: {
            uint32_t instr;
            if (buf->prefix == REPN) instr = SCVTD_NEON;
            else instr = SCVTF_NEON;
            if (t1&MEM) {
                emit_address_decode(context, &buf->b, SC1R, buf->prefix);
                emit32(context, _construct_r_r_imm(LDR32_REG, SC1, SC1, 0));
                emit32(context, instr | (r0) | (SC1R << 5));
            } else if (t1 == REG) {
                emit32(context, instr | (r0) | (x64_regs[r1]<<5));
            } else panic("ENCODER::UNHANDLED_CVTSI2SD");
        } break;
        case CVTSD2SS:
            if (t0 == (REG|XMM) && t1 == (REG|XMM)) {
                emit32(context, FCVT_NEON | (r0) | (r1 << 5));
            } else panic("ENCODER::UNHANDLED_CVTSS2SS");
            break;
        case CVTSS2SD:
            if (t0 == (REG|XMM) && t1 == (REG|XMM)) {
                emit32(context, FCVTU_NEON | (r0) | (r1 << 5));
            } else if (t1 & MEM) {
                emit_address_decode(context, &buf->b, SC1R, buf->prefix);
                emit32(context, LDR_NEON | (16) | (SC1R<<5));
                emit32(context, FCVTU_NEON | (r0) | (16 << 5));
            } else panic("ENCODER::UNHANDLED_CVTSS2SD");
            break;
        case MOVQ:
            if (t1&XMM) emit32(context, FMOV_NEON | (x64_regs[r0]) | (r1 << 5));
            else emit32(context, FMOVR_NEON | (r0) | (x64_regs[r1] << 5));
            break;
        case MOVAPD:
            emit32(context, sf|MOV_NEON | (r0) | (r1 << 5) | (r1 << 16));
            break;
        default:
            panic("ENCODER::UNKNOWN_INSTRUCTION: %i", buf->type);
    }
}