#define POP8\
    asm volatile(\
        "ldp x6, x7, [x28], #0x10\n"\
        : : : "x6", "x7", "x28", "memory")
#define WRAP_NORETURN(ret, name, ...) \
    __attribute__((naked, noreturn)) \
    ret my_##name(__VA_ARGS__) { \
        asm volatile("b " #name); \
    }