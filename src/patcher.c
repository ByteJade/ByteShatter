#include "patcher.h"
#include "cache.h"
#include "core.h"
#include "memory.h"
#include "decoder.h"
#include <signal.h>
#include <stdint.h>

void handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    struct sigcontext* sc = (struct sigcontext*)&ctx->uc_mcontext;
    #if defined(__aarch64__) || defined(_M_ARM64)
    print("RAX: %lX", sc->regs[0]);
    print("RCX: %lX", sc->regs[3]);
    print("RDX: %lX", sc->regs[2]);
    print("RBX: %lX", sc->regs[14]);
    print("RSP: %lX", sc->sp);
    print("RBP: %lX", sc->regs[15]);
    print("RSI: %lX", sc->regs[1]);
    print("RDI: %lX", sc->regs[9]);
    print("R8:  %lX", sc->regs[4]);
    print("R9:  %lX", sc->regs[5]);
    print("R10: %lX", sc->regs[10]);
    print("R11: %lX", sc->regs[11]);
    print("R12: %lX", sc->regs[16]);
    print("R13: %lX", sc->regs[17]);
    print("R14: %lX", sc->regs[18]);
    print("R15: %lX", sc->regs[19]);

    uint32_t* code = (uint32_t*)sc->pc;
    uint32_t instruction = *code;
    uint16_t ret = (instruction >> 5) & 0xFFFF;
    if (ret == 0) {
        panic("Unknown instruction");
    }
    print("ret: %x", ret);
    JumpUnit* jump = cache_get_jump(ret);
    CacheUnit* cahce = cache_get_block(jump->block);
    print("jump: %i", jump->guest_off);
    uint32_t gp = cahce->gp + jump->guest_off;
    const uint8_t* block = cache_search(gp);
    if (block == NULL) {
        warning("PATCHER::NOT_FOUND %lx", gp);
        block = get_host() + get_hp();
        decode(gp);
    }
    int32_t offset = (uint64_t)block - sc->pc + 4;
    print("offset: %i", offset);
    *code = 0x54000000 | ((offset & 0x7FFFF) << 3);
    #endif
}

void patcher_init() {
    struct sigaction sa = {
        .sa_sigaction = handler,
        .sa_flags = SA_SIGINFO,
    };
    sigaction(SIGTRAP, &sa, 0);
}