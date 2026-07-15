#include "decoder.h"
#include "memory.h"
#include "core.h"

void decode_0F(X64_instruction* buf) {
    uint8_t byte = fetch8();
    switch (byte) {
        case 0x10:
            buf->opcount = 2;
            buf->type = MOVSS;
            decode_regrm(buf);
            buf->op0.type |= XMM;
            break;
        default: panic("DECODER::UNKNOWN_F0_SYMBOL: %X", byte);
    }
}