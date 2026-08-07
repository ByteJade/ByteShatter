#ifdef __aarch64__
#define WRAP_FUNC_VOID(func) \
    void my_##func() { \
        asm volatile("b " #func); \
        __builtin_unreachable(); \
    }
#define WRAP_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "mov x27, x30\n" \
            "bl " #func "\n" \
            "mov x30, x27\n" \
            "mov x9, x0\n" \
        ); \
    }
#define WRAP_MED_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "mov x27, x30\n" \
            "ldp x6, x7, [x28]\n" \
            "bl " #func "\n" \
            "mov x30, x27\n" \
            "mov x9, x0\n" \
        ); \
    }
#define WRAP_BIG_FUNC(func) \
    void my_##func() { \
        asm volatile( \
            "mov x27, x30\n" \
            "ldp x6, x7, [x28], #16\n" \
            "mov sp, x28\n" \
            "bl " #func "\n" \
            "sub x28, #16\n" \
            "mov x30, x27\n" \
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