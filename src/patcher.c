#include "patcher.h"
#include "cache.h"
#include "core.h"
#include "memory.h"
#include "decoder.h"
#include "armdef.h"
#include <signal.h>
#include <stdint.h>

void print_cpu(struct sigcontext* sc) {
    #if defined(__aarch64__) || defined(_M_ARM64)
    print("PC: %lX", sc->pc);
    print("RAX: %lX", sc->regs[0]);
    print("RCX: %lX", sc->regs[3]);
    print("RDX: %lX", sc->regs[2]);
    print("RBX: %lX", sc->regs[14]);
    print("RSP: %lX", sc->regs[28]);
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
    int N = (sc->pstate >> 31) & 1;
    int Z = (sc->pstate >> 30) & 1;  
    int C = (sc->pstate >> 29) & 1;
    int V = (sc->pstate >> 28) & 1;
    print("Flags: N%x Z%x C%x V%x", N, Z, C, V);
    #endif
}

void brk_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    struct sigcontext* sc = (struct sigcontext*)&ctx->uc_mcontext;
    print_cpu(sc);
    #if defined(__aarch64__) || defined(_M_ARM64)
    uint32_t* code = (uint32_t*)sc->pc;
    uint32_t instruction = *code;
    uint16_t ret = (instruction >> 5) & 0xFFFF;
    if (ret == 0) {
        panic("Unknown instruction");
    }
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
        case JE:
            print("patch JE");
            *code = 0x54000000 | ((offset & 0x7FFFF) << 3);
            break;
        case LEA:
            print("patch LEA");
            *code = 0x10000000 | ((offset & 0x3) << 29) | ((offset & 0x1FFFFC) << 3) | x64_regs[patch->meta];
            break;
        case JMP:
            print("patch LEA");
            *code = 0x14000000 | (offset & 0x3FFFFFF);
            break;
        case CALL:
            print("patch LEA");
            *code = 0x94000000 | (offset & 0x3FFFFFF);
            break;
        default:
            panic("PATCHER::UNCNOWN_PATCH");
    }
    cache_flush(patch->block);
    #endif
    success("patching");
}
void segv_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    struct sigcontext* sc = (struct sigcontext*)&ctx->uc_mcontext;
    print_cpu(sc);
    cache_print();
    #if defined(__aarch64__) || defined(_M_ARM64)
    uint32_t* code = (uint32_t*)sc->pc;
    panic("segfault: %x", *code);
    #endif
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
    sigaction(SIGTRAP, &sa_trap, 0);
    sigaction(SIGSEGV, &sa_segv, 0);
    sigaction(SIGILL, &sa_segv, 0);
}