#include "decoder.h"
#include "memory.h"
#include "core.h"

void decode_0F(Instruction* buf) {
    uint8_t byte = fetch8();
    switch (byte) {
        case 0x1E:
            buf->prefix = 0;
            buf->type = EBR;
            buf->a.type = NONE;
            fetch8();
            break;
        case 0x11:
            buf->type = MOVS;
            decode_rm_r(buf);
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
            decode_r_rm(buf);
            buf->a.type |= XMM;
            break;
        case 0x2C:
            buf->type = CVTSD2SI;
            decode_rm_r(buf);
            buf->b.type |= XMM;
            break;
        case 0x2f:
            if (buf->prefix == OS) buf->prefix = REPN;
            else buf->prefix = REPE;
            buf->type = COMIS;
            goto set;
        case 0x47:
            buf->type = CMOVA;
            decode_r_rm(buf);
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
            decode_rm_r(buf);
            buf->b.type |= XMM;
            break;
        case 0x6e:
            buf->type = MOVQ;
            decode_r_rm(buf);
            buf->a.type |= XMM;
            break;
        case 0xEF:
            buf->type = PXOR;
        set:
            decode_r_rm(buf);
            buf->a.type |= XMM;
            if (buf->b.type == REG)
                buf->b.type |= XMM;
            break;
        case 0x82 ... 0x8F:
            buf->type = byte - 0x82 + JB;
            buf->a.type = IMM;
            buf->a.imm = fetch_imm32();
            buf->b.type = NONE;
            break;
        case 0xB6:
            buf->type = MOVZX;
            decode_r_rm(buf);
            break;
        default: panic("DECODER::UNKNOWN_F0_SYMBOL: %X", byte);
    }
}