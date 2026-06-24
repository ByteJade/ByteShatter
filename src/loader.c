#include "loader.h"
#include "core.h"
#include "memory.h"
#include "dlmanager.h"
#include <elf.h>
#include <stdlib.h>
#include <string.h>

ExeMeta* loader_open_elf(const char* filename) {
    ExeMeta* exe = (ExeMeta*)malloc(sizeof(ExeMeta));
    ElfMeta* elf = (ElfMeta*)malloc(sizeof(ElfMeta));
    elf->fp = fopen(filename, "rb");
    if (elf->fp == NULL) {
        panic("LOADER::FILE_OPENING_ERROR: %s", filename);
    }
    fread(
        &elf->header, sizeof(Elf64_Ehdr),
        1, elf->fp
    );
    if (memcmp(elf->header.e_ident, ELFMAG, SELFMAG)) {
        panic("LOADER::THIS_IS_NOT_ELF: %s", filename);
    }
    elf->pheaders = (Elf64_Phdr*)malloc(
        sizeof(Elf64_Phdr) * elf->header.e_phnum
    );
    elf->sheaders = (Elf64_Shdr*)malloc(
        sizeof(Elf64_Shdr) * elf->header.e_shnum
    );
    fseek(elf->fp, elf->header.e_phoff, SEEK_SET);
    fread(
        elf->pheaders, sizeof(Elf64_Phdr),
        elf->header.e_phnum, elf->fp
    );
    fseek(elf->fp, elf->header.e_shoff, SEEK_SET);
    fread(
        elf->sheaders, sizeof(Elf64_Shdr),
        elf->header.e_shnum, elf->fp
    );
    Elf64_Shdr* shstrtab_hdr = elf->sheaders + elf->header.e_shstrndx;
    elf->shstrtab = (char*)malloc(shstrtab_hdr->sh_size);
    fseek(elf->fp, shstrtab_hdr->sh_offset, SEEK_SET);
    fread(elf->shstrtab, 1, shstrtab_hdr->sh_size, elf->fp);
    success("File opened: %s", filename);
    exe->elf = elf;
    return exe;
}

void loader_close_elf(ExeMeta* exe) {
    ElfMeta* elf = exe->elf;
    fclose(elf->fp);
    free(elf->pheaders);
    free(elf->shstrtab);
    free(elf);
}
void loader_close_exe(ExeMeta* exe) {
    free(exe);
}

#define PAGE_SIZE 0x1000
#define ALIGN_D(addr) addr & ~(PAGE_SIZE - 1)
#define ALIGN_U(addr) (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)

