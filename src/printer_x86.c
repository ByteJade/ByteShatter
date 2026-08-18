#include "printer_x86.h"
#include "decoder.h"
#include <stdio.h>

const char* instr_types[] = {
    "add", "or", "adc", "sbb",
    "and", "sub", "xor", "cmp",

    "rol", "ror", "rcl", "rcr",
    "shl", "shr", "sal", "sar",
    
    "push", "pop", "movsx", "mov",

    "jo", "jno", "jb", "jae",
    "je", "jne", "jbe", "ja",
    "js", "jns", "jp", "jpo",
    "jl", "jge", "jle", "jg",

    "cmovo", "cmovno", "cmovb", "cmovae",
    "cmove", "cmovne", "cmovbe", "cmova",
    "cmovs", "cmovns", "cmovp", "cmovpo",
    "cmovl", "cmovge", "cmovle", "cmovg",

    "seto", "setno", "setb", "setae",
    "sete", "setne", "setbe", "seta",
    "sets", "setns", "setp", "setpo",
    "setl", "setge", "setle", "setg",

    "test", "lea", "nop", "ebr64",
    "ret", "leave", "call", "jmp",

    "pxor", "adds", "muls",
    "cvtss2sd", "cvtsd2ss",
    "subs", "divs",
    "comis", "movs",
    "movq", "movapd",
    "cvtsd2si", "cvtsi2s",
    "movzx", "idiv", "imul",
    "cltq", "cltd"
};
const char* regs64[] = {
    "rax", "rcx", "rdx", "rbx",
    "rsp", "rbp", "rsi", "rdi",
    "r8", "r9", "r10", "r11",
    "r12", "r13", "r14", "r15",
};
const char* regs32[] = {
    "eax", "ecx", "edx", "ebx",
    "esp", "ebp", "esi", "edi",
    "r8d", "r9d", "r10d", "r11d",
    "r12d", "r13d", "r14d", "r15d",
};
const char* regs16[] = {
    "ax", "cx", "dx", "bx",
    "sp", "bp", "si", "di",
    "r8w", "r9w", "r10w", "r11w",
    "r12w", "r13w", "r14w", "r15w",
};
const char* regs8[] = {
    "al", "cl", "dl", "bl",
    "ah", "ch", "dh", "bh",
    "r8b", "r9b", "r10b", "r11b",
    "r12b", "r13b", "r14b", "r15b",
};
const char* scale[] = {
    "", "* 2 ", "* 4 ", "* 8 ",
};
char* sprint_op(Instruction* buf, char* out, Operand* op) {
    if (op->type == REG) {
        if (buf->size == 64) 
            out += sprintf(out, "%s ", regs64[op->reg]);
        else if (buf->size == 32) 
            out += sprintf(out, "%s ", regs32[op->reg]);
        else if (buf->size == 16) 
            out += sprintf(out, "%s ", regs16[op->reg]);
        else out += sprintf(out, "%s ", regs8[op->reg]);
    } else if (op->type == IMM) {
        if (op->imm < 0)
            out += sprintf(out, "-%lx ", -op->imm);
        else out += sprintf(out, "%lx ", op->imm);
    } else if (op->type == (REG|XMM)) {
        out += sprintf(out, "xmm%i ", op->reg);
    } else {
        out += sprintf(out, "[ ");
        if (op->type&REG) {
            out += sprintf(out, "%s ", regs64[op->reg]);
            if (op->type&IDX) out += sprintf(out, "+ ");
        }
        if (op->type&IDX) {
            if (buf->prefix == FS)
                out += sprintf(out, "fs ");
            else out += sprintf(out, "%s ", regs64[op->idx]);
            out += sprintf(out, "%s", scale[op->scale]);
        }
        if (op->type&IMM) {
            if (op->type == (MEM|IMM)) {
                if (buf->prefix == FS)
                    out += sprintf(out, "fs ");
                else out += sprintf(out, "rip ");
            }
            if (op->imm < 0)
                out += sprintf(out, "- %llx ", -op->imm);
            if (op->imm > 0)
                out += sprintf(out, "+ %llx ", op->imm);
        }
        out += sprintf(out, "] ");
    }
    return out;
}

void sprint_x86_64(Instruction* buf, char* out) {
    out += sprintf(
        out, "\033[34m%s \033[32m",
        instr_types[buf->type]
    );
    if (buf->a.type) {
        out = sprint_op(buf, out, &buf->a);
        if (buf->b.type) {
            out = sprint_op(buf, out, &buf->b);
        }
    }
    sprintf(out, "\033[0m");
}