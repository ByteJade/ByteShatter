#include "decoder.h"
#include "memory.h"
#include "printer_x86.h"
#include "debugger.h"
#include "encoder.h"
#include "cache.h"
#include "core.h"

int64_t fetch_imm8(void) {
    return (int64_t)(int8_t)fetch8();
}
int64_t fetch_imm32(void) {
    return (int64_t)(int32_t)fetch32();
}
void decode_sib(Operand* op, uint8_t mod) {
    uint8_t sib = fetch8();
    op->reg = sib&7;
    op->idx = (sib>>3)&7;
    op->scale = sib>>6;
    if (op->idx != 4) op->type |= IDX;

    if (op->reg == 5 && mod == 0) {
        op->type |= IMM;
        op->imm = fetch_imm32();
    } else {
        op->type |= REG;
    }
}
void decode_rm(Operand* op, uint8_t modrm) {
    uint8_t mod = modrm >> 6;
    uint8_t rm = modrm & 7;
    if (mod < 3) {
        op->type = MEM;
        if (rm == 4) {
            decode_sib(op, mod);
        } else if (mod == 0 && rm == 5) {
            op->type |= IMM;
            op->imm = fetch_imm32();
        } else {
            op->type |= REG;
            op->reg = rm;
        }
        if (mod == 1) {
            op->type |= IMM;
            op->imm = fetch_imm8();
        } else if (mod == 2) {
            op->type |= IMM;
            op->imm = fetch_imm32();
        }
    } else {
        op->type = REG;
        op->reg = rm;
    }
}
void decode_r_rm(Instruction* buf) {
    uint8_t byte = fetch8();
    buf->reverse = 0;
    buf->a.type = REG;
    buf->a.reg = (byte >> 3) & 7;
    decode_rm(&buf->b, byte);
}
void decode_rm_r(Instruction* buf) {
    uint8_t byte = fetch8();
    buf->reverse = 1;
    buf->b.type = REG;
    buf->b.reg = (byte >> 3) & 7;
    decode_rm(&buf->a, byte);
}
void decode_rex(Instruction* buf, uint8_t rex) {
    if (rex&8) buf->size = 64;
    if (buf->reverse) {
        if (rex&4) buf->b.reg += 8;
        if (rex&2) buf->a.idx += 8;
        if (rex&1) buf->a.reg += 8;
    } else {
        if (rex&4) buf->a.reg += 8;
        if (rex&2) buf->b.idx += 8;
        if (rex&1) buf->b.reg += 8;
    }
}
void decode_GRP0(Instruction* buf, uint8_t byte) {
    if (!(byte&1)) buf->size = 8;
    uint8_t grp = byte&7;
    if (grp < 2) {
        decode_rm_r(buf);
    } else if (grp < 4) {
        decode_r_rm(buf);
    } else {
        buf->reverse = 0;
        buf->a.type = REG;
        buf->a.reg = RAX;
        buf->b.type = IMM;
        if (buf->size == 8)
            buf->b.imm = fetch_imm8();
        else buf->b.imm = fetch_imm32();
    }
}
void decode_GRP3(Instruction* buf, uint8_t code) {
    if (code > 1) {
        buf->size = 64;
        buf->b.type = NONE;
    }
    switch (code) {
        case 0:
            buf->type = ADD;
            buf->b.type = IMM;
            buf->b.imm = 1;
            break;
        case 1:
            buf->type = SUB;
            buf->b.type = IMM;
            buf->b.imm = 1;
            break;
        case 2: buf->type = CALL; break;
        case 4: buf->type = JMP; break;
        case 6: buf->type = PUSH; break;
        default: panic("UNKNOWN_GRP3: %x", code);
    }
}

