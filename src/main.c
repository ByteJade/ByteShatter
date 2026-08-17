#include "core.h"
#include "memory.h"
#include "cache.h"
#include "elf_loader.h"
#include "patcher.h"
#include "stack.h"
#include "executer.h"
#include "debugger.h"
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

int read_argv(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (arg[0] == '-') {
            switch (arg[1]) {
                case 'd': debug_enable(); break;
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
    Elf* elf = elf_load(argv[end]);
    cahce_init((uint64_t)elf->base);

    elf_read_dynamic(elf);
    patcher_init();

    elf_init(elf);
    finish_stack(elf);
    push_arg(0);
    for (int n = argc-1; n > end-1; n--) {
        push_arg(argv[n]);
    }
    push_argc();
    
    debug_wait();
    execute((uint64_t)elf->base + elf->head.e_entry);

    elf_close(elf);
    cahce_fini();
    memory_fini();
    stack_fini();
    success("anythink");
}