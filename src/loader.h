#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <elf.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    FILE* fp;
    Elf64_Ehdr header;
    Elf64_Phdr* pheaders;
    Elf64_Shdr* sheaders;
    Elf64_Sym* dynsym;
    int dynsymsz;
    Elf64_Sym* symtab;
    char* strtab;
    char* shstrtab;
    Elf64_Rela* rela;
    int relasz;
    Elf64_Rela* jmprel;
    int pltrelsz;
    Elf64_Relr* relr;
    int relrsz;
    uint32_t* sym_cache;
} ElfMeta;

typedef struct {
    ElfMeta* elf;

    uint8_t* base;
    uint64_t basesz;

    uint64_t init_array;
    uint64_t init_arraysz;
    uint64_t init;
    
    uint64_t fini_array;
    uint64_t fini_arraysz;
    uint64_t fini;
    uint8_t native;
} ExeMeta;

ExeMeta* loader_open_elf(const char* filename);
void loader_close_elf(ExeMeta* exe);
void loader_close_exe(ExeMeta* exe);

void loader_map_segments(ExeMeta* exe);
void loader_reloc_dependencies(ExeMeta* exe);

#endif