int decode_instr(Instruction* buf) {
    buf->prefix = 0;
    buf->size = 32;
    uint8_t rex = 0;
    uint8_t ret = 0;
    uint8_t byte = fetch8();
    if ((byte >= FS && byte <= OS) ||
    (byte >= REPN && byte <= REPE)) {
        buf->prefix = byte;
        byte = fetch8();
    }
    if ((byte&0xF0) == 0x40) {
        rex = byte & 0xF;
        byte = fetch8();
    }
    switch (byte) {
        case 0x00 ... 0x3F:
            if (byte == 0x0F) {
                decode_0F(buf);
            } else {
                buf->type = ADD + ((byte >> 3) & 7);
                decode_GRP0(buf, byte);
            }
            break;
        case 0x50 ... 0x57:
            buf->size = 64;
            buf->reverse = 0;
            buf->type = PUSH;
            buf->a.type = REG;
            buf->a.reg = byte&7;
            buf->b.type = NONE;
            break;
        case 0x58 ... 0x5F:
            buf->size = 64;
            buf->reverse = 0;
            buf->type = POP;
            buf->a.type = REG;
            buf->a.reg = byte&7;
            buf->b.type = NONE;
            break;
        case 0x63:
            buf->type = MOVSX;
            decode_r_rm(buf);
            break;
        case 0x68:
            buf->type = PUSH;
            buf->a.type = IMM;
            buf->a.imm = fetch_imm32();
            buf->b.type = NONE;
            break;
        case 0x6A:
            buf->type = PUSH;
            buf->a.type = IMM;
            buf->a.imm = fetch_imm8();
            buf->b.type = NONE;
            break;
        case 0x70 ... 0x7F:
            buf->type = JO + byte - 0x70;
            buf->a.type = IMM;
            buf->a.imm = fetch_imm8();
            buf->b.type = NONE;
            break;
        case 0x80: 
            buf->size = 8;
            [[fallthrough]];
        case 0x81: case 0x83: {
            uint8_t modrm = fetch8();
            buf->reverse = 1;
            buf->type = ADD + ((modrm >> 3) & 7);
            decode_rm(&buf->a, modrm);
            buf->b.type = IMM;
            if (byte == 0x81) {
                buf->b.imm = fetch_imm32();
            } else buf->b.imm = fetch_imm8();
        } break;
        case 0x84: case 0x85:
            buf->type = TEST;
            decode_rm_r(buf);
            break;
        case 0x88 ... 0x8B:
            buf->type = MOV;
            decode_GRP0(buf, byte);
            break;
        case 0x8D:
            buf->type = LEA;
            decode_r_rm(buf);
            break;
        case 0x90: 
            buf->type = NOP;
            buf->a.type = NONE;
            break;
        case 0x98:
            buf->type = CLTQ;
            buf->a.type = NONE;
            break;
        case 0x99:
            buf->type = CLTD;
            buf->a.type = NONE;
            break;
        case 0x9F:
            buf->size = 64;
            buf->reverse = 1;
            buf->type = POP;
            decode_rm(&buf->a, fetch8());
            buf->b.type = NONE;
            break;
        case 0xB0 ... 0xBF:
            buf->reverse = 0;
            buf->type = MOV;
            buf->a.type = REG;
            buf->a.reg = byte&7;
            buf->b.type = IMM;
            if (byte < 0xB8) {
                buf->size = 8;
                buf->b.imm = fetch_imm8();
            }else buf->b.imm = fetch_imm32();
            break;
        case 0xC0: case 0xC1:
        case 0xD0: case 0xD1:
        case 0xD2: case 0xD3:{
            if (!(byte&1)) buf->size = 8;
            uint8_t modrm = fetch8();
            buf->reverse = 1;
            buf->type = ROL + ((modrm >> 3) & 7);
            decode_rm(&buf->a, modrm);
            if (byte < 0xD2) {
                buf->b.type = IMM;
                if (byte > 0xC1) buf->b.imm = 1;
                else buf->b.imm = fetch_imm8();
            } else {
                buf->b.type = REG;
                buf->b.reg = RCX;
            }
        } break;
        case 0xF4:
        case 0xC3:
            buf->type = RET;
            buf->a.type = NONE;
            ret = 1;
            break;
        case 0xC6:
            buf->size = 8;
            buf->reverse = 1;
            buf->type = MOV;
            decode_rm(&buf->a, fetch8());
            buf->b.type = IMM;
            buf->b.imm = fetch_imm8();
            break;
        case 0xC7:
            buf->reverse = 1;
            buf->type = MOV;
            decode_rm(&buf->a, fetch8());
            buf->b.type = IMM;
            buf->b.imm = fetch_imm32();
            break;
        case 0xC9:
            buf->type = LEAVE;
            buf->a.type = NONE;
            break;
        case 0xE8:
        case 0xE9:
            buf->type = CALL + (byte - 0xE8);
            buf->a.type = IMM;
            buf->a.imm = fetch_imm32();
            buf->b.type = NONE;
            if (buf->type == JMP) ret = 1;
            break;
        case 0xEB:
            buf->type = JMP;
            buf->a.type = IMM;
            buf->a.imm = fetch_imm8();
            buf->b.type = NONE;
            ret = 1;
            break;
        case 0xF7: {
            uint8_t byte = fetch8();
            if ((byte&0b11111000) == 0xF8) {
                buf->type = IDIV;
                buf->a.type = REG;
                buf->a.reg = byte&0x7;
                buf->b.reg = NONE;
            } else panic("Unhandled F7");
        } break;
        case 0xFE:
            buf->size = 8;
            [[fallthrough]];
        case 0xFF: {
            buf->reverse = 1;
            uint8_t modrm = fetch8();
            uint8_t code = (modrm >> 3) & 7;
            decode_rm(&buf->a, modrm);
            decode_GRP3(buf, code);
            if (buf->type == JMP) ret = 1;
        } break;
        default: panic("DECODER::UNKNOWN_SYMBOL: %X", byte);
    }
    if (rex) decode_rex(buf, rex);
    if (get_log_level() >= LOG_PRINT) {
        char out[128];
        sprint_x86_64(buf, out);
        print("%s", out);
    }
    return ret;
}
int decode_step() {
    uint8_t brk = 0;
    if (get_gp() == debug_breakp()) brk = 1;
    cache_block_point();
    Instruction buf;
    int type = decode_instr(&buf);
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
    while (1) {
        jump_type = decode_step();
        if (jump_type) break;
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