#include "cache.h"
#include "decoder.h"
#include "memory.h"
#include "core.h"

void decode_0F(Context* context, Instruction* buf) {
    uint8_t byte = fetch8(context);
    switch (byte) {
        case 0x1E:
            buf->prefix = 0;
            buf->type = EBR;
            buf->a.type = NONE;
            fetch8(context);
            break;
        case 0x11:
            buf->type = MOVS;
            decode_rm_r(context, buf);
            if (buf->a.type == REG)
                buf->a.type |= XMM;
            buf->b.type |= XMM;
            break;
        case 0x10:
            buf->type = MOVS;
            goto set;
        case 0x28:
            buf->type = MOVAPD;
            goto set;
        case 0x2A:
            buf->type = CVTSI2S;
            decode_r_rm(context, buf);
            buf->a.type |= XMM;
            break;
        case 0x2C:
            buf->type = CVTSD2SI;
            decode_rm_r(context, buf);
            buf->b.type |= XMM;
            break;
        case 0x2f:
            if (buf->prefix == OS) buf->prefix = REPN;
            else buf->prefix = REPE;
            buf->type = COMIS;
            goto set;
        case 0x40 ... 0x4F:
            buf->type = byte - 0x40 + CMOVO;
            decode_r_rm(context, buf);
            break;
        case 0x57:
            buf->type = PXOR;
            goto set;
        case 0x58:
            buf->type = ADDS;
            goto set;
        case 0x59:
            buf->type = MULS;
            goto set;
        case 0x5a:
            if (buf->prefix == REPE) buf->type = CVTSS2SD;
            else buf->type = CVTSD2SS;
            goto set;
        case 0x5C:
            buf->type = SUBS;
            goto set;
        case 0x5E:
            buf->type = DIVS;
            goto set;
        case 0x7e:
            buf->type = MOVQ;
            decode_rm_r(context, buf);
            buf->b.type |= XMM;
            break;
        case 0x6e:
            buf->type = MOVQ;
            decode_r_rm(context, buf);
            buf->a.type |= XMM;
            break;
        case 0xEF:
            buf->type = PXOR;
        set:
            decode_r_rm(context, buf);
            buf->a.type |= XMM;
            if (buf->b.type == REG)
                buf->b.type |= XMM;
            break;
        case 0x80 ... 0x8F:
            buf->type = byte - 0x80 + JO;
            buf->a.type = IMM;
            buf->a.imm = fetch_imm32;
            buf->b.type = NONE;
            break;
        case 0x90 ... 0x9F:
            buf->size = 8;
            buf->type = byte - 0x90 + SETO;
            decode_rm(context, &buf->a, fetch8(context));
            buf->b.type = NONE;
            break;
        case 0xAF:
            buf->type = IMUL;
            decode_r_rm(context, buf);
            break;
        case 0xB6:
            buf->type = MOVZX;
            decode_r_rm(context, buf);
            break;
        default: panic("DECODER::UNKNOWN_0F_SYMBOL: 0x%X", byte);
    }
}