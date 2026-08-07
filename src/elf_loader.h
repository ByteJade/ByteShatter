#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <elf.h>
#include <stdio.h>

typedef struct {
    Elf64_Ehdr head;
    Elf64_Shdr* sheads;
    Elf64_Phdr* pheads;
    Elf64_Sym* symtab;

    char* strtab;
    uint8_t* base;
    uint8_t* init;
    uint8_t** init_array;

    size_t init_arraysz;
} Elf;

Elf* elf_load(const char* filename);
void elf_close(Elf* elf);
void elf_read_dynamic(Elf* elf);
void elf_init(Elf* elf);

#endif