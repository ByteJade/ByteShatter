#include "patcher.h"
#include "core.h"
#include "cache.h"
#include "memory.h"
#include "decoder.h"
#include "arm64emitter.h"
#include "printer_x86.h"
#include "debugger.h"
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

static struct sigcontext* sc;
static int memory_check = 0;

uint64_t get_reg(const char* name) {
    if (strcmp(name, "rsp") == 0) return sc->sp;
    for (int i = 0; i < 16; i++) {
        if (strcmp(name, regs64[i]) == 0)
            return sc->regs[x64_regs[i]];
    }
    return 0;
}
uint64_t get_pc() {
    return sc->pc;
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
        printf("%s: %llX\n", regs64[i], sc->regs[x64_regs[i]]);
    }
    print_flags();
}
void print_native_cpu(void) {
    uint32_t* pc = (uint32_t*)get_pc();
    printf("PC:  %p (%p)\n", pc, (void*)((uint64_t)pc - (uint64_t)get_host()));
    for (int i = 0; i < 31; i++) {
        printf("X%i: %llX\n", i, sc->regs[i]);
    }
    printf("sp: %llX\n", sc->sp);
    print_flags();
}
void brk_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    sc = (struct sigcontext*)&ctx->uc_mcontext;
    
    uint32_t* pc = (uint32_t*)get_pc();
    uint32_t instruction = *pc;
    uint16_t ret = (instruction >> 5) & 0xFFFF;
    if (ret == 0) {
        debug_wait();
        return;
    }
    print("ret: %x", ret);
    PatchUnit* patch = cache_get_patch(ret);
    print("patch: %i", patch->guest_off);
    const uint32_t* block = cache_search(patch->guest_off);
    if (block == NULL) {
        warning("PATCHER::NOT_FOUND %lx", patch->guest_off);
        block = get_host() + get_hp();
        decode(patch->guest_off);
    }
    int32_t offset = (block - pc);
    print("offset: %i", offset);
    switch (patch->type) {
        case JL:
            print("patch JL");
            *pc = BLT_IMM | ((offset & 0x7FFFF) << 5);
            break;
        case JLE:
            print("patch JL");
            *pc = 0x5400000D | ((offset & 0x7FFFF) << 5);
            break;
        case JE:
            print("patch JE");
            *pc = 0x54000000 | ((offset & 0x7FFFF) << 5);
            break;
        case JAE:
            print("patch JAE");
            *pc = 0x54000002 | ((offset & 0x7FFFF) << 5);
            break;
        case JNE:
            print("patch JNE");
            *pc = 0x54000001 | ((offset & 0x7FFFF) << 5);
            break;
        case JG:
            print("patch JG");
            *pc = 0x5400000C | ((offset & 0x7FFFF) << 5);
            break;
        case JGE:
            print("patch JGE");
            *pc = 0x5400000A | ((offset & 0x7FFFF) << 5);
            break;
        case JBE:
            print("patch JBE");
            *pc = 0x54000009 | ((offset & 0x7FFFF) << 5);
            break;
        case JMP:
            print("patch JMP");
            *pc = BR_IMM | (offset & 0x3FFFFFF);
            break;
        case CALL:
            print("patch CALL");
            *pc = BLR_IMM | (offset & 0x3FFFFFF);
            break;
        default:
            panic("PATCHER::UNKNOWN_PATCH");
    }
    void* clear = (void*)pc;
    __builtin___clear_cache(clear, clear + 4);
}
void segv_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext_t* ctx = (ucontext_t*)ucontext;
    sc = (struct sigcontext*)&ctx->uc_mcontext;
    if (memory_check) {
        sc->pc += 4;
        memory_check = 0;
        return;
    }
    if (info->si_code == SEGV_ACCERR || sc->pc%4 != 0) {
        success("found unhandled jump");
        uint32_t block = cache_search_block((uint32_t*)sc->pc - get_host());
        if (block == UINT32_MAX) {
            warning("PATCHER::NOT_FOUND %lx", sc->pc);
            decode(sc->pc - (uint64_t)get_guest());
            block = cache_search_block((uint32_t*)sc->pc - get_host());
        }
        sc->pc = (uint64_t)(get_host() + cache_get_block(block)->hp);
        return;
    }
    uint32_t* code = (uint32_t*)sc->pc;
    const char* name;
    if (sig == SIGBUS) name = "SIGBUS";
    else name = "segfault";
    if (debug_enabled()) {
        warning("%s", name);
        debug_wait();
    }
    print_cpu();
    if (code) {
        panic("%s: %x", name, *code);
    } else panic("%s", name);
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
    sigaction(SIGBUS, &sa_segv, NULL);
    sigaction(SIGINT, &sa_segi, NULL);
}