.macro WRAP_FUNC alias
    .global my_\alias
    .extern \alias
    my_\alias:
        b \alias
.endm
.macro WRAP_BIG_FUNC alias
    .global my_\alias
    .extern \alias
    my_\alias:
        ldr x19, [x28], #8
        stp x19, x28, [sp, #-16]!
        stp x29, x30, [sp, #-16]!
        ldp x6, x7, [x28]
        add x28, x28, #16
        mov x19, sp
        mov sp, x28
        bl \alias
        mov sp, x19
        ldp x29, x30, [sp], #16
        ldp x19, x28, [sp], #16
        str x19, [x28]
        ret
.endm
