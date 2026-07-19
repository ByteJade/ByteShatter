#include "armdef.h"
#include "decoder.h"
#include "memory.h"
#include "core.h"

void decode_0F(X64_instruction* buf) {
    uint8_t byte = fetch8();
    switch (byte) {
        case 0x1E:
            buf->prefix = 0;
            buf->opcount = 0;
            buf->type = EBR;
            fetch8();
            break;
        case 0x11:
            buf->reverse = 1;
        case 0x10:
            buf->type = MOVS;
            goto set;
        case 0x28:
            buf->type = MOVAPD;
            goto set;
        case 0x2A:
            buf->opcount = 2;
            buf->type = CVTSI2SD;
            decode_regrm(buf);
            buf->op0.type |= XMM;
            break;
        case 0x2C:
            buf->reverse = 1;
            buf->opcount = 2;
            buf->type = CVTSD2SI;
            decode_regrm(buf);
            buf->op0.type |= XMM;
            break;
        case 0x2f:
            if (buf->prefix == P66) buf->prefix = REPN;
            else buf->prefix = REP;
            buf->type = COMIS;
            goto set;
        case 0x5E:
            buf->type = DIVS;
            goto set;
        case 0x58:
            buf->type = ADDS;
            goto set;
        case 0x59:
            buf->type = MULS;
            goto set;
        case 0x5C:
            buf->type = SUBS;
            goto set;
        case 0x5a:
            if (buf->prefix == REP) buf->type = CVTSS2SD;
            else buf->type = CVTSS2SS;
            goto set;
        case 0x7e:
            buf->reverse = 1;
        case 0x6e:
            buf->opcount = 2;
            buf->type = MOVQ;
            decode_regrm(buf);
            buf->op0.type |= XMM;
            break;
        case 0xEF:
            buf->type = PXOR;
        set:
            buf->opcount = 2;
            decode_regrm(buf);
            buf->op0.type |= XMM;
            if (buf->op1.type == REG)
                buf->op1.type |= XMM;
            break;
        default: panic("DECODER::UNKNOWN_F0_SYMBOL: %X", byte);
    }
}