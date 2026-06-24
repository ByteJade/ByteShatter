#include "decoder.h"
#include "armdef.h"
#include "memory.h"
#include "encoder.h"
#include "cache.h"
#include "core.h"
#include <stdint.h>
#include <stdio.h>

X64_instruction buf;

int64_t fetch_imm8() {
    return (int64_t)(int8_t)fetch8();
}
int64_t fetch_imm32() {
    return (int64_t)(int32_t)fetch32();
}
void decode_sib() {
    uint8_t sib = fetch8();
    uint8_t base = sib&7;
    buf.optype1 |= IDX;
    buf.scale = sib>>6;
    buf.idx = (sib>>3)&7;
    if (base == 0b101 && (buf.optype1&IMM) == 0) {
        buf.optype1 |= IMM;
        buf.imm1 = fetch_imm32();
    } else {
        buf.optype1 |= BASE;
        buf.reg1 = base;
    }
}
void decode_reg(uint8_t reg) {
    buf.optype0 = REG;
    buf.reg0 = (reg >> 3)&7;
}
void decode_rm(uint8_t modrm) {
    uint8_t mod = modrm >> 6;
    uint8_t rm = modrm & 7;
    if (mod < 3) {
        buf.optype1 = MEM;
        if (mod == 1) {
            buf.optype1 |= IMM;
            buf.imm1 = fetch_imm8();
        } else if (mod == 2) {
            buf.optype1 |= IMM;
            buf.imm1 = fetch_imm32();
        }
        if (mod == 0 && rm == 0b101) {
            buf.optype1 |= IMM;
            buf.imm1 = fetch_imm32();
        } else if (rm == 0b100) {
            decode_sib();
        } else {
            buf.optype1 |= BASE;
            buf.reg1 = rm;
        }
    } else {
        buf.optype1 = REG;
        buf.reg1 = rm;
    }
}
void decode_regrm() {
    uint8_t byte = fetch8();
    decode_reg(byte);
    decode_rm(byte);
}

void print_reg() {
    if (buf.optype0 == REG) {
        if (buf.size == 64) {
            printf("r%s ", regs[buf.reg0]);
        } else {
            printf("e%s ", regs[buf.reg0]);
        }
    } else {
        printf("%lx ", buf.imm0);
    }
}
void print_rm() {
    if (buf.optype1 == REG) {
        if (buf.size == 64) {
            printf("r%s ", regs[buf.reg1]);
        } else {
            printf("e%s ", regs[buf.reg1]);
        }
    } else {
        printf("[ ");
        if (buf.optype1&BASE) {
            printf("r%s + ", regs[buf.reg1]);
        }
        if (buf.optype1&IDX) {
            printf("r%s ", regs[buf.idx]);
            if (buf.scale == 1) {
                printf("* 2 ");
            } else if (buf.scale == 2) {
                printf("* 4 ");
            } else if (buf.scale == 3) {
                printf("* 8 ");
            }
        }
        if (buf.optype1&IMM) {
            if (buf.optype1 == (MEM|IMM)) {
                printf("rip ");
            }
            printf("+ %lx ", buf.imm1);
        }
        printf("] ");
    }
}
void print_instr() {
    printf("%s ", types[buf.type]);
    if (buf.reverse) {
        if (buf.opcount > 0)
            print_rm();
        if (buf.opcount > 1)
            print_reg();
    } else {
        if (buf.opcount > 0)
            print_reg();
        if (buf.opcount > 1)
            print_rm();
    }
    printf("\n");
}
int decode_instruction() {
    int ret = 0;
    uint8_t byte = fetch8();
    uint8_t rex = 0;
    buf.size = 32;
    if (byte >> 4 == 0x4) {
        rex = byte & 0xF;
        byte = fetch8();
    } else if (byte == 0x66) {
        buf.size = 16;
    }
    switch (byte) {
        case 0x31:
            buf.reverse = 0;
            buf.opcount = 2;
            buf.type = XOR;
            decode_regrm();
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            buf.size = 64;
            buf.reverse = 0;
            buf.opcount = 1;
            buf.type = PUSH;
            buf.optype0 = REG;
            buf.reg0 = byte&0x7;
            break;
        case 0x5E:
            buf.size = 64;
            buf.reverse = 0;
            buf.opcount = 1;
            buf.type = POP;
            buf.optype0 = REG;
            buf.reg0 = 6;
            break;
        case 0x74:
            buf.reverse = 0;
            buf.opcount = 1;
            buf.type = JE;
            buf.optype0 = IMM;
            buf.imm0 = fetch_imm8();
            ret = JE;
            break;
        case 0x83: {
            buf.reverse = 1;
            buf.opcount = 2;
            uint8_t modrm = fetch8();
            buf.optype0 = IMM;
            buf.imm0 = fetch_imm8();
            decode_rm(modrm);
            switch ((modrm >> 3)&7) {
                case 0: buf.type = ADD; break;
                case 1: break; // or
                case 2: break; // adc
                case 3: break; // sbb
                case 4: buf.type = AND; break;
                case 5: buf.type = SUB; break;
                case 6: buf.type = XOR; break;
                case 7: break; // cmp
            }
        } break;
        case 0x85:
            buf.reverse = 0;
            buf.opcount = 2;
            buf.type = TST;
            decode_regrm();
            break;
        case 0x89:
            buf.reverse = 1;
            buf.opcount = 2;
            buf.type = MOV;
            decode_regrm();
            break;
        case 0x8B:
            buf.reverse = 0;
            buf.opcount = 2;
            buf.type = MOV;
            decode_regrm();
            break;
        case 0x8D:
            buf.reverse = 0;
            buf.opcount = 2;
            buf.type = LEA;
            decode_regrm();
            break;
        case 0xC3:
        case 0xF4: // actualy hlt
            buf.opcount = 0;
            buf.type = RET;
            ret = RET;
            break;
        case 0xf3: // currently only endbr64 is used
            warning("DECODER::REP_PREFIX");
            fetch16(); fetch8(); // just skip
            return 0;
        case 0xff: {
            buf.size = 64;
            buf.reverse = 1;
            buf.opcount = 1;
            uint8_t modrm = fetch8();
            decode_rm(modrm);
            switch ((modrm >> 3)&7) {
                case 0: buf.type = ADD; break; // inc
                case 1: buf.type = SUB; break; // dec
                case 2:
                case 3: buf.type = CALL; ret = CALL; break; // call
                case 4:
                case 5: break; // jmps
                case 6: buf.type = PUSH; break; // push
            }
        } break;
        default:
            panic("DECODER::UNKNOWN_SYMBOL: %X", byte);
    }
    if (rex) {
        if (rex&8) buf.size = 64;
        if (rex&4) buf.reg0 += 8;
        if (rex&2) buf.idx += 8;
        if (rex&1) buf.reg1 += 8;
    }
    print_instr();
    encode(&buf);
    return ret;
}

void decode(uint32_t gp) {
    print("Start decode %x", gp);
    set_gp(gp);
    cache_block_start();
    while (1) {
        cache_block_point();
        uint8_t jump_type = decode_instruction();
        if (jump_type == RET) break;
    }
    cache_block_end();
}