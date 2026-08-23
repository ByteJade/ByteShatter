#include "elf_loader.h"
#include "cache.h"
#include "elf_manager.h"
#include "core.h"
#include "memory.h"
#include "patcher.h"
#include "overrides.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define PAGE_SIZE 0x1000

void mmap_base(Elf* elf) {
    size_t max = 0;
    size_t min = SIZE_MAX;
    for (int i = 0; i < elf->head.e_phnum; i++) {
        Elf64_Phdr* phdr = elf->pheads + i;
        if (phdr->p_type == PT_LOAD) {
            size_t start = phdr->p_vaddr;
            size_t end = start + phdr->p_memsz;
            if (start < min) min = start;
            if (end > max) max = end;
        }
    }
    min &= ~(PAGE_SIZE - 1);
    max = (max + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uint32_t size = max - min;
    elf->base = (uint8_t*)mmap_guest(size+size);
    uint32_t* host = (uint32_t*)(elf->base + size);
    elf->base -= min;
    mprotect(host, size, PROT_READ | PROT_WRITE | PROT_EXEC);
    cahce_init((uint64_t)elf->base, host, size);
}

Elf* elf_load(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) return NULL;
    Elf* elf = (Elf*)calloc(1, sizeof(Elf));
    fread(&elf->head, sizeof(Elf64_Ehdr), 1, fp);
    elf->pheads = (Elf64_Phdr*)malloc(sizeof(Elf64_Phdr) * elf->head.e_phnum);
    elf->sheads = (Elf64_Shdr*)malloc(sizeof(Elf64_Shdr) * elf->head.e_shnum);
    fseek(fp, elf->head.e_phoff, SEEK_SET);
    fread(elf->pheads, sizeof(Elf64_Phdr), elf->head.e_phnum, fp);
    fseek(fp, elf->head.e_shoff, SEEK_SET);
    fread(elf->sheads, sizeof(Elf64_Shdr), elf->head.e_shnum, fp);
    mmap_base(elf);
    for (int i = 0; i < elf->head.e_phnum; i++) {
        Elf64_Phdr* phdr = elf->pheads + i;
        if (phdr->p_type == PT_LOAD) {
            uint8_t* dst = elf->base + phdr->p_vaddr;
            fseek(fp, phdr->p_offset, SEEK_SET);
            fread(dst, 1, phdr->p_filesz, fp);
            if (phdr->p_filesz != phdr->p_memsz) {
                memset(dst + phdr->p_filesz, 0,
                    phdr->p_memsz - phdr->p_filesz
                );
            }
        }
    }
    fclose(fp);
    return elf;
}
void elf_close(Elf* elf) {
    free(elf->sheads);
    free(elf->pheads);
    free(elf);
}
void reloc_relr(Elf* elf, Elf64_Relr* relr, int relrsz) {
    size_t count = relrsz / sizeof(Elf64_Addr);
    Elf64_Addr *where = NULL;
    for (size_t i = 0; i < count; i++) {
        Elf64_Addr entry = relr[i];
        if (!(entry & 1)) {
            where = (Elf64_Addr *)(elf->base + entry);
            *where++ += (Elf64_Addr)elf->base;
        } else {
            for (long i = 0; (entry >>= 1) != 0; i++) {
                if (entry&1) where[i] += (Elf64_Addr)elf->base;
            } where += 63;
        }
    }
}
void reloc_rela(Elf* elf, Elf64_Rela* rela, int relasz) {
    int allright = 1;
    for (size_t i = 0; i < relasz / sizeof(Elf64_Rela); i++) {
        Elf64_Rela* rel = rela + i;
        int type = ELF64_R_TYPE(rel->r_info);
        int sym_idx = ELF64_R_SYM(rel->r_info);
        Elf64_Addr* patch = (Elf64_Addr*)(elf->base + rel->r_offset);
        Elf64_Sym* sym = elf->symtab + sym_idx;
        const char* symname = elf->strtab + sym->st_name;
        switch(type) {
            case R_X86_64_NONE: break;
            case R_X86_64_64:
                *patch = (Elf64_Addr)(sym->st_value + rel->r_addend);
                break;
            case R_X86_64_RELATIVE:
                *patch = (Elf64_Addr)(elf->base + rel->r_addend);
                break;
            case R_X86_64_JUMP_SLOT:
            case R_X86_64_GLOB_DAT: {
                void *sym_addr = get_symbol_wrapped(symname);
                if (!sym_addr) sym_addr = sym_override(symname);
                if (!sym_addr) {
                    warning("LOADER::USING_NATIVE %s", symname);
                    sym_addr = get_symbol(symname);
                }
                if (sym_addr) {
                    *patch = (Elf64_Addr)sym_addr;
                } else if (ELF64_ST_BIND(sym->st_info) == STB_WEAK) {
                    *patch = 0;
                } else {
                    warning("LOADER::UNDEFINED_GLOB_SYMBOL %s", symname);
                    allright = 0;
                }
            } break;
            case R_X86_64_COPY: {
                void *sym_addr = get_symbol(symname);
                size_t size = sym->st_size;
                if (sym_addr && size) {
                    memmove(patch, sym_addr, size);
                    patch_library_got(symname, patch);
                } else {
                    warning("LOADER::UNDEFINED_SYMBOL %s", symname);
                    allright = 0;
                }
            } break;
            default:
                warning("Unknown RELA %i", type);
                allright = 0;
        }
    }
    if (!allright) panic("LOADER::RELA::ERROR");
}
void elf_read_dynamic(Elf* elf) {
    Elf64_Phdr* dyn_phdr = NULL;
    for (int i = 0; i < elf->head.e_phnum; i++) {
        Elf64_Phdr* phdr = elf->pheads + i;
        if (phdr->p_type == PT_DYNAMIC) {
            dyn_phdr = phdr;
            break;
        }
    }
    if (dyn_phdr == NULL) panic("LOADER::NO_DYNAMIC_SECTION");
    Elf64_Dyn* dyn = (Elf64_Dyn*)(elf->base + dyn_phdr->p_vaddr);
    size_t relrsz = 0;
    Elf64_Relr* relr = NULL;
    size_t relasz = 0;
    Elf64_Rela* rela = NULL;
    size_t jmprelsz = 0;
    Elf64_Rela* jmprel = NULL;
    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
            case DT_PLTRELSZ:
                jmprelsz = dyn->d_un.d_val;
                break;
            case DT_STRTAB:
                elf->strtab = (char*)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_SYMTAB:
                elf->symtab = (Elf64_Sym*)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_RELA:
                rela = (Elf64_Rela*)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_RELASZ:
                relasz = dyn->d_un.d_val;
                break;
            case DT_INIT:
                elf->init = elf->base + dyn->d_un.d_ptr;
                break;
            case DT_JMPREL:
                jmprel = (Elf64_Rela*)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_INIT_ARRAY:
                elf->init_array = (uint8_t**)(elf->base + dyn->d_un.d_ptr);
                break;
            case DT_INIT_ARRAYSZ:
                elf->init_arraysz = dyn->d_un.d_val;
                break;
            case DT_RELRSZ:
                relrsz = dyn->d_un.d_val;
                break;
            case DT_RELR:
                relr = (Elf64_Relr*)(elf->base + dyn->d_un.d_ptr);
                break;
        }
    }
    dyn = (Elf64_Dyn*)(elf->base + dyn_phdr->p_vaddr);
    for (; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_NEEDED) {
            open_library(elf->strtab + dyn->d_un.d_val);
        }
    }
    if (relr) reloc_relr(elf, relr, relrsz);
    if (rela) reloc_rela(elf, rela, relasz);
    if (jmprel) reloc_rela(elf, jmprel, jmprelsz);
}
void elf_init(Elf* elf) {
    if (elf->init) {
        execute_with_save((uint64_t)elf->init);
    }
    if (elf->init_array) {
        size_t count = elf->init_arraysz / sizeof(Elf64_Addr);
        for (size_t i = 0; i < count; i++) {
            print("jump init_array[%i] (%p)", i, elf->init_array);
            execute_with_save((uint64_t)elf->init_array[i]);
        }
    }
    cache_clear();
}