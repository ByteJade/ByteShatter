.include "./wrapper.inc"

.section .text
.global my___libc_start_main
.extern printf
.global my_printf

// just call main and return
my___libc_start_main:
    stp x29, x30, [sp, #-16]!
    mov x7, x0
    mov x0, x1
    mov x1, x2
    blr x7
    ldp x29, x30, [sp], #16
    ret

WRAP_BIG_FUNC printf
WRAP_FUNC exit
WRAP_FUNC puts
WRAP_FUNC malloc
WRAP_FUNC free
WRAP_FUNC strlen
WRAP_FUNC strstr
WRAP_FUNC getenv
