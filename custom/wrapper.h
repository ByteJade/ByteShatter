#ifdef __aarch64__
#define WRAP_FUNC_VOID(func) \
    void my_##func() { \
        asm volatile("b " #func); \
        __builtin_unreachable(); \
    }
#define WRAP_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "stp x28, x30, [sp, #-16]!\n" \
            "bl " #func "\n" \
            "ldp x28, x30, [sp], #16\n" \
            "mov x9, x0\n" \
        ); \
    }
#define WRAP_MED_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "stp x28, x30, [sp, #-16]!\n" \
            "ldp x6, x7, [x28], #16\n" \
            "bl " #func "\n" \
            "ldp x28, x30, [sp], #16\n" \
            "mov x9, x0\n" \
        ); \
    }
#define WRAP_BIG_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "stp x28, x30, [sp, #-16]!\n" \
            "ldp x6, x7, [x28], #16\n" \
            "mov x20, sp\n" \
            "mov sp, x28\n" \
            "bl " #func "\n" \
            "mov sp, x20\n" \
            "ldp x28, x30, [sp], #16\n" \
            "mov x9, x0\n" \
        ); \
    }
#else
#define JUMP(func) \
    void my_##func() { \
        asm volatile("jmp " #func); \
        __builtin_unreachable(); \
    }

#define WRAP_FUNC_VOID(func)  JUMP(func)
#define WRAP_FUNC(func)  JUMP(func)
#define WRAP_MED_FUNC(func) JUMP(func)
#define WRAP_BIG_FUNC(func)    JUMP(func)
#endif