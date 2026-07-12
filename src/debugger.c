#include "debugger.h"
#include "core.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int enabled = 0;
static int breakp = -1;

void debug_enable() {
    enabled = 1;
}
int block_break() {
    return breakp;
}
void help() {
    printf("Commands:\n");
    printf("brb <i> - set break point in block i\n");
    printf("sb - go to next block\n");
    printf("si - go to next instruction\n");
    printf("print <state> - print state (x64_regs, arm_regs, cache)\n");
    printf("continue - return to execution\n");
}
void debug_wait() {
    if (!enabled) return;
    char com[32];
    char arg[32];
    char line[256];
    scanf("%s", line);
    if (sscanf(line, "%s %s", com, arg) == 2) {
        if (strcmp(com, "brb") == 0) {
            breakp = strtol(arg, NULL, 10);
        } else if (strcmp(com, "print") == 0) {
            panic("DEBUGGER::TODO");
        } else {
            help();
        }
    } else {
        if (strcmp(com, "si") == 0) {
            panic("DEBUGGER::TODO");
        } else if (strcmp(com, "sb") == 0) {
            breakp++;
        } else if (strcmp(com, "continue") == 0) {
            return;
        } else {
            help();
        }
    }
}
