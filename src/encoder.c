#include "encoder.h"
#include "core.h"
#include "decoder.h"
#include "memory.h"
#include "cache.h"
#include "arm64emitter.h"
#include <stdint.h>

/*
TODO:
- encoding for 8, 16 and 32 bit instructions
- clean emitter
*/
void emit_add_signed(uint8_t r0, uint8_t r1, int64_t imm) {
    if (imm > 0)
        emit_add_imm(r0, r1, imm);
    else emit_sub_imm(r0, r1, -imm);
}
void emit_address_decode(Operand* op, uint8_t reg, uint8_t prefix) {
    uint8_t t = op->type;
    if (prefix==FS) {
        emit32(GET_FS | x64_regs[reg]);
        emit_add_signed(reg, reg, op->imm);
        return;
    }
    if (t == (MEM|IMM)) {
        uint64_t full = (uint64_t)(get_guest() + get_gp() + op->imm);
        int64_t target = full & ~0xFFF;
        int64_t current = (uint64_t)(get_host() + get_hp()) & ~0xFFF;
        int64_t delta = (target - current) >> 12;
        if (delta < -4294967296LL || delta > 4294967296LL) {
            panic("ENCODER::TOO_LARGE_DISTANCE");
        }
        emit_adrp(reg, delta);
        emit_add_imm(reg, reg, full & 0xFFF);
        return;
    }
    if (t&IDX) {
        emit32(_construct_r_r_imm(SF|ADD_IMM, reg, op->idx, 0));
        if (op->scale != 0) {
            emit_lsl_imm(reg, reg, op->scale);
        }
        if (t&REG) {
            emit32(_construct_r_r_r(SF|ADD_REG, reg, reg, op->reg));
        }
        if ((t&IMM) && op->imm != 0) {
            emit_add_signed(reg, reg, op->imm);
        }
    }else {
        if (t&IMM) {
            emit_add_signed(reg, op->reg, op->imm);
        } else {
            emit32(_construct_r_r_imm(SF|ADD_IMM, reg, op->reg, 0));
        }
    }
}
void emit_branch(Instruction* buf, uint32_t code, uint8_t type) {
    if (buf->a.type == REG) {
        emit32(code | (x64_regs[buf->a.reg] << 5));
    } else if (buf->a.type == IMM) {
        emit_brk(cache_patch_point(type, 0, buf->a.imm));
    } else if (buf->a.type&MEM) {
        emit_address_decode(&buf->a, SC1, 0);
        emit_ldr_reg(SC1, SC1, 0);
        emit32(code | (x64_regs[SC1] << 5));
    }
}
void emit_imm(int64_t imm, uint8_t rd) {
    if (imm >= 0) {
        if (imm <= INT16_MAX) emit_movz(rd, imm, 0);
        else {
            emit_mov32(rd, imm);
        }
    } else {
        if (~imm <= INT16_MAX) emit_movn(rd, ~imm, 0);
        else {
            emit_mov32(rd, imm);
            emit32(SXTW_REG | (x64_regs[rd] << 5) | x64_regs[rd]);
        }
    }
}
void emit_neon(Instruction* buf, int opcode) {
    uint8_t r0 = buf->a.reg;
    uint8_t r1 = buf->b.reg;
    uint32_t osf = (buf->prefix == REPN) * FT;
    uint32_t msf = (buf->prefix == REPN) * MFT;
    if (buf->a.type & MEM) {
        emit_address_decode(&buf->a, SC1, buf->prefix);
        emit32(msf|LDR_NEON | (x64_regs[SC1]<<5) | 16);
        r0 = 16;
    } else if (buf->b.type & MEM) {
        emit_address_decode(&buf->b,  SC1, buf->prefix);
        emit32(msf|LDR_NEON | (x64_regs[SC1]<<5) | 16);
        r1 = 16;
    }
    emit32(osf|opcode|(r0)|(r0<<5)|(r1<<16));
    if (buf->a.type & MEM) emit32(msf|STR_NEON | (x64_regs[SC1]<<5) | 16);
}
int emit_load(uint8_t rd, Operand* op, uint32_t sf, uint8_t prefix) {
    sf >>= 1;
    if (op->type == (MEM|REG|IMM) &&
        op->imm > -256 &&
        op->imm < 255) {
        emit32(sf|LDUR|((op->imm&0x1FF)<<12)|(x64_regs[op->reg]<<5)|(rd));
        return 1;
    } else if (op->type == (MEM|REG)) {
        emit32(sf|LDR32_REG|(x64_regs[op->reg]<<5)|rd);
        return 1;
    } else {
        emit_address_decode(op, SC1, prefix);
        emit32(sf|LDR32_REG|(x64_regs[SC1]<<5)|rd);
        return 0;
    }
}
int emit_store(uint8_t rd, Operand* op, uint32_t sf, uint8_t prefix, uint8_t address) {
    sf >>= 1;
    if (op->type == (MEM|REG|IMM) &&
        op->imm > -256 &&
        op->imm < 255) {
        emit32(sf|STUR|((op->imm&0x1FF)<<12)|(x64_regs[op->reg]<<5)|(rd));
        return 1;
    } else {
        if (address) emit_address_decode(op, SC1, prefix);
        emit32(sf|STR32_REG|(x64_regs[SC1]<<5)|rd);
        return 0;
    }
}
void encode8bit(Instruction* buf) {
    uint8_t r0 = buf->a.reg;
    uint8_t r1 = buf->b.reg;
    uint8_t t0 = buf->a.type;
    uint8_t t1 = buf->b.type;
    switch (buf->type) {
        case MOV: {
            if (t0&MEM) {
                emit_address_decode(&buf->a, SC1, buf->prefix);
                if (t1 == REG){
                    emit32(_construct_r_r_imm(STR8_REG, r1, SC1, 0));
                } else {
                    emit_movz(SC2, buf->b.imm, 0);
                    emit32(_construct_r_r_imm(STR8_REG, SC2, SC1, 0));
                }
            } else panic("ENCODER::UNHANDLED_MOV");
        } break;
        case TEST:{
            if (t0 == REG && t1 == REG) {
                emit32(0x12001c00 | (x64_regs[r0]<<5) | (x64_regs[SC1])); 
                emit32(0x12001c00 | (x64_regs[r1]<<5) | (x64_regs[SC2])); 
                emit32(_construct_r_r_r(ANDS_REG, XZR, SC1, SC2));
            } else if (t0 == REG && t1 == IMM) {
                emit32(0x12001c00 | (x64_regs[r0]<<5) | (x64_regs[SC1])); 
                emit_movz(SC2, buf->b.imm, 0);
                emit32(_construct_r_r_r(ANDS_REG, XZR, SC1, SC2));
            } else panic("ENCODER::UNHANDLED_TST");
        } break;
        case CMP:{
            if (t0 == REG) {
                emit32(_construct_r_r_r(SUB_IMM|S, XZR, r0, buf->b.imm));
            } else if (t0&MEM) {
                emit_address_decode(&buf->a, SC1, buf->prefix);
                emit32(_construct_r_r_imm(LDR8_REG, SC1, SC1, 0));
                emit32(_construct_r_r_imm(SUB_IMM|S, XZR, SC1, buf->b.imm));
            } else panic("ENCODER::UNHANDLED_CMP");
        } break;
        default:
            panic("ENCODER::UNKNOWN_8BIT_INSTRUCTION: %x", buf->type);
    }
}
static int prev_instruction = NOP;
static int prev_register = NOP;

