#include "debugger.h"
#include "core.h"
#include "cache.h"
#include "patcher.h"
#include "memory.h"
#include "decoder.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

static int enabled = 0;
uint64_t breakp = 0;
uint64_t break_pc = 0;
uint64_t break_block = -1;
static uint64_t break_point = 0;
static uint32_t prev_instr = 0;
static uint32_t* prev_instrp = NULL;

void debug_enable(void) {
    enabled = 1;
}
int debug_break(void) {
    return break_block;
}
uint64_t debug_breakp(void) {
    return breakp;
}
void set_break_point(uint32_t pc) {
    break_pc = pc;
    CacheUnit* cache = cache_get_block(break_block);
    uint32_t* instr = (uint32_t*)(get_host() + cache->hp + pc);
    prev_instr = *instr;
    prev_instrp = instr;
    *instr = 0xD4200000;
    __builtin___clear_cache(instr, instr+4);
}
void set_break() {
    break_pc = 0;
    uint32_t* instr = (uint32_t*)(get_host() + get_hp() - 4);
    prev_instr = *instr;
    prev_instrp = instr;
    *instr = 0xD4200000;
    __builtin___clear_cache(instr, instr+4);
}
void help(void) {
    printf("Commands:\n");
    printf("brb <i> - set break point in block i\n");
    printf("brk <imm64> - set break point in imm64\n");
    printf("sb - go to next block\n");
    printf("si - go to next instruction\n");
    printf("log <level> - set logs to level (A,W,E)\n");
    printf("print <state> - print state (flags, x64regs, regs, cache)\n");
    printf("print [x64reg+imm] or (imm64) - print memory\n");
    printf("print <x64reg> - print register\n");
    printf("run - return to execution\n");
    printf("exit - stop execution\n");
}
int has_access(void* ptr) {
    memory_check_mode();
    char i = *(char*)ptr;
    if (memory_fail()) {
        printf("No access to memory\n");
        return 0;
    }
    return 1;
}
void handle_print(char* arg) {
    if (arg[0] == '(') {
        uint64_t* imm = (uint64_t*)strtol(arg+1, NULL, 16);
        if (has_access(imm))
            printf("\033[34m%s\033[0m: %lX\n", arg, *imm);
    } else if (arg[0] == '[') {
        char* ptr = arg+1;
        int p = 0;
        char reg[4];
        char sign = ']';
        reg[3] = '\0';
        while(1) {
            char c = *ptr++;
            if (c == '-' || c == '+' || c == ']'){
                sign = c;
                break;
            }
            reg[p++] = c;
        }
        uint64_t imm = 0;
        if (sign != ']') imm = strtol(ptr, NULL, 16);
        uint64_t base = get_reg(reg);
        if (sign == '+') base += imm;
        else if (sign == '-') base -= imm;
        if (has_access((void*)base))
            printf("\033[34m%s\033[0m: %lX\n", arg, *(uint64_t*)base);
    } else {
        if (strcmp(arg, "x64regs") == 0) {
            print_cpu();
        } else if (strcmp(arg, "regs") == 0) {
            print_native_cpu();
        } else if (strcmp(arg, "cache") == 0) {
            cache_print(break_block);
        } else if (strcmp(arg, "flags") == 0) {
            print_flags();
        } else {
            printf("\033[34m%s\033[0m: %lX\n", arg, get_reg(arg));
        }
    }
}
void debug_wait(void) {
    if (!enabled) return;
    char com[32];
    char arg[32];
    char line[256];
    if (prev_instrp) {
        *prev_instrp = prev_instr;
        __builtin___clear_cache(prev_instrp, prev_instrp+4);
        prev_instrp = NULL;
    }
    while (1) {
        printf(" <- ");
        fgets(line, sizeof(line), stdin);
        if (sscanf(line, "%s %s", com, arg) == 2) {
            if (strcmp(com, "brb") == 0) {
                break_block = strtol(arg, NULL, 10);
                printf("Set break point in block %li\n", break_block);
            } else if (strcmp(com, "brk") == 0) {
                breakp = strtol(arg, NULL, 16);
                printf("Set break point in pc %lX\n", break_pc);
            } else if (strcmp(com, "print") == 0) {
                handle_print(arg);
            }  else if (strcmp(com, "log") == 0) {
                set_log_level(arg[0]);
            } else {
                help();
            }
        } else {
            if (strcmp(com, "si") == 0) {
                CacheUnit* unit = cache_get_block(break_block);
                for (int x = 0; x < unit->offsetssz; x++) {
                    if (break_pc == unit->offsets[x].hoff*4) {
                        X64_instruction buf;
                        set_gp(unit->gp + unit->offsets[x].goff);
                        decode_instr(&buf);
                        char out[32];
                        sprint_instr(out, &buf);
                        printf("%s\n", out);
                    }
                }
                set_break_point(break_pc + 4);
                break;
            } else if (strcmp(com, "sb") == 0) {
                break_block++;
                break;
            } else if (strcmp(com, "run") == 0) {
                if (break_block != -1) 
                    printf("Stop at break point\n");
                break;
            } else if (strcmp(com, "exit") == 0) {
                exit(0);
            } else {
                help();
            }
        }
    }
}
