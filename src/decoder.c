#include "decoder.h"
#include "memory.h"
#include "printer_x86.h"
#include "debugger.h"
#include "encoder.h"
#include "cache.h"
#include "core.h"
#include <stdint.h>

void decode_rm(Context* context, Operand* op, uint8_t modrm) {
    uint8_t mod = modrm >> 6;
    uint8_t rm = modrm & 7;
    if (mod < 3) {
        op->type = MEM;
        if (rm == 4) {
            uint8_t sib = fetch8(context);
            op->reg = sib&7;
            op->idx = (sib>>3)&7;
            op->scale = sib>>6;
            if (op->idx != 4) op->type |= IDX;

            if (op->reg == 5 && mod == 0) {
                op->type |= IMM;
                op->imm = (int64_t)(int32_t)fetch32(context);
            } else {
                op->type |= REG;
            }
        } else if (mod == 0 && rm == 5) {
            op->type |= IMM;
            op->imm = (int64_t)(int32_t)fetch32(context);
        } else {
            op->type |= REG;
            op->reg = rm;
        }
        if (mod == 1) {
            op->type |= IMM;
            op->imm = (int64_t)(int8_t)fetch8(context);
        } else if (mod == 2) {
            op->type |= IMM;
            op->imm = (int64_t)(int32_t)fetch32(context);
        }
    } else {
        op->type = REG;
        op->reg = rm;
    }
}
void decode_r_rm(Context* context, Instruction* buf) {
    uint8_t byte = fetch8(context);
    buf->reverse = 0;
    buf->a.type = REG;
    buf->a.reg = (byte >> 3) & 7;
    decode_rm(context, &buf->b, byte);
}
void decode_rm_r(Context* context, Instruction* buf) {
    uint8_t byte = fetch8(context);
    buf->reverse = 1;
    buf->b.type = REG;
    buf->b.reg = (byte >> 3) & 7;
    decode_rm(context, &buf->a, byte);
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
void decode_GRP0(Context* context, Instruction* buf, uint8_t byte) {
    if (!(byte&1)) buf->size = 8;
    uint8_t grp = byte&7;
    if (grp < 2) {
        decode_rm_r(context, buf);
    } else if (grp < 4) {
        decode_r_rm(context, buf);
    } else {
        buf->reverse = 0;
        buf->a.type = REG;
        buf->a.reg = RAX;
        buf->b.type = IMM;
        if (buf->size == 8)
            buf->b.imm = (int64_t)(int8_t)fetch8(context);
        else buf->b.imm = (int64_t)(int32_t)fetch32(context);
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


void decode_instr(Context* context, Instruction* buf) {
    buf->prefix = 0;
    buf->size = 32;
    uint8_t rex = 0;
    uint8_t byte = fetch8(context);
    if ((byte >= FS && byte <= OS) ||
    (byte >= REPN && byte <= REPE)) {
        buf->prefix = byte;
        byte = fetch8(context);
    }
    if ((byte&0xF0) == 0x40) {
        rex = byte & 0xF;
        byte = fetch8(context);
    }
    switch (byte) {
        case 0x00 ... 0x3F:
            if (byte == 0x0F) {
                decode_0F(context, buf);
            } else {
                buf->type = ADD + ((byte >> 3) & 7);
                decode_GRP0(context, buf, byte);
            }
            break;
        case 0x50 ... 0x57:
            buf->size = 64;
            buf->reverse = 1;
            buf->type = PUSH;
            buf->a.type = REG;
            buf->a.reg = byte&7;
            buf->b.type = NONE;
            break;
        case 0x58 ... 0x5F:
            buf->size = 64;
            buf->reverse = 1;
            buf->type = POP;
            buf->a.type = REG;
            buf->a.reg = byte&7;
            buf->b.type = NONE;
            break;
        case 0x63:
            buf->type = MOVSX;
            decode_r_rm(context, buf);
            break;
        case 0x68:
            buf->type = PUSH;
            buf->a.type = IMM;
            buf->a.imm = (int64_t)(int32_t)fetch32(context);
            buf->b.type = NONE;
            break;
        case 0x69:
            buf->type = IMUL;
            decode_r_rm(context, buf);
            buf->b.imm = (int64_t)(int32_t)fetch32(context);
            break;
        case 0x6A:
            buf->type = PUSH;
            buf->a.type = IMM;
            buf->a.imm = (int64_t)(int8_t)fetch8(context);
            buf->b.type = NONE;
            break;
        case 0x70 ... 0x7F:
            buf->type = JO + byte - 0x70;
            buf->a.type = IMM;
            buf->a.imm = (int64_t)(int8_t)fetch8(context);
            buf->b.type = NONE;
            break;
        case 0x80: 
            buf->size = 8;
            [[fallthrough]];
        case 0x81: case 0x83: {
            uint8_t modrm = fetch8(context);
            buf->reverse = 1;
            buf->type = ADD + ((modrm >> 3) & 7);
            decode_rm(context, &buf->a, modrm);
            buf->b.type = IMM;
            if (byte == 0x81) {
                buf->b.imm = (int64_t)(int32_t)fetch32(context);
            } else buf->b.imm = (int64_t)(int8_t)fetch8(context);
        } break;
        case 0x84:
            buf->size = 8;
            [[fallthrough]];
        case 0x85:
            buf->type = TEST;
            decode_rm_r(context, buf);
            break;
        case 0x88 ... 0x8B:
            buf->type = MOV;
            decode_GRP0(context, buf, byte);
            break;
        case 0x8D:
            buf->type = LEA;
            decode_r_rm(context, buf);
            break;
        case 0x8F:
            buf->size = 64;
            buf->reverse = 1;
            buf->type = POP;
            decode_rm(context, &buf->a, fetch8(context));
            buf->b.type = NONE;
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
        case 0xB0 ... 0xBF:
            buf->reverse = 1;
            buf->type = MOV;
            buf->a.type = REG;
            buf->a.reg = byte&7;
            buf->b.type = IMM;
            if (byte < 0xB8) {
                buf->size = 8;
                buf->b.imm = (int64_t)(int8_t)fetch8(context);
            }else {
                if (rex & 8) buf->b.imm = fetch64(context);
                else buf->b.imm = (int64_t)(int32_t)fetch32(context);
            }
            break;
        case 0xC0: case 0xC1:
        case 0xD0: case 0xD1:
        case 0xD2: case 0xD3:{
            if (!(byte&1)) buf->size = 8;
            uint8_t modrm = fetch8(context);
            buf->reverse = 1;
            buf->type = ROL + ((modrm >> 3) & 7);
            decode_rm(context, &buf->a, modrm);
            if (byte < 0xD2) {
                buf->b.type = IMM;
                if (byte > 0xC1) buf->b.imm = 1;
                else buf->b.imm = (int64_t)(int8_t)fetch8(context);
            } else {
                buf->b.type = REG;
                buf->b.reg = RCX;
            }
        } break;
        case 0xF4:
        case 0xC3:
            buf->type = RET;
            buf->a.type = NONE;
            break;
        case 0xC6:
            buf->size = 8;
            buf->reverse = 1;
            buf->type = MOV;
            decode_rm(context, &buf->a, fetch8(context));
            buf->b.type = IMM;
            buf->b.imm = (int64_t)(int8_t)fetch8(context);
            break;
        case 0xC7:
            buf->reverse = 1;
            buf->type = MOV;
            decode_rm(context, &buf->a, fetch8(context));
            buf->b.type = IMM;
            if (buf->prefix == OS) {
                buf->b.imm = (int64_t)(int32_t)fetch16(context);
                buf->size = 16;
            } else buf->b.imm = (int64_t)(int32_t)fetch32(context);
            break;
        case 0xC9:
            buf->type = LEAVE;
            buf->a.type = NONE;
            break;
        case 0xE8:
        case 0xE9:
            buf->type = CALL + (byte - 0xE8);
            buf->a.type = IMM;
            buf->a.imm = (int64_t)(int32_t)fetch32(context);
            buf->b.type = NONE;
            break;
        case 0xEB:
            buf->type = JMP;
            buf->a.type = IMM;
            buf->a.imm = (int64_t)(int8_t)fetch8(context);
            buf->b.type = NONE;
            break;
        case 0xF7: {
            uint8_t modrm = fetch8(context);
            uint8_t code = (modrm >> 3) & 7;
            if (code == 7) {
                buf->type = IDIV;
                buf->a.type = REG;
                buf->a.reg = RAX;
                decode_rm(context, &buf->b, modrm);
            } else if (code == 5) {
                buf->type = IMUL;
                buf->a.type = REG;
                buf->a.reg = RAX;
                decode_rm(context, &buf->b, modrm);
            } else panic("Unhandled F7");
        } break;
        case 0xFE:
            buf->size = 8;
            [[fallthrough]];
        case 0xFF: {
            buf->reverse = 1;
            uint8_t modrm = fetch8(context);
            uint8_t code = (modrm >> 3) & 7;
            decode_rm(context, &buf->a, modrm);
            decode_GRP3(buf, code);
        } break;
        default: panic("DECODER::UNKNOWN_SYMBOL: 0x%X", byte);
    }
    if (rex) decode_rex(buf, rex);
    if (get_log_level() >= LOG_PRINT) {
        char out[128];
        sprint_x86_64(buf, out);
        print("%s", out);
    }
}
void decode(uint64_t gp) {
    print("Start decode %lx", gp);
    Context* context = context_pull(gp);
    context_block_start(context);
    while (1) {
        context_block_guest_point(context);
        Instruction* buf = context_pull_buffer(context);
        decode_instr(context, buf);
        if (buf->type >= JO && buf->type <= JG) {
            context_push_jump(context, context->gp + buf->a.imm);
        } else if (buf->type == JMP) {
            if (buf->a.type == IMM)
                context_push_jump(context, context->gp + buf->a.imm);
            goto parse;
        } else if (buf->type == RET) {
        parse:
            context_block_end(context);
            uint32_t* gp = context_pull_jump(context);
            if (!gp) break;
            context->gp = *gp;
            print("new gp: %x", *gp);
            context_block_start(context);
        }
    }
    cache_start_block(context);
    for (int i = 0; i < context->blocks_p; i++) {
        CacheUnit* cache = context->blocks+i;
        cache->hp_lo = context->hp;
        uint32_t buffer = cache->buffer;
        for (int x = 0; x < cache->buffer_end; x++) {
            context_block_host_point(context, cache);
            encode(context, &context->buffer[buffer + x]);
        }
        OffsetUnit* offsets = context->offsets + cache->offsets;
        for (int x = 0; x < cache->offsetssz; x++) {
            print("offset %i: %i %i", x, offsets[x].goff, offsets[x].hoff);
        }
    }
    cache_end_block(context);
    context_free(context);
    if (debug_enabled()) debug_check_break();
}