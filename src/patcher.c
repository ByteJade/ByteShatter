#include "patcher.h"
#include "cache.h"
#include "core.h"
#include "memory.h"
#include "decoder.h"
#include "arm64emitter.h"
#include "armdef.h"
#include "debugger.h"
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

static struct sigcontext* sc;

void print_cpu() {
    #if defined(__aarch64__) || defined(_M_ARM64)
    print("PC:  %lX (%lX)", sc->pc, sc->pc - (uint64_t)get_host());
    for (int i = 0; i < 16; i++) {
        print("r%s: %lX", regs[i], sc->regs[x64_regs[i]]);
    }
    int N = (sc->pstate >> 31) & 1;
    int Z = (sc->pstate >> 30) & 1;  
    int C = (sc->pstate >> 29) & 1;
    int V = (sc->pstate >> 28) & 1;
    print("Flags: N%x Z%x C%x V%x", N, Z, C, V);
    #endif
}
void print_native_cpu() {
    #if defined(__aarch64__) || defined(_M_ARM64)
    print("PC:  %lX (%lX)", sc->pc, sc->pc - (uint64_t)get_host());
    for (int i = 0; i < 31; i++) {
        print("X%i: %lX", i, sc->regs[i]);
    }
    print("sp: %lX", sc->sp);
    int N = (sc->pstate >> 31) & 1;
    int Z = (sc->pstate >> 30) & 1;  
    int C = (sc->pstate >> 29) & 1;
    int V = (sc->pstate >> 28) & 1;
    print("Flags: N%x Z%x C%x V%x", N, Z, C, V);
    #endif
}
void brk(void) {return;};
void brk_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    sc = (struct sigcontext*)&ctx->uc_mcontext;
    #if defined(__aarch64__) || defined(_M_ARM64)
    uint32_t* code = (uint32_t*)sc->pc;
    uint32_t instruction = *code;
    uint16_t ret = (instruction >> 5) & 0xFFFF;
    if (ret == 0) debug_wait();
    print("ret: %x", ret);
    PatchUnit* patch = cache_get_patch(ret);
    CacheUnit* cahce = cache_get_block(patch->block);
    print("patch: %i", patch->guest_off);
    uint32_t gp = cahce->gp + patch->guest_off;
    const uint8_t* block = cache_search(gp);
    if (block == NULL) {
        warning("PATCHER::NOT_FOUND %lx", gp);
        block = get_host() + get_hp();
        decode(gp);
    }
    int32_t offset = (uint64_t)block - sc->pc;
    print("offset: %i", offset);
    switch (patch->type) {
        case JL:
            print("patch JL");
            *code = BLT_IMM | (((offset/4) & 0x7FFFF) << 5);
            break;
        case JLE:
            print("patch JL");
            *code = 0x5400000D | (((offset/4) & 0x7FFFF) << 5);
            break;
        case JE:
            print("patch JE");
            *code = 0x54000000 | (((offset/4) & 0x7FFFF) << 5);
            break;
        case JNE:
            print("patch JNE");
            *code = 0x54000001 | (((offset/4) & 0x7FFFF) << 5);
            break;
        case JG:
            print("patch JG");
            *code = 0x5400000C | (((offset/4) & 0x7FFFF) << 5);
            break;
        case JGE:
            print("patch JGE");
            *code = 0x5400000A | (((offset/4) & 0x7FFFF) << 5);
            break;
        case LEA:
            print("patch LEA");
            *code = 0x10000000 | ((offset & 0x3) << 29) | ((offset & 0x1FFFFC) << 3) | x64_regs[patch->meta];
            break;
        case JMP:
            print("patch JMP");
            *code = BR_IMM | ((offset/4) & 0x3FFFFFF);
            break;
        case CALL:
            print("patch CALL");
            *code = BLR_IMM | ((offset/4) & 0x3FFFFFF);
            break;
        default:
            panic("PATCHER::UNKNOWN_PATCH");
    }
    cache_flush(patch->block);
    if (cache_bp()-1 == block_break()) {
        success("Found break point %i", cache_bp()-1);
        debug_wait();
    }
    #endif
    brk();
}
void segv_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    struct sigcontext* sc = (struct sigcontext*)&ctx->uc_mcontext;
    print_cpu();
    cache_print();
    #if defined(__aarch64__) || defined(_M_ARM64)
    uint32_t* code = (uint32_t*)sc->pc;
    #else
    uint32_t* code = NULL;
    #endif
    if (code) {
        panic("segfault: %x", *code);
    } else panic("segfault");
}
void segi_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    struct sigcontext* sc = (struct sigcontext*)&ctx->uc_mcontext;
    print_cpu();
    cache_print();
    exit(0);
}
void patcher_init() {
    struct sigaction sa_trap = {
        .sa_sigaction = brk_handler,
        .sa_flags = SA_SIGINFO,
    };
    struct sigaction sa_segv = {
        .sa_sigaction = segv_handler,
        .sa_flags = SA_SIGINFO,
    };
    struct sigaction sa_segi = {
        .sa_sigaction = segi_handler,
        .sa_flags = SA_SIGINFO,
    };
    sigaction(SIGTRAP, &sa_trap, NULL);
    sigaction(SIGSEGV, &sa_segv, NULL);
    sigaction(SIGILL, &sa_segv, NULL);
    sigaction(SIGINT, &sa_segi, NULL);
}