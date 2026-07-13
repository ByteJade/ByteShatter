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
        ldp x20, x21, [x28]
        stp x20, x21, [sp, #-16]!
        ldp x20, x21, [x28, #16]
        stp x20, x21, [sp, #-16]!
        stp x28, x30, [sp, #-16]!
        ldp x6, x7, [x28]
        add x28, x28, #24
        mov x20, sp
        mov sp, x28
        bl \alias
        mov sp, x20
        ldp x28, x30, [sp], #16
        ldp x20, x21, [sp], #16
        stp x20, x21, [x28, #16]
        ldp x20, x21, [sp], #16
        stp x20, x21, [x28]
        ret
.endm
