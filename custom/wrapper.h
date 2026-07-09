#define POP8\
    asm volatile(\
        "ldp x6, x7, [x28]\n"\
        : : : "x6", "x7", "memory")
#define WRAP(ret, name, ...) \
    __attribute__((naked)) \
    ret my_##name(__VA_ARGS__) { \
        asm volatile("b " #name); \
    }