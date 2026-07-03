#include "decoder.h"
#include "armdef.h"
#include "memory.h"
#include "encoder.h"
#include "cache.h"
#include "core.h"
#include <stdint.h>
#include <stdio.h>

int64_t fetch_imm8() {
    return (int64_t)(int8_t)fetch8();
}
int64_t fetch_imm32() {
    return (int64_t)(int32_t)fetch32();
}
void decode_sib(Operand* op) {
    uint8_t sib = fetch8();
    uint8_t base = sib&7;
    op->type |= IDX;
    op->scale = sib>>6;
    op->idx = (sib>>3)&7;
    if (base == 0b101 && (op->type&IMM) == 0) {
        op->type |= IMM;
        op->imm = fetch_imm32();
    } else {
        op->type |= REG;
        op->reg = base;
    }
}
void decode_reg(Operand* op, uint8_t reg) {
    op->type = REG;
    op->reg = (reg >> 3)&7;
}
void decode_rm(Operand* op, uint8_t modrm) {
    uint8_t mod = modrm >> 6;
    uint8_t rm = modrm & 7;
    if (mod < 3) {
        op->type = MEM;
        if (mod == 1) {
            op->type |= IMM;
            op->imm = fetch_imm8();
        } else if (mod == 2) {
            op->type |= IMM;
            op->imm = fetch_imm32();
        }
        if (mod == 0 && rm == 0b101) {
            op->type |= IMM;
            op->imm = fetch_imm32();
        } else if (rm == 0b100) {
            decode_sib(op);
        } else {
            op->type |= REG;
            op->reg = rm;
        }
    } else {
        op->type = REG;
        op->reg = rm;
    }
}
void decode_regrm(X64_instruction* buf) {
    uint8_t byte = fetch8();
    decode_reg(&buf->op0, byte);
    decode_rm(&buf->op1, byte);
}

void print_op(X64_instruction* buf, Operand* op) {
    if (op->type == REG) {
        if (buf->size == 64) {
            printf("r%s ", regs[op->reg]);
        } else {
            printf("e%s ", regs[op->reg]);
        }
    } else if (op->type == IMM) {
        printf("%lx ", op->imm);
    } else {
        printf("[ ");
        if (op->type&REG) {
            printf("r%s ", regs[op->reg]);
            if ((op->type&IDX) || (op->type&IMM)) printf("+ ");
        }
        if (op->type&IDX) {
            printf("r%s ", regs[op->idx]);
            if (op->scale == 1) {
                printf("* 2 ");
            } else if (op->scale == 2) {
                printf("* 4 ");
            } else if (op->scale == 3) {
                printf("* 8 ");
            }
            printf("+ ");
        }
        if (op->type&IMM) {
            if (op->type == (MEM|IMM)) {
                printf("rip + ");
            }
            printf("%lx ", op->imm);
        }
        printf("] ");
    }
}
void print_instr(X64_instruction* buf) {
    printf("%s ", types[buf->type]);
    if (buf->opcount > 0)
        print_op(buf, &buf->op0);
    if (buf->opcount > 1)
        print_op(buf, &buf->op1);
    printf("\n");
}
int decode_instr(X64_instruction* buf) {
    int ret = 0;
    int reverse = 0;
    uint8_t rex = 0;
    uint8_t byte = fetch8();
    buf->size = 32;
    if (byte >> 4 == 0x4) {
        rex = byte & 0xF;
        byte = fetch8();
    } else if (byte == 0x66) {
        buf->size = 16;
        byte = fetch8();
    }
    switch (byte) {
        case 0x01:
            reverse = 1;
            buf->opcount = 2;
            buf->type = ADD;
            decode_regrm(buf);
            break;
        case 0x31:
            buf->opcount = 2;
            buf->type = XOR;
            decode_regrm(buf);
            break;
        case 0x3B:
            buf->opcount = 2;
            buf->type = CMP;
            decode_regrm(buf);
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            buf->size = 64;
            buf->opcount = 1;
            buf->type = PUSH;
            buf->op0.type = REG;
            buf->op0.reg = byte - 0x50;
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
            buf->size = 64;
            buf->opcount = 1;
            buf->type = POP;
            buf->op0.type = REG;
            buf->op0.reg = byte - 0x58;
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
        case 0x83: {
            reverse = 1;
            buf->opcount = 2;
            uint8_t modrm = fetch8();
            decode_rm(&buf->op1, modrm);
            buf->op0.type = IMM;
            buf->op0.imm = fetch_imm8();
            switch ((modrm >> 3)&7) {
                case 0: buf->type = ADD; break;
                case 1: break; // or
                case 2: break; // adc
                case 3: break; // sbb
                case 4: buf->type = AND; break;
                case 5: buf->type = SUB; break;
                case 6: buf->type = XOR; break;
                case 7: buf->type = CMP; break; // cmp
            }
        } break;
        case 0x85:
            buf->opcount = 2;
            buf->type = TST;
            decode_regrm(buf);
            break;
        case 0x89:
            reverse = 1;
            buf->opcount = 2;
            buf->type = MOV;
            decode_regrm(buf);
            break;
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
        case 0x98:
            buf->opcount = 1;
            buf->type = CLTQ;
            buf->op0.type = REG;
            buf->op0.reg = RAX;
            break;
        case 0xB8:
            buf->opcount = 2;
            buf->type = MOV;
            buf->op0.type = REG;
            buf->op0.reg = RAX;
            buf->op1.type = IMM;
            buf->op1.imm = fetch_imm32();
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
        case 0xC7:
            reverse = 1;
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
        case 0xf3: // currently only endbr64 is used
            warning("DECODER::REP_PREFIX");
            fetch16(); fetch8(); // just skip
            buf->opcount = 0;
            buf->type = EBR;
            break;
        case 0xff: {
            buf->size = 64;
            reverse = 1;
            buf->opcount = 1;
            uint8_t modrm = fetch8();
            decode_rm(&buf->op1, modrm);
            switch ((modrm >> 3)&7) {
                case 0:
                    buf->type = ADD;
                    buf->op0.type = IMM;
                    buf->op0.imm = 1;
                    break; // inc
                case 1:
                    buf->type = SUB;
                    buf->op0.type = IMM;
                    buf->op0.imm = 1;
                    break; // dec
                case 2:
                case 3: buf->type = CALL; break; // call
                case 4:
                case 5: buf->type = JMP; ret = JMP; break; // jmps
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
    if (reverse) {
        Operand tmp = buf->op0;
        buf->op0 = buf->op1;
        buf->op1 = tmp;
    }
    return ret;
}

void decode(uint32_t gp) {
    print("Start decode %x", gp);
    set_gp(gp);
    uint16_t block = cache_block_start();
    while (1) {
        cache_block_point();
        X64_instruction buf;
        uint8_t jump_type = decode_instr(&buf);
        print_instr(&buf);
        encode(&buf);
        if (jump_type == RET || jump_type == JMP) break;
        /*
        TODO: Static analysis of block jumps. 
        Cache lookups are resource-intensive.
        */
        const uint8_t* block = cache_search(get_gp());
        if (block) {
            int32_t offset = (uint64_t)block - (uint64_t)(get_host()+get_hp());
            warning("DECODER::DUPLICATION %i", offset);
            cache_block_point();
            emit32(0x14000000 | ((offset/4) & 0x3FFFFFF));
            break;
        }
    }
    cache_block_end();
    cache_flush(block);
}