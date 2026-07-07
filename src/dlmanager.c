#include "dlmanager.h"
#include "loader.h"
#include "core.h"
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>

static const char* ld_paths[] = {
    ".",
    "/lib",
    "/lib64",
    "/usr/lib",
    "/usr/lib64",
    "/lib/aarch64-linux-gnu",
#ifdef __ANDROID__
    "/data/data/com.termux/files/usr/lib",
#endif
    NULL
};
typedef struct {
    const char* name;
    ExeMeta* wrapped;
    void* native;
} Library;

Library libs[16];
int libs_count = 0;

uint32_t my_hash(const char* str) {
    uint32_t hash = 5381;
    while (*str) {
        hash = (hash << 5) + hash + *str;
        str++;
    }
    return hash;
}

int is_external_offset(uint32_t offset) {
    ExeMeta* exe = libs[0].wrapped;
    ElfMeta* elf = exe->elf;
    for (int i = 0; i < elf->header.e_shnum; i++) {
        Elf64_Shdr* header = elf->sheaders + i;
        uint64_t start = header->sh_addr;
        uint64_t end = start + header->sh_size;
        if (offset >= start && offset < end) {
            const char* shname = elf->shstrtab + header->sh_name;
            print("in %x: %lx", offset, *(uint64_t*)(exe->base+offset));
            if (strcmp(shname, ".got") == 0 ||
                     strcmp(shname, ".got.plt") == 0 ||
                     strcmp(shname, ".plt") == 0) {
                print("→ external!");
                return 1;
            }else if (header->sh_type == SHT_PROGBITS && 
                (header->sh_flags & SHF_EXECINSTR)) {
                print("→ internal func");
            }else {
                print("→ internal data");
                return -1;
            }
            return 0;
        }
    }
    panic("not found %x", offset);
    return 0;
}
ExeMeta* load_object(const char* filename) {
    ExeMeta* exe = loader_open_elf(filename);
    if (exe) {
        libs[libs_count].name = strdup(strrchr(filename, '/')+1);
        libs[libs_count].wrapped = exe;
        libs[libs_count].native = NULL;
        libs_count++;
        loader_map_segments(exe);
        loader_reloc_dependencies(exe);
    }
    return exe;
}
void load_wrapped_library(const char* filename) {
    for (int i = 0; i < libs_count; i++) {
        if (strcmp(filename, libs[i].name) == 0) { 
            return;
        }
    }
    char fullpath[1024];
    snprintf(
        fullpath, sizeof(fullpath),
        "./my_%s", filename
    );
    ExeMeta* exe = load_object(fullpath);
    if (!exe) {
        panic("DLMANAGER::NO_WRAPPER: %s", filename);
    }
}
void load_native_library(const char* filename) {
    for (int i = 0; i < libs_count; i++) {
        if (strcmp(filename, libs[i].name) == 0) { 
            return;
        }
    }
    void* lib = NULL;
    for (int i = 0; ld_paths[i]; i++) {
        char fullpath[1024];
        snprintf(
            fullpath, sizeof(fullpath),
            "%s/%s", ld_paths[i], filename
        );
        lib = dlopen(fullpath, RTLD_LAZY | RTLD_GLOBAL);
        if (lib) {
            success("load: %s", fullpath);
            libs[libs_count-1].native = lib;
            return;
        }
    }
    warning("No library: %s", filename);
}
void* get_native_symbol(const char* symname) {
    // damn __libc_start_main is not accepted to be used in aarch64
    // so it's not here
    void* sym = dlsym(RTLD_DEFAULT, symname);
    if (sym) return sym;
    for (int i = 1; i < libs_count; i++) {
        void* sym = dlsym(libs[i].native, symname);
        if (sym) return sym;
    }
    warning("No native symbol: %s", symname);
    return NULL;
}
void* get_wrapped_symbol(const char* symname) {
    char fullname[1024];
    snprintf(
        fullname, sizeof(fullname),
        "my_%s", symname
    );
    uint32_t hash = my_hash(fullname);
    for (int i = 1; i < libs_count; i++) {
        ExeMeta* exe = libs[i].wrapped;
        ElfMeta* elf = exe->elf;
        char* symtab_str = elf->strtab; 
        for (size_t j = 1; j < elf->dynsymsz; j++) {
            Elf64_Sym* sym = &elf->dynsym[j];
            
            if (elf->sym_cache[j] == hash) {
                const char* sym_name = symtab_str + sym->st_name;
                if (strcmp(sym_name, fullname) == 0) {
                    print("found %s in %s", fullname, libs[i].name);
                    return exe->base + sym->st_value;
                }
            }
        }
    }
    warning("No wrapped symbol: %s", fullname);
    return NULL;
}