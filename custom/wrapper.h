#ifdef __aarch64__
#define WRAP_FUNC_VOID(func) \
    void my_##func() { \
        asm volatile("b " #func); \
    }
#define WRAP_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "mov x20, x30\n" \
            "bl " #func "\n" \
            "mov x30, x20\n" \
            "mov x9, x0\n" \
        ); \
    }
#define WRAP_MED_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "mov x20, x30\n" \
            "ldp x6, x7, [sp]\n" \
            "bl " #func "\n" \
            "mov x30, x20\n" \
            "mov x9, x0\n" \
        ); \
    }
#define WRAP_BIG_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "mov x20, x30\n" \
            "ldp x21, x22, [sp]\n" \
            "ldp x6, x7, [sp], #16\n" \
            "bl " #func "\n" \
            "stp x21, x22, [sp, #-16]!\n" \
            "mov x30, x20\n" \
            "mov x9, x0\n" \
        ); \
    }
#else
#define JUMP(func) \
    __attribute__((naked)) \
    void my_##func() { \
        asm volatile("jmp " #func); \
    }

#define WRAP_FUNC_VOID(func)  JUMP(func)
#define WRAP_FUNC(func)  JUMP(func)
#define WRAP_MED_FUNC(func) JUMP(func)
#define WRAP_BIG_FUNC(func)    JUMP(func)
#endif