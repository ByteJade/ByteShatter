#include "dlmanager.h"
#include "loader.h"
#include "core.h"
#include "memory.h"
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>

static const char* ld_paths[] = {
    "./",
    "/lib/",
    "/lib64/",
    "/usr/lib/",
    "/usr/lib64/",
    "/lib/x86_64-linux-gnu/",
#ifdef __ANDROID__
    "/data/data/com.termux/files/usr/lib/",
#endif
    NULL
};
typedef struct {
    const char* name;
    void* data;
} Library;
Library libs[16];
int libs_count = 0;

ExeMeta* files [3];
int fp = 0;

int is_external_offset(uint32_t offset) {
    ExeMeta* exe = files[0];
    ElfMeta* elf = exe->elf;
    for (int i = 0; i < elf->header.e_shnum; i++) {
        Elf64_Shdr* header = elf->sheaders + i;
        if (offset >= header->sh_addr && offset < header->sh_addr + header->sh_size) {
            const char* shname = elf->shstrtab + header->sh_name;

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
            }
            print("in %x: %x", offset, *(uint64_t*)(get_guest()+offset));
            return 0;
        }
    }
    panic("not found %x", offset);
    return 0;
}
ExeMeta* load_object(const char* filename) {
    ExeMeta* exe = loader_open_elf(filename);
    files[fp++] = exe;
    loader_map_segments(exe);
    loader_reloc_dependencies(exe);
    return exe;
}
void load_library(const char* filename) {
    for (int i = 0; i < libs_count; i++) {
        if (strcmp(filename, libs[i].name) == 0) return;
    }
    void* lib = NULL;
    for (int i = 0; ld_paths[i]; i++) {
        char fullpath[1024];
        snprintf(
            fullpath, sizeof(fullpath),
            "%s%s", ld_paths[i], filename
        );
        lib = dlopen(fullpath, RTLD_NOW);
        if (lib) {
            printf("load: %s\n", fullpath);
            libs[libs_count].name = strdup(filename);
            libs[libs_count].data = lib;
            libs_count++;
            break;
        }
    }
}
void* get_symbol(const char* symname) {
    for (int i = 0; i < libs_count; i++) {
        void* sym = dlsym(libs[i].data, symname);
        if (sym) return sym;
    }
    warning("No symbol: %s", symname);
    return NULL;
}