void encode(Instruction* buf) {
    if (buf->size == 8) {
        encode8bit(buf);
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
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_r(SUB_REG|S, r0, r0, r1));
            } else if (t0 == REG && t1 == IMM) {
                emit32(sf|_construct_r_r_imm(SUB_IMM|S, r0, r0, buf->b.imm&IMM12));
            } else if (t1&MEM) {
                emit_load(x64_regs[SC2], &buf->b, sf, buf->prefix);
                emit32(sf|_construct_r_r_r(SUB_REG|S, r0, r0, SC2));
            } else panic("ENCODER::UNHANDLED_SUB");
        } break;
        case ADD:{
            if (t1&MEM) {
                emit_load(x64_regs[SC2], &buf->b, sf, buf->prefix);
                r1 = SC2;
            }else if (t0&MEM) {
                emit_load(x64_regs[SC2], &buf->a, sf, buf->prefix);
                r0 = SC2;
            }
            if (t1 == REG) {
                emit32(sf|_construct_r_r_r(ADD_REG|S, r0, r0, r1));
            } else {
                emit_add_signed(r0, r0, buf->b.imm);
            }
            if (t0&MEM) {
                emit_store(x64_regs[SC2], &buf->a, sf, buf->prefix, 0);
            }
        } break;
        case SHL:{
            if (t0 == REG && t1 == IMM)
                emit_lsl_imm(r0, r1, buf->b.imm);
            else panic("ENCODER::UNHANDLED_SHL");  
        } break;
        case SHR:{
            if (t0 == REG && t1 == IMM)
                emit_lsr_imm(r0, r1, buf->b.imm);
            else panic("ENCODER::UNHANDLED_SHR");
            break;
        }
        case SAR:{
            if (t0 == REG && t1 == IMM)
                emit_asr_imm(r0, r1, buf->b.imm);
            else panic("ENCODER::UNHANDLED_SAR");
        } break;
        case MOVSX: {
            if (t0 == REG && t1 == REG) {
                emit32(0x93407c00 | (x64_regs[r0]<<5) | (x64_regs[r1]));
            }else panic("ENCODER::UNHANDLED_MOVSLQ");
        } break;
        case MOVZX:
        case MOV:{
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_imm(ADD_IMM, r0, r1, 0));
            }else if (t0 == REG && t1 == IMM){
                emit_imm(buf->b.imm, r0);
            } else if (t1&MEM) {
                emit_load(x64_regs[r0], &buf->b, sf, buf->prefix);
            } else if (t0&MEM) {
                if (t1 == IMM){
                    if (buf->b.imm == 0) {
                        r1 = XZR;
                    } else {
                        emit_imm(buf->b.imm, SC2);
                        r1 = SC2;
                    }
                }
                emit_store(x64_regs[r1], &buf->a, sf, buf->prefix, 1);
            } else panic("ENCODER::UNHANDLED_MOV");
        } break;
        case LEA:{
            if (t1&MEM) {
                emit_address_decode(&buf->b, r0, buf->prefix);
            } else panic("ENCODER::UNHANDLED_LEA");
        } break;
        case TEST:{
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_r(ANDS_REG, XZR, r0, r1));
            } else panic("ENCODER::UNHANDLED_TST");
        } break;
        case CMP:{
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_r(SUB_REG|S, XZR, r0, r1));
            } else if (t0 == REG && t1 == IMM) {
                emit_movz(SC2, buf->b.imm, 0);
                emit32(sf|_construct_r_r_r(SUB_REG|S, XZR, r0, SC2));
            } else if (t1&MEM) {
                emit_load(x64_regs[SC1], &buf->b, sf, buf->prefix);
                emit32(sf|_construct_r_r_r(SUB_REG|S, XZR, r0, SC1));
            } else if (t0&MEM) {
                emit_load(x64_regs[SC1], &buf->a, sf, buf->prefix);
                if (t1 == IMM) {
                    emit_movz(SC2, buf->b.imm, 0);
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
            if (t1 == IMM) {
                emit_imm(buf->b.imm, SC2);
                r1 = SC2;
                t1 = REG;
            }
            if (t0 == REG && t1 == REG) {
                emit32(sf|_construct_r_r_r(AND_REG, r0, r0, r1));
            } else panic("ENCODER::UNHANDLED_AND");
        } break;
        case POP:{
            if (t0 == REG) {
                if (r0 == RBP) {
                    emit32(POPP | (29<<10) | 30);
                    break;
                }
                if (prev_instruction == POP) {
                    patch32();
                    emit32(POPP | (x64_regs[r0]<<10) | (x64_regs[prev_register]));
                    cache_back();
                    prev_instruction = NOP;
                } else {
                    emit_pop_reg(r0);
                    prev_instruction = POP;
                    prev_register = r0;
                }
            } else panic("ENCODER::UNHANDLED_POP");
        } break;
        case PUSH:{
            if (t0 == REG) {
                if (r0 == RBP) {
                    emit32(PUSHP | (29<<10) | 30);
                    break;
                }
                if (prev_instruction == PUSH) {
                    patch32();
                    emit32(PUSHP | (x64_regs[r0]<<10) | (x64_regs[prev_register]));
                    cache_back();
                    prev_instruction = NOP;
                } else {
                    emit_push_reg(r0);
                    prev_instruction = PUSH;
                    prev_register = r0;
                }
            } else if (t0 == IMM) {
                emit_movz(SC1, buf->a.imm, 0);
                emit_push_reg(SC1);
                prev_instruction = NOP;
            } else if (t0&MEM) {
                emit_load(x64_regs[SC1],&buf->a, sf, buf->prefix);
                emit_push_reg(SC1);
                prev_instruction = NOP;
            } else panic("ENCODER::UNHANDLED_PUSH");
        } break;
        case LEAVE: {
            emit_add_imm(RSP, RBP, 0);
            emit32(POPP | (29<<10) | 30);
        } break;
        case CLTQ: {
            emit32(SXTW_REG | (x64_regs[RAX] << 5) | x64_regs[RAX]);
        } break;
        case CLTD: {
            emit32(SXTW_REG | (x64_regs[RAX] << 5) | x64_regs[RAX]);
            emit32(0x937ffd22); // asr x2, x9, #63
        } break;
        case IDIV: {
            emit_add_imm(x64_regs[SC1], x64_regs[RAX], 0);
            emit32(0x9ac00d89 | (x64_regs[r0]<<16)); // sdiv	x9, x12, r0
            emit32(0x9b00b122 | (x64_regs[r0]<<16)); // msub	x2, x9, r0, x12
        } break;
        case JG:
        case JGE:
        case JL:
        case JLE:
        case JNE:
        case JBE:
        case JAE:
        case JE:{
            emit_brk(cache_patch_point(buf->type, 0, buf->a.imm));
        } break;
        case JMP:{
            emit_branch(buf, BR_REG, JMP);
        } break;
        case CALL:{
            emit_branch(buf, BLR_REG, CALL);
        } break;
        case RET: emit_ret(); break;
        case EBR: case NOP: break;
        case MOVS: {
            sf = (buf->prefix == REPN) * MFT;
            if (t0 & MEM) {
                emit_address_decode(&buf->a, SC1, buf->prefix);
                emit32(sf|STR_NEON | (x64_regs[SC1]<<5) | r1);
            }else if (t1 & MEM) {
                emit_address_decode(&buf->b, SC1, buf->prefix);
                emit32(sf|LDR_NEON | (x64_regs[SC1]<<5) | r0);
            } else panic("ENCODER::UNHANDLED_MOVSS");
        } break;
        case CMOVA: {
            if (t0 == REG && t1 == REG) {
                emit32(_construct_r_r_r(CSELHI, r0, r1, r0));
            }  else panic("ENCODER::UNHANDLED_CMOVA");
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
        case COMIS:
            emit_neon(buf, CMP_NEON);
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
        case CVTSI2S: {
            uint32_t instr;
            if (buf->prefix == REPN) instr = SCVTD_NEON;
            else instr = SCVTF_NEON;
            if (t1&MEM) {
                emit_address_decode(&buf->b, SC1, buf->prefix);
                emit32(_construct_r_r_imm(LDR32_REG, SC1, SC1, 0));
                emit32(instr | (r0) | (x64_regs[SC1] << 5));
            } else if (t1 == REG) {
                emit32(instr | (r0) | (x64_regs[r1]<<5));
            } else panic("ENCODER::UNHANDLED_CVTSI2SD");
        } break;
        case CVTSD2SS:
            if (t0 == (REG|XMM) && t1 == (REG|XMM)) {
                emit32(FCVT_NEON | (r0) | (r1 << 5));
            } else panic("ENCODER::UNHANDLED_CVTSS2SS");
            break;
        case CVTSS2SD:
            if (t0 == (REG|XMM) && t1 == (REG|XMM)) {
                emit32(FCVTU_NEON | (r0) | (r1 << 5));
            } else if (t1 & MEM) {
                emit_address_decode(&buf->b, SC1, buf->prefix);
                emit32(LDR_NEON | (16) | (x64_regs[SC1]<<5));
                emit32(FCVTU_NEON | (r0) | (16 << 5));
            } else panic("ENCODER::UNHANDLED_CVTSS2SD");
            break;
        case MOVQ:
            if (t1&XMM) emit32(FMOV_NEON | (x64_regs[r0]) | (r1 << 5));
            else emit32(FMOVR_NEON | (r0) | (x64_regs[r1] << 5));
            break;
        case MOVAPD:
            emit32(sf|MOV_NEON | (r0) | (r1 << 5) | (r1 << 16));
            break;
        default:
            panic("ENCODER::UNKNOWN_INSTRUCTION: %x", buf->type);
    }
}