void loader_map_segments(ExeMeta* exe) {
    ElfMeta* elf = exe->elf;
    uint64_t addr_min = UINT64_MAX;
    uint64_t addr_max = 0x0;
    for (int i = 0; i < elf->header.e_phnum; i++) {
        Elf64_Phdr* phdr = elf->pheaders + i;
        if (phdr->p_type == PT_LOAD) {
            uint64_t seg_start = phdr->p_vaddr;
            uint64_t seg_end = seg_start + phdr->p_memsz;
            if (seg_start < addr_min) addr_min = seg_start;
            if (seg_end > addr_max) addr_max = seg_end;
        }
    }
    exe->basesz = ALIGN_U(addr_max) - ALIGN_D(addr_min);
    /* TODO: non-obvious initialization, needs to be moved */
    memory_init(exe->basesz);
    exe->base = (uint8_t*)get_guest();
    print("base done %p", exe->base);
    for (int i = 0; i < elf->header.e_phnum; i++) {
        Elf64_Phdr* phdr = elf->pheaders + i;
        if (phdr->p_type == PT_LOAD) {
            fseek(elf->fp, phdr->p_offset, SEEK_SET);
            fread(exe->base + phdr->p_vaddr, 1, phdr->p_filesz, elf->fp);
            /* Usually we use protection here
               but during emulation it will interfere */
            //mprotect(exe->base + phdr->p_vaddr, phdr->p_filesz, 
            //    get_phdr_mmap_prot(phdr->p_flags) | PROT_WRITE);
            print("map %lx, %lx", phdr->p_vaddr, phdr->p_filesz);
            /* fill .bss */
            if(phdr->p_filesz != phdr->p_memsz) {
                memset(
                    exe->base + phdr->p_vaddr + phdr->p_filesz, 0,
                    phdr->p_memsz - phdr->p_filesz
                );
            }
        }
    }
}
void reloc_relr(ExeMeta* exe, Elf64_Relr* relr, int size) {
    print("process relr");
    size_t count = size / sizeof(Elf64_Addr);
    Elf64_Addr *where = NULL;
    
    for (size_t i = 0; i < count; i++) {
        Elf64_Addr entry = relr[i];
        
        if ((entry & 1) == 0) {
            where = (Elf64_Addr *)(exe->base + entry);
            *where++ += (Elf64_Addr)exe->base;
        } else {
            for (long i = 0; (entry >>= 1) != 0; i++) {
                if (entry & 1) {
                    where[i] += (Elf64_Addr)exe->base;
                }
            }
            where += 63;
        }
    }
}
void reloc_rela(ExeMeta* exe, Elf64_Rela* rela, int size) {
    ElfMeta* elf = exe->elf;
    print("process rela");
    for (int i = 0; i < size / sizeof(Elf64_Rela); i++) {
        Elf64_Rela* rel = rela + i;
        int t = ELF64_R_TYPE(rel->r_info);
        int sym_idx = ELF64_R_SYM(rel->r_info);
        Elf64_Addr *patch = (Elf64_Addr*)(exe->base + rel->r_offset);
        Elf64_Sym* sym = elf->symtab + sym_idx;
        switch(t) {
            case R_X86_64_NONE:
                break;
            case R_X86_64_64:
                *patch = (Elf64_Addr)(sym->st_value + rel->r_addend);
                //printf("Apply R_X86_64_64 %lx\n", *patch);
                break;
            case R_X86_64_RELATIVE:
                *patch = (Elf64_Addr)(exe->base + rel->r_addend);
                //printf("Apply R_X86_64_RELATIVE %lx\n", *patch);
                break;
            case R_X86_64_JUMP_SLOT:
            case R_X86_64_GLOB_DAT: {
                const char* sym_name = elf->strtab + sym->st_name;
                void *sym_addr = get_symbol(sym_name);
    
                if (sym_addr) {
                    *patch = (Elf64_Addr)sym_addr;
                    //printf("GLOB_DAT: %s -> %p\n", sym_name, sym_addr);
                } else if (ELF64_ST_BIND(sym->st_info) == STB_WEAK) {
                    *patch = 0;
                    //printf("GLOB_DAT: %s -> 0 (weak)\n", sym_name);
                } else {
                    //printf("ERROR: undefined symbol %s\n", sym_name);
                    *patch = 0;
                }
            } break;
            case R_X86_64_COPY: {
                const char* sym_name = elf->strtab + sym->st_name;
                //printf("R_X86_64_COPY: copying %lx bytes of %s\n",
                //    sym->st_size, sym_name);
                void *sym_addr = get_symbol(sym_name);
                //printf("R_X86_64_COPY: copying %p %p %lx\n",
                //    (uint64_t*)patch, sym_addr, *(uint64_t*)sym_addr);
                size_t size = sym->st_size;
                if (sym_addr && size) {
                    memmove(patch, sym_addr, size);
                }
            } break;
            default:
                panic("Unknown RELA %x", t);
        }
    }
}
void loader_reloc_dependencies(ExeMeta* exe) {
    ElfMeta* elf = exe->elf;
    Elf64_Phdr* dyn_phdr = NULL;
    for (int i = 0; i < elf->header.e_phnum; i++) {
        Elf64_Phdr* phdr = elf->pheaders + i;
        if (phdr->p_type == PT_DYNAMIC) {
            dyn_phdr = phdr;
            break;
        }
    }
    if (!dyn_phdr) {
        panic("LOADER::NO_DYNAMIC_SEGMENT");
        return;
    }
    Elf64_Dyn* dyn = (Elf64_Dyn*)(exe->base + dyn_phdr->p_vaddr);
    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
            case DT_SYMTAB:
                elf->symtab = (Elf64_Sym*)(exe->base + dyn->d_un.d_ptr);
                break;
            case DT_STRTAB:
                elf->strtab = (char*)(exe->base + dyn->d_un.d_ptr);
                break;
            case DT_RELA:
                elf->rela = (Elf64_Rela*)(exe->base + dyn->d_un.d_ptr);
                break;
            case DT_RELASZ:
                elf->relasz = dyn->d_un.d_val;
                break;
            case DT_RELR:
                elf->relr = (Elf64_Relr*)(exe->base + dyn->d_un.d_ptr);
                break;
            case DT_RELRSZ:
                elf->relrsz = dyn->d_un.d_val;
                break;
            case DT_JMPREL:
                elf->jmprel = (Elf64_Rela*)(exe->base + dyn->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                elf->pltrelsz = dyn->d_un.d_val;
                break;
            case DT_PLTGOT:
                break;
            case DT_PLTREL:
                if (dyn->d_un.d_val == DT_REL) {
                    panic("unsupported! DT_PLTREL");
                } else {
                    //print("DT_PLTRELA");
                }
                break;
            case DT_INIT_ARRAY:
                exe->init_array = dyn->d_un.d_ptr;
                break;
            case DT_INIT_ARRAYSZ:
                exe->init_arraysz = dyn->d_un.d_val;
                break;
            case DT_INIT:
                exe->init= dyn->d_un.d_ptr;
                break;
            case DT_FINI_ARRAY:
                exe->fini_array = dyn->d_un.d_ptr;
                break;
            case DT_FINI_ARRAYSZ:
                exe->fini_arraysz = dyn->d_un.d_val;
                break;
            case DT_FINI:
                exe->fini = dyn->d_un.d_ptr;
                break;

            case DT_REL:
                warning("unsupported! REL");
                break;
            case DT_PREINIT_ARRAY:
                warning("unsupported! PREINIT_ARRAY");
                break;
        }
    }
    dyn = (Elf64_Dyn*)(exe->base + dyn_phdr->p_vaddr);
    for (; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_NEEDED) {
            load_library(elf->strtab + dyn->d_un.d_val);
        }
    }
    if (elf->relr) reloc_relr(exe, elf->relr, elf->relrsz);
    if (elf->rela) reloc_rela(exe, elf->rela, elf->relasz);
    if (elf->jmprel) reloc_rela(exe, elf->jmprel, elf->pltrelsz);
}