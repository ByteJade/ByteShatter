#include "core.h"
#include "memory.h"
#include "cache.h"
#include "dlmanager.h"
#include "patcher.h"
#include "stack.h"
#include "executer.h"
#include "debugger.h"
#include <stdint.h>
#include <stdlib.h>


void usage() {
    printf("Usage: shatter [commands] <file> [arguments]\n");
    printf("\t-d  Enable debug mode\n");
    printf("\t-l  Set log level (-lA,-lE,-lW,-lN)\n");
    printf("\t-h  Print this help message\n");
    exit(0);
}

char* read_argv(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (i == (argc - 1)) usage();
        char* arg = argv[i];
        if (arg[0] == '-') {
            switch (arg[1]) {
                case 'd': debug_enable(); break;
                case 'l': set_log_level(arg[2]); break;
                case 'h':
                default: usage();
            }
        } else {
            for (int n = argc-1; n > i-1; n--) {
                push_arg(argv[n]);
            }
            return arg;
        }
    }
    usage();
    return NULL;
}

int main(int argc, char** argv, const char** envp) {
    patcher_init();
    cahce_init();
    stack_init();
    set_envp(envp);
    
    ExeMeta* exe = load_object(read_argv(argc, argv));
    finish_stack(exe);
    
    debug_wait();
    set_guest((uint64_t)exe->base);
    execute(exe->elf->header.e_entry);

    loader_close_elf(exe);
    loader_close_exe(exe);
    cahce_fini();
    memory_fini();
    stack_fini();
    success("anythink");
}