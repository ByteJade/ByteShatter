#include "debugger.h"
#include "core.h"
#include "cache.h"
#include "patcher.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int enabled = 0;
static int breakp = -1;

void debug_enable(void) {
    enabled = 1;
}
int block_break(void) {
    return breakp;
}
void help(void) {
    printf("Commands:\n");
    printf("brb <i> - set break point in block i\n");
    printf("sb - go to next block\n");
    printf("si - go to next instruction\n");
    printf("log <level> - set logs to level (A,W,E)\n");
    printf("print <state> - print state (x64_regs, arm_regs, cache)\n");
    printf("continue - return to execution\n");
    printf("exit - stop execution\n");
}
void handle_print(char* arg) {
    if (strcmp(arg, "x64_regs") == 0) {
        print_cpu();
    } else if (strcmp(arg, "arm_regs") == 0) {
        print_native_cpu();
    } else if (strcmp(arg, "cache") == 0) {
        cache_print(breakp);
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
                breakp = strtol(arg, NULL, 10);
            } else if (strcmp(com, "print") == 0) {
                handle_print(arg);
            }  else if (strcmp(com, "log") == 0) {
                set_log_level(com[0]);
            } else {
                help();
            }
        } else {
            if (strcmp(com, "si") == 0) {
                panic("DEBUGGER::TODO");
            } else if (strcmp(com, "sb") == 0) {
                breakp++;
                return;
            } else if (strcmp(com, "continue") == 0) {
                return;
            } else if (strcmp(com, "exit") == 0) {
                exit(0);
            } else {
                help();
            }
        }
    }
}
