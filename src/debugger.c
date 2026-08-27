#include "debugger.h"
#include "core.h"
#include "cache.h"
#include "patcher.h"
#include "memory.h"
#include "decoder.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/mman.h>

static int enabled = 0;
static uint32_t prev_instr = 0;
static uint32_t* prev_instrp = NULL;
static uint32_t current_block = UINT32_MAX;
static uint32_t break_block = UINT32_MAX;
static uint64_t break_pc = UINT64_MAX;
static uint8_t* base;

void debug_enable(Elf* elf) {
    enabled = 1;
    break_block = 0;
    base = elf->base;
}
int debug_enabled(void) {
    return enabled;
}
void set_bp(uint32_t* instr) {
    prev_instr = *instr;
    prev_instrp = instr;
    *instr = 0xD4200000;
    __builtin___clear_cache(instr, instr+4);
    break_block = UINT32_MAX;
    break_pc = UINT64_MAX;
}
void debug_check_break() {
    CacheUnit* cache = cache_get_block(break_block);
    if (cache) {
        Anchor* anchor = cache_get_anchor(get_pc());
        set_bp(anchor->host + cache->hp);
        print("break block found");
    }
    uint32_t* instr = cache_search(break_pc);
    if (instr) {
        set_bp(instr);
        print("break pc found");
    }
}
void help(void) {
    printf("Commands:\n");
    printf("brb <i> - set break point in block i\n");
    printf("brk <imm64> - set break point in imm64\n");
    printf("sb - go to next block\n");
    printf("si - go to next instruction\n");
    printf("log <level> - set logs to level (A,W,E)\n");
    printf("print <state> - print state (flags, x64regs, regs, cache, usage)\n");
    printf("print [x64reg+imm] or (imm64) - print memory\n");
    printf("print <x64reg> - print register\n");
    printf("run - return to execution\n");
    printf("exit - stop execution\n");
}
int has_access(void* ptr) {
    memory_check_mode();
    uint64_t i = *(uint64_t*)ptr;
    if (memory_fail()) {
        printf("No access to memory %lx\n", i);
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
            cache_print(current_block);
        } else if (strcmp(arg, "flags") == 0) {
            print_flags();
        } else if (strcmp(arg, "usage") == 0) {
            Anchor* anchor = cache_get_anchor(get_pc());
            printf("cache usage: %i bytes\n", cache_usage());
            printf("host usage: %i bytes\n", anchor->host_p*4);
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
    Anchor* anchor = cache_get_anchor(get_pc());
    current_block = cache_search_block(get_pc());
    while (1) {
        printf(" <- ");
        fgets(line, sizeof(line), stdin);
        if (sscanf(line, "%s %s", com, arg) == 2) {
            if (strcmp(com, "brb") == 0) {
                break_block = strtol(arg, NULL, 10);
                printf("Set break point in block %X\n", break_block);
            } else if (strcmp(com, "brk") == 0) {
                break_pc = (uint64_t)base + strtol(arg, NULL, 16);
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
                CacheUnit* unit = cache_get_block(current_block);
                OffsetUnit* offsets = unit->offsets + cache_offsets();
                for (int x = 0; x < unit->offsetssz; x++) {
                    if ((uint32_t*)get_pc() - anchor->host == unit->hp + offsets[x].hoff) {
                        Context* context = context_pull(
                            ((uint64_t)anchor->gp_hi<<32) +
                            unit->gp_lo + offsets[x].goff
                        );
                        Instruction buf;
                        decode_instr(context, &buf);
                    }
                }
                set_bp((uint32_t*)(get_pc() + 4));
                break;
            } else if (strcmp(com, "sb") == 0) {
                break_block = current_block+1;
                break;
            } else if (strcmp(com, "run") == 0) {
                if (break_block != UINT32_MAX || break_pc != UINT64_MAX) 
                    printf("Stop at break point\n");
                break;
            } else if (strcmp(com, "exit") == 0) {
                _exit(0);
            } else {
                help();
            }
        }
    }
    debug_check_break();
}
