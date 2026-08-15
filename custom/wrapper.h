#ifdef __aarch64__
#define WRAP_FUNC_VOID(func) \
    __attribute__((naked, used)) \
    void my_##func() { \
        __asm__ volatile("b " #func); \
    }
#define WRAP_FUNC(func) \
    void my_##func() { \
        __asm__ volatile( \
            "mov x20, x30\n" \
            "bl " #func "\n" \
            "mov x30, x20\n" \
            "mov x9, x0\n" \
        ); \
    }
#define WRAP_MED_FUNC(func) \
    void my_##func() { \
        __asm__ volatile( \
            "mov x20, x30\n" \
            "ldp x6, x7, [x28]\n" \
            "bl " #func "\n" \
            "mov x30, x20\n" \
            "mov x9, x0\n" \
        ); \
    }
#define WRAP_BIG_FUNC(func) \
    void my_##func() { \
        __asm__ volatile( \
            "stp x28, x30, [sp, #-16]!\n" \
            "ldp x21, x22, [x28]\n" \
            "ldp x6, x7, [x28], #16\n" \
            "mov x20, sp\n" \
            "mov sp, x28\n" \
            "bl " #func "\n" \
            "mov sp, x20\n" \
            "ldp x28, x30, [sp], #16\n" \
            "stp x21, x22, [x28]\n" \
            "mov x9, x0\n" \
        ); \
    }
#define RETURN(state) __asm__ volatile("mov x9, x0");
#else
#define JUMP(func) \
    __attribute__((naked)) \
    void my_##func() { \
        __asm__ volatile("jmp " #func); \
    }

#define WRAP_FUNC_VOID(func)  JUMP(func)
#define WRAP_FUNC(func)  JUMP(func)
#define WRAP_MED_FUNC(func) JUMP(func)
#define WRAP_BIG_FUNC(func)    JUMP(func)
#define RETURN(state) __asm__ volatile( "" );
#endif