#include "debugger.h"
#include "core.h"
#include "cache.h"
#include "patcher.h"
#include "memory.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int enabled = 0;
int break_block = 0;
uint64_t break_point = 0;
uint32_t prev_instr = 0;
uint32_t* prev_instrp = NULL;

void debug_enable(void) {
    enabled = 1;
}
int debug_break(void) {
    return break_block;
}
void set_break_point() {
    uint32_t* pc = (uint32_t*)(get_host() + get_hp());
    printf("host is %p\n", get_host());
    printf("hp is %li\n", get_hp());
    if (prev_instrp) *prev_instrp = prev_instr;
    prev_instr = *pc;
    prev_instrp = pc;
    *pc = 0xD4200000;
}
void help(void) {
    printf("Commands:\n");
    printf("brb <i> - set break point in block i\n");
    printf("sb - go to next block\n");
    printf("si - go to next instruction\n");
    printf("log <level> - set logs to level (A,W,E)\n");
    printf("print <state> - print state (x64_regs, arm_regs, cache)\n");
    printf("run - return to execution\n");
    printf("exit - stop execution\n");
}
void handle_print(char* arg) {
    if (strcmp(arg, "x64_regs") == 0) {
        print_cpu();
    } else if (strcmp(arg, "arm_regs") == 0) {
        print_native_cpu();
    } else if (strcmp(arg, "cache") == 0) {
        cache_print(break_block);
    } else {
        help();
    }
}
void debug_wait(void) {
    if (!enabled) return;
    char com[32];
    char arg[32];
    char line[256];
    while (1) {
        printf(" <- ");
        fgets(line, sizeof(line), stdin);
        if (sscanf(line, "%s %s", com, arg) == 2) {
            if (strcmp(com, "brb") == 0) {
                break_block = strtol(arg, NULL, 10);
                printf("Set break point in block %i\n", break_block);
            } else if (strcmp(com, "print") == 0) {
                handle_print(arg);
            }  else if (strcmp(com, "log") == 0) {
                set_log_level(arg[0]);
            } else {
                help();
            }
        } else {
            if (strcmp(com, "si") == 0) {
                panic("DEBUGGER::TODO");
            } else if (strcmp(com, "sb") == 0) {
                break_block++;
                return;
            } else if (strcmp(com, "run") == 0) {
                return;
            } else if (strcmp(com, "exit") == 0) {
                exit(0);
            } else {
                help();
            }
        }
    }
}
