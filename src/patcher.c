#include "patcher.h"
#include "cache.h"
#include "core.h"
#include <signal.h>
#include <stdlib.h>

void handler(int sig, siginfo_t* info, void* ucontext) {
    #if defined(__aarch64__) || defined(_M_ARM64)
    ucontext_t* ctx = (ucontext_t*)ucontext;
    struct sigcontext* sc = (struct sigcontext*)&ctx->uc_mcontext;
    print("RAX: %lX", sc->regs[0]);
    print("RCX: %lX", sc->regs[3]);
    print("RDX: %lX", sc->regs[2]);
    print("RBX: %lX", sc->regs[14]);
    print("RSP: %lX", sc->regs[31]);
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
    #endif
    exit(0);
}

void patcher_init() {
    struct sigaction sa = {
        .sa_sigaction = handler,
        .sa_flags = SA_SIGINFO,
    };
    sigaction(SIGTRAP, &sa, 0);
}