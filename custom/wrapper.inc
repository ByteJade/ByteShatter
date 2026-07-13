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
        ldp x19, x20, [x28]
        stp x19, x20, [sp, #-16]!
        ldp x19, x20, [x28, #16]
        stp x19, x20, [sp, #-16]!
        stp x28, x30, [sp, #-16]!
        ldp x6, x7, [x28]
        add x28, x28, #24
        mov x19, sp
        mov sp, x28
        bl \alias
        mov sp, x19
        ldp x28, x30, [sp], #16
        ldp x19, x20, [sp], #16
        stp x19, x20, [x28]
        ldp x19, x20, [sp], #16
        stp x19, x20, [x28, #16]
        ret
.endm
