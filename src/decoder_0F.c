#include "decoder.h"
#include "memory.h"
#include "core.h"

void decode_0F(X64_instruction* buf) {
    uint8_t byte = fetch8();
    switch (byte) {
        case 0x1E:
            buf->opcount = 0;
            buf->type = EBR;
            fetch8();
            break;
        case 0x11:
            buf->reverse = 1;
        case 0x10:
            buf->type = MOVSS;
            goto set;
        case 0x28:
            buf->type = MOVAPD;
            goto set;
        case 0x5E:
            buf->type = DIVSS;
            goto set;
        case 0x59:
            buf->type = MULSS;
            goto set;
        case 0x5a:
            buf->type = CVTSS2SD;
            goto set;
        case 0x7e:
            buf->reverse = 1;
        case 0x6e:
            buf->opcount = 2;
            buf->type = MOVQ;
            decode_regrm(buf);
            buf->op1.type |= XMM;
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