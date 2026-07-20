#include "patcher.h"
#include "core.h"
#include "cache.h"
#include "memory.h"
#include "decoder.h"
#include "arm64emitter.h"
#include "armdef.h"
#include "debugger.h"
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

static struct sigcontext* sc;
static int memory_check = 0;

uint64_t get_reg(const char* name) {
    name++;
    for (int i = 0; i < 16; i++) {
        if (strcmp(name, regs[i]) == 0)
            return sc->regs[x64_regs[i]];
    }
    return 0;
}
void memory_check_mode() {
    memory_check = 1;
}
int memory_fail() {
    if (memory_check) {
        memory_check = 0;
        return 0;
    }
    return 1;
}
void print_flags(void) {
    int N = (sc->pstate >> 31) & 1;
    int Z = (sc->pstate >> 30) & 1;  
    int C = (sc->pstate >> 29) & 1;
    int V = (sc->pstate >> 28) & 1;
    printf("Flags: N%x Z%x C%x V%x\n", N, Z, C, V);
}
void print_cpu(void) {
    printf("PC:  %llX (%llX)\n", sc->pc, sc->pc - (uint64_t)get_host());
    for (int i = 0; i < 16; i++) {
        printf("r%s: %llX\n", regs[i], sc->regs[x64_regs[i]]);
    }
    print_flags();
}
void print_native_cpu(void) {
    printf("PC:  %llX (%llX)\n", sc->pc, sc->pc - (uint64_t)get_host());
    for (int i = 0; i < 31; i++) {
        printf("X%i: %llX\n", i, sc->regs[i]);
    }
    printf("sp: %llX\n", sc->sp);
    print_flags();
}
void brk_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    sc = (struct sigcontext*)&ctx->uc_mcontext;
    
    uint32_t* code = (uint32_t*)sc->pc;
    uint32_t instruction = *code;
    uint16_t ret = (instruction >> 5) & 0xFFFF;
    if (ret == 0) {
        debug_wait();
        return;
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
        case JAE:
            print("patch JAE");
            *code = 0x54000002 | (((offset/4) & 0x7FFFF) << 5);
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
        case JBE:
            print("patch JBE");
            *code = 0x54000009 | (((offset/4) & 0x7FFFF) << 5);
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
}
void segv_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    sc = (struct sigcontext*)&ctx->uc_mcontext;
    if (memory_check) {
        sc->pc += 4;
        memory_check = 0;
        return;
    }
    print_cpu();
    uint32_t* code = (uint32_t*)sc->pc;
    if (code) {
        panic("segfault: %x", *code);
    } else panic("segfault");
}
void segi_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    sc = (struct sigcontext*)&ctx->uc_mcontext;
    if (debug_enabled()) {
        debug_wait();
    } else exit(0);
}
void patcher_init(void) {
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