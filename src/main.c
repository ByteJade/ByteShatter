#include "core.h"
#include "elf_loader.h"
#include "patcher.h"
#include "stack.h"
#include "debugger.h"
#include "memory.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void usage(void) {
    printf("Usage: shatter [commands] <file> [arguments]\n");
    printf("\t-d  Enable debug mode\n");
    printf("\t-l  Set log level (-lA,-lW,-lE)\n");
    printf("\t-h  Print this help message\n");
    exit(0);
}

int debug = 0;

int read_argv(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (arg[0] == '-') {
            switch (arg[1]) {
                case 'd': debug = 1; break;
                case 'l': set_log_level(arg[2]); break;
                case 'h':
                default: usage();
            }
        } else return i;
    }
    usage();
    return 0;
}

int main(int argc, char** argv, const char** envp) {
    stack_init();
    set_envp(envp);
    int end = read_argv(argc, argv);
    context_init();
    Elf* elf = elf_load(argv[end]);

    elf_read_dynamic(elf);
    patcher_init();

    elf_init(elf);
    finish_stack(elf);
    push_arg(0);
    for (int n = argc-1; n > end-1; n--) {
        push_arg(argv[n]);
    }
    push_argc();
    
    if (debug) debug_enable(elf);
    void* sp = get_sp();
    void* entry = elf->base + elf->head.e_entry;
    #ifdef __aarch64__
    asm volatile (
        "mov sp, %0\n"
        "br %1"
        : : "r"(sp), "r"(entry)
        : "memory"
    );
    #else 
    asm volatile (
        "mov rsp, %0\n"
        "jmp *%1"
        : : "r"(sp), "r"(entry)
        : "memory"
    );
    #endif
}