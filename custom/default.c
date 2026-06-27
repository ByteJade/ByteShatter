#include <stdlib.h>
#include <stdint.h>

int __libc_start_main(
    int (*main)(int, char**, char**),
    int argc,
    char** argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void* stack_end
) {
    if (init) init();
    int result = main(argc, argv, NULL);
    if (fini) fini();
    return result;
}

// Экспортируем символ
__attribute__((visibility("default"))) 
int __libc_start_main(
    int (*main)(int, char**, char**),
    int argc,
    char** argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void* stack_end
);