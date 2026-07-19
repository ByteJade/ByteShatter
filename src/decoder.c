#include "decoder.h"
#include "armdef.h"
#include "debugger.h"
#include "memory.h"
#include "encoder.h"
#include "cache.h"
#include "core.h"
#include <stdint.h>
#include <stdio.h>

int64_t fetch_imm8(void) {
    return (int64_t)(int8_t)fetch8();
}
int64_t fetch_imm32(void) {
    return (int64_t)(int32_t)fetch32();
}
void decode_sib(Operand* op) {
    uint8_t sib = fetch8();
    uint8_t base = sib&7;
    op->type |= IDX;
    op->scale = sib>>6;
    op->idx = (sib>>3)&7;
    if (base == 5 && (op->type&IMM) == 0) {
        op->type |= IMM;
        op->imm = fetch_imm32();
    } else {
        op->type |= REG;
        op->reg = base;
    }
}
void decode_rm(Operand* op, uint8_t modrm) {
    uint8_t mod = modrm >> 6;
    uint8_t rm = modrm & 7;
    if (mod < 3) {
        op->type = MEM;
        if (mod == 1 || mod == 2) {
            op->type |= IMM;
        }
        if (mod == 0 && rm == 5) {
            op->type |= IMM;
            op->imm = fetch_imm32();
        } else if (rm == 4) {
            decode_sib(op);
        } else {
            op->type |= REG;
            op->reg = rm;
        }
        if (mod == 1) {
            op->imm = fetch_imm8();
        } else if (mod == 2) {
            op->imm = fetch_imm32();
        }
    } else {
        op->type = REG;
        op->reg = rm;
    }
}
void decode_regrm(X64_instruction* buf) {
    uint8_t byte = fetch8();
    buf->op0.type = REG;
    buf->op0.reg = (byte >> 3)&7;
    decode_rm(&buf->op1, byte);
}
void decode_shift_table(X64_instruction* buf, uint8_t modrm) {
    uint8_t shift = (modrm >> 3)&7;
    switch (shift) {
        case 4: buf->type = SHL; break;
        case 5: buf->type = SHR; break;
        case 7: buf->type = SAR; break;
        default: panic("DECODER::UNKNOWN_SHIFT_SYMBOL: %X", shift);
    }
}
void decode_grp1(X64_instruction* buf, uint8_t modrm) {
    uint8_t shift = (modrm >> 3)&7;
    switch (shift) {
        case 0: buf->type = ADD; break;
        //case 1: break; // or
        //case 2: break; // adc
        //case 3: break; // sbb
        case 4: buf->type = AND; break;
        case 5: buf->type = SUB; break;
        case 6: buf->type = XOR; break;
        case 7: buf->type = CMP; break;
    default: panic("DECODER::UNKNOWN_GRP1: %X", shift);
    }
}
void print_op(char** ptr, X64_instruction* buf, Operand* op) {
    char* out = *ptr;
    if (op->type == REG) {
        if (buf->size == 64) {
            out += sprintf(out, "r%s ", regs[op->reg]);
        } else if (buf->size == 32) {
            out += sprintf(out, "e%s ", regs[op->reg]);
        } else {
            out += sprintf(out, "%s ", regs[op->reg]);
        }
    } else if (op->type == (REG | XMM)) {
        out += sprintf(out, "xmm%i ", op->reg);
    } else if (op->type == IMM) {
        out += sprintf(out, "%lx ", op->imm);
    } else {
        out += sprintf(out, "[ ");
        if (op->type&REG) {
            out += sprintf(out, "r%s ", regs[op->reg]);
            if (op->type&IDX) out += sprintf(out, "+ ");
        }
        if (op->type&IDX) {
            out += sprintf(out, "r%s ", regs[op->idx]);
            if (op->scale == 1) {
                out += sprintf(out, "* 2 ");
            } else if (op->scale == 2) {
                out += sprintf(out, "* 4 ");
            } else if (op->scale == 3) {
                out += sprintf(out, "* 8 ");
            }
        }
        if (op->type&IMM) {
            if (op->type == (MEM|IMM)) {
                out += sprintf(out, "rip ");
            }
            if (op->imm > 0) out += sprintf(out, "+ %lx ", op->imm);
            if (op->imm < 0) out += sprintf(out, "- %lx ", -op->imm);
        }
        out += sprintf(out, "] ");
    }
    *ptr = out;
}
void sprint_instr(char* out, X64_instruction* buf) {
    char prefix = ' ';
    if (buf->prefix == REP) prefix = 's';
    if (buf->prefix == REPN) prefix = 'd';
    out += sprintf(out, "\033[34m%s%c \033[32m",
        types[buf->type], prefix);
    if (buf->opcount > 0)
        print_op(&out, buf, &buf->op0);
    if (buf->opcount > 1)
        print_op(&out, buf, &buf->op1);
    out += sprintf(out, "\033[0m");
}
int decode_instr(X64_instruction* buf) {
    int ret = 0;
    buf->reverse = 0;
    uint8_t rex = 0;
    uint8_t byte = fetch8();
    buf->size = 32;
    buf->prefix = 0;
    if (byte >> 4 == 0x4) {
        rex = byte & 0xF;
        byte = fetch8();
    }
    if (byte == P66) {
        buf->prefix = P66;
        byte = fetch8();
    } if (byte == REP) {
        buf->prefix = REP;
        byte = fetch8();
    } else if (byte == REPN) {
        buf->prefix = REPN;
        byte = fetch8();
    } else if (byte == 0x64) {
        buf->prefix = TLS;
        byte = fetch8();
    }
    switch (byte) {
        case 0x00: 
            buf->opcount = 0;
            buf->type = NOP;
            break;
        case 0x01:
            buf->reverse = 1;
            buf->opcount = 2;
            buf->type = ADD;
            decode_regrm(buf);
            break;
        case 0x0F: {
            if (buf->prefix) {
                decode_0F(buf);
                break;
            }
            buf->opcount = 1;
            uint8_t modrm = fetch8();
            switch (modrm) {
                case 0x28:
                    buf->type = MOVAPD;
                    goto done;
                case 0x57:
                    buf->type = PXOR;
                done:
                    buf->opcount = 2;
                    decode_regrm(buf);
                    buf->op0.type |= XMM;
                    if (buf->op1.type == REG)
                        buf->op1.type |= XMM;
                    break;
                case 0x84:
                    buf->type = JE;
                    buf->op0.type = IMM;
                    buf->op0.imm = fetch_imm32();
                    break;
                case 0x85:
                    buf->type = JNE;
                    buf->op0.type = IMM;
                    buf->op0.imm = fetch_imm32();
                    break;
                case 0x8C:
                    buf->type = JL;
                    buf->op0.type = IMM;
                    buf->op0.imm = fetch_imm32();
                    break;
                case 0x8D:
                    buf->type = JGE;
                    buf->op0.type = IMM;
                    buf->op0.imm = fetch_imm32();
                    break;
                case 0x8E:
                    buf->type = JLE;
                    buf->op0.type = IMM;
                    buf->op0.imm = fetch_imm32();
                    break;
                case 0x8F:
                    buf->type = JG;
                    buf->op0.type = IMM;
                    buf->op0.imm = fetch_imm32();
                    break;
                case 0xB6:
                    buf->opcount = 2;
                    buf->type = MOVZX;
                    decode_regrm(buf);
                    break;
                default:
                    panic("DECODER::UNKNOWN_0F_SYMBOL: %X", modrm);
            }
        } break;
        case 0x29:
            buf->reverse = 1;
        case 0x2B:
            buf->opcount = 2;
            buf->type = SUB;
            decode_regrm(buf);
            break;
        case 0x31:
            buf->opcount = 2;
            buf->type = XOR;
            decode_regrm(buf);
            break;
        case 0x39:
            buf->reverse = 1;
        case 0x3B:
            buf->opcount = 2;
            buf->type = CMP;
            decode_regrm(buf);
            break;
        case 0x3C:
            buf->size = 8;
            buf->opcount = 2;
            buf->type = CMP;
            buf->op0.type = REG;
            buf->op0.reg = byte - 0x3C;
            buf->op1.type = IMM;
            buf->op1.imm = fetch8();
            break;
        case 0x50 ... 0x57:
            buf->reverse = 1;
            buf->size = 64;
            buf->opcount = 1;
            buf->type = PUSH;
            buf->op1.type = REG;
            buf->op1.reg = byte - 0x50;
            break;
        case 0x58 ... 0x5F:
            buf->size = 64;
            buf->opcount = 1;
            buf->type = POP;
            buf->op0.type = REG;
            buf->op0.reg = byte - 0x58;
            break;
        case 0x63:
            buf->opcount = 2;
            buf->type = MOVSLQ;
            decode_regrm(buf);
            break;
        case 0x68:
            buf->opcount = 1;
            buf->type = PUSH;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm32();
            break;
        case 0x6A:
            buf->opcount = 1;
            buf->type = PUSH;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            break;
        case 0x74:
            buf->opcount = 1;
            buf->type = JE;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            break;
        case 0x75:
            buf->opcount = 1;
            buf->type = JNE;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            break;
        case 0x7C:
            buf->opcount = 1;
            buf->type = JL;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            break;
        case 0x7D:
            buf->opcount = 1;
            buf->type = JGE;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            break;
        case 0x7E:
            buf->opcount = 1;
            buf->type = JLE;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            break;
        case 0x7F:
            buf->opcount = 1;
            buf->type = JG;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            break;
        case 0x80:
            buf->size = 8;
            buf->opcount = 2;
            buf->type = CMP;
            decode_rm(&buf->op0, fetch8());
            buf->op1.type = IMM;
            buf->op1.imm = fetch8();
            break;
        case 0x81: {
            buf->opcount = 2;
            uint8_t modrm = fetch8();
            decode_rm(&buf->op0, modrm);
            buf->op1.type = IMM;
            buf->op1.imm = fetch_imm32();
            decode_grp1(buf, modrm);
        } break;
        case 0x83: {
            buf->reverse = 1;
            buf->opcount = 2;
            uint8_t modrm = fetch8();
            decode_rm(&buf->op1, modrm);
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            decode_grp1(buf, modrm);
        } break;
        case 0x84:
            buf->size = 8;
            buf->opcount = 2;
            buf->type = TST;
            decode_regrm(buf);
            break;
        case 0x85:
            buf->opcount = 2;
            buf->type = TST;
            decode_regrm(buf);
            break;
        case 0x89:
            buf->reverse = 1;
        case 0x8B:
            buf->opcount = 2;
            buf->type = MOV;
            decode_regrm(buf);
            break;
        case 0x8D:
            buf->opcount = 2;
            buf->type = LEA;
            decode_regrm(buf);
            break;
        case 0x90: 
            buf->opcount = 0;
            buf->type = NOP;
            break;
        case 0x98:
            buf->opcount = 0;
            buf->type = CLTQ;
            break;
        case 0x99:
            buf->opcount = 0;
            buf->type = CLTD;
            break;
        case 0xB8 ... 0xBF:
            buf->reverse = 1;
            buf->opcount = 2;
            buf->type = MOV;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm32();
            buf->op1.type = REG;
            buf->op1.reg = byte - 0xB8;
            break;
        case 0xE8:
            buf->opcount = 1;
            buf->type = CALL;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm32();
            break;
        case 0xE9:
            buf->opcount = 1;
            buf->type = JMP;
            ret = JMP;
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm32();
            break;
        case 0xEb:
            buf->opcount = 1;
            buf->type = JMP;
            ret = JMP;
            buf->op0.type = IMM;
            buf->op0.imm = fetch8();
            break;
        case 0xD1:{
            buf->reverse = 1;
            buf->opcount = 2;
            uint8_t modrm = fetch8();
            decode_rm(&buf->op1, modrm);
            buf->op0.type = IMM;
            buf->op0.imm = 1;
            decode_shift_table(buf, modrm);
        } break;
        case 0xC1:{
            buf->reverse = 1;
            buf->opcount = 2;
            uint8_t modrm = fetch8();
            decode_rm(&buf->op1, modrm);
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            decode_shift_table(buf, modrm);
        } break;
        case 0xC6:
            buf->reverse = 1;
            buf->size = 8;
            buf->opcount = 2;
            buf->type = MOV;
            decode_rm(&buf->op1, fetch8());
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            break;
        case 0xC7:
            buf->reverse = 1;
            buf->opcount = 2;
            buf->type = MOV;
            decode_rm(&buf->op1, fetch8());
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm32();
            break;
        case 0xC9:
            buf->opcount = 0;
            buf->type = LEAVE;
            break;
        case 0xC3:
        case 0xF4: // actualy hlt
            buf->opcount = 0;
            buf->type = RET;
            ret = RET;
            break;
        case 0xF7: {
            uint8_t byte = fetch8();
            if ((byte&0b11111000) == 0xF8) {
                buf->opcount = 1;
                buf->type = IDIV;
                buf->op0.type = REG;
                buf->op0.reg = byte&0x7;
            } else panic("Unhandled F7");
        } break;
        case 0xff: {
            buf->size = 64;
            buf->reverse = 1;
            buf->opcount = 1;
            uint8_t modrm = fetch8();
            decode_rm(&buf->op1, modrm);
            switch ((modrm >> 3)&7) {
                case 0:
                    buf->opcount = 2;
                    buf->type = ADD;
                    buf->op0.type = IMM;
                    buf->op0.imm = 1;
                    break; // inc
                case 1:
                    buf->opcount = 2;
                    buf->type = SUB;
                    buf->op0.type = IMM;
                    buf->op0.imm = 1;
                    break; // dec
                case 2: case 3:
                    buf->type = CALL; break; // call
                case 4: case 5:
                    buf->type = JMP; ret = JMP; break; // jmps
                case 6: buf->type = PUSH; break; // push
            }
        } break;
        default:
            panic("DECODER::UNKNOWN_SYMBOL: %X", byte);
    }
    if (rex) {
        if (rex&8) buf->size = 64;
        if (rex&4) buf->op0.reg += 8;
        if (rex&2) buf->op1.idx += 8;
        if (rex&1) buf->op1.reg += 8;
    }
    if (buf->reverse) {
        Operand tmp = buf->op0;
        buf->op0 = buf->op1;
        buf->op1 = tmp;
    }
    return ret;
}
int decode_step() {
    uint8_t brk = 0;
    if (get_gp() == debug_breakp()) brk = 1;
    cache_block_point();
    X64_instruction buf;
    int type = decode_instr(&buf);
    char out[64];
    sprint_instr(out, &buf);
    print("%s", out);
    encode(&buf);
    if (brk) set_break();
    return type;
}
void decode(uint32_t gp) {
    print("Start decode %lx", gp);
    set_gp(gp);
    uint16_t block = cache_block_start();
    uint8_t jump_type = 0;
    if (block == debug_break()) {
        jump_type = decode_step();
        set_break();
    }
    while (jump_type != RET && jump_type != JMP) {
        jump_type = decode_step();
        /*
        TODO: Static analysis of block jumps. 
        Cache lookups are resource-intensive.
        */
        const uint8_t* blockp = cache_search(get_gp());
        if (blockp) {
            int32_t offset = (uint64_t)blockp - (uint64_t)(get_host()+get_hp());
            warning("DECODER::DUPLICATION %i", offset);
            cache_block_point();
            emit32(0x14000000 | ((offset/4) & 0x3FFFFFF));
            break;
        }
        if (cache_overflow()) {
            block++;
            if (block == debug_break()) {
                jump_type = decode_step();
                set_break();
            }
        }
    }
    cache_block_end();
}