.section .text
.global my___libc_start_main
.extern printf
.global my_printf

.macro WRAP_FUNC alias
    .global my_\alias
    .extern \alias
    my_\alias:
        b \alias
.endm

// just call main and return
my___libc_start_main:
    stp x29, x30, [sp, #-16]!
    mov x7, x0
    mov x0, x1
    mov x1, x2
    blr x7
    ldp x29, x30, [sp], #16
    ret

my_printf:
    str x19, [x28], #8
    stp x19, x28, [sp, #-16]!
    stp x29, x30, [sp, #-16]!
    ldp x6, x7, [x28]
    add x28, x28, #16
    mov x19, sp
    mov sp, x28
    bl printf
    mov sp, x19
    ldp x29, x30, [sp], #16
    ldp x19, x28, [sp], #16
    ldr x19, [x28]
    ret

WRAP_FUNC exit
WRAP_FUNC puts
WRAP_FUNC malloc
WRAP_FUNC free
WRAP_FUNC strlen
WRAP_FUNC strstr
WRAP_FUNC getenv
