#define _GNU_SOURCE
#include "elf_manager.h"
#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <link.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct {
    void* wrapper;
    const char* name;
} Lib;

Lib* libs = NULL;
int lib_count = 0;
int lib_capacity = 0;

const char* ld_paths[] = {
    ".",
    "/lib",
    "/lib64",
    "/system/lib64",
    "/data/data/com.termux/files/usr/lib",
    NULL
};

void open_library(const char* filename) {
    for (int i = 0; i < lib_count; i++) {
        Lib* lib = libs + i;
        if (strcmp(lib->name, filename) == 0) return;
    }
    if (lib_count >= lib_capacity) {
        lib_capacity += 4;
        libs = realloc(libs, lib_capacity * sizeof(Lib));
    }
    char fullpath[512];
    Lib* lib = libs + lib_count;
    snprintf(
        fullpath, sizeof(fullpath),
        "./lib/my_%s", filename
    );
    lib->wrapper = dlopen(fullpath, RTLD_NOW);
    if (!lib->wrapper) {
        print("%s", dlerror());
        panic("WRAPPER_NOT_FOUND %s", fullpath);
    }
    success("Wrap %s", fullpath);
    lib->name = strdup(filename);
    lib_count++;
}
char* get_symbol(const char* symbol) {
    void* sym = dlsym(RTLD_DEFAULT, symbol);
    return sym;
}
char* get_cpp_symbol(const char* symbol) {
    static const char* symbols[] = {
        "_ZSt4cout",
        NULL
    };
    static const char* targets[] = {
        "_ZNSt3__14coutE",
        NULL
    };
    for (int position = 0; symbols[position]; position++) {
        if (strcmp(symbol, symbols[position]) == 0) {
            return get_symbol(targets[position]);
        }
    }
    return NULL;
}
char* get_symbol_wrapped(const char* symbol) {
    char my_symbol[512];
    snprintf(
        my_symbol, sizeof(my_symbol),
        "my_%s", symbol
    );
    for (int i = 0; i < lib_count; i++) {
        Lib* lib = libs + i;
        void* sym = dlsym(lib->wrapper, my_symbol);
        if (sym) return sym;
    }
    return NULL;
}
typedef struct {
    const char* symbol;
    void* new_got;
} Search_data;
void* get_ptr(uint64_t base, uint64_t link) {
    if (link < base) return (void*)(base + link);
    return (void*)link;
}
static int patch_library(struct dl_phdr_info* info, size_t size, void* data) {
    ((void)size);
    Search_data* search = (Search_data*)data;
    Elf64_Addr base = info->dlpi_addr;
    Elf64_Dyn* dyn = NULL;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const Elf64_Phdr* phdr = &info->dlpi_phdr[i];
        if (phdr->p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn*)(info->dlpi_addr + phdr->p_vaddr);
            break;
        }
    }

    const char* strtab = NULL;
    Elf64_Sym* symtab = NULL;
    Elf64_Rela *rela = NULL;
    size_t rela_size = 0;
    for (; dyn->d_tag != DT_NULL; ++dyn) {
        switch (dyn->d_tag) {
            case DT_STRTAB:
                strtab = (const char*)get_ptr(base, dyn->d_un.d_ptr);
                break;
            case DT_SYMTAB:
                symtab = (Elf64_Sym*)get_ptr(base, dyn->d_un.d_ptr);
                break;
            case DT_RELA:
                rela = (Elf64_Rela*)get_ptr(base, dyn->d_un.d_ptr);
                break;
            case DT_RELASZ:
                rela_size = dyn->d_un.d_val;
                break;
        }
    }
    if (!(strtab && symtab && rela)) return 0;
    size_t count = rela_size / sizeof(Elf64_Rela);
    for (size_t i = 0; i < count; i++) {
        uint32_t idx = ELF64_R_SYM(rela[i].r_info);
        uint32_t type = ELF64_R_TYPE(rela[i].r_info);
        Elf64_Addr* patch = (Elf64_Addr*)(base + rela[i].r_offset);
        if (type == R_X86_64_JUMP_SLOT || type == R_X86_64_GLOB_DAT ||
            type == R_AARCH64_JUMP_SLOT || type == R_AARCH64_GLOB_DAT) {
            const char* name = strtab + symtab[idx].st_name;
            if (strcmp(name, search->symbol) == 0) {
                print("patch %s", name);
                void* page_start = (void*)(((uint64_t)patch) & ~(0x1000 - 1));
                mprotect(page_start, 0x1000, PROT_READ | PROT_WRITE);
                *patch = (Elf64_Addr)search->new_got;
            }
        }
    }
    return 0;
}
void patch_library_got(const char* symbol, void* new_got) {
    Search_data data = {symbol, new_got};
    dl_iterate_phdr(patch_library, &data);
}