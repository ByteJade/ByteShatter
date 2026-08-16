#ifndef ELF_MANAGER_H
#define ELF_MANAGER_H

#include "elf_loader.h"

void open_library(const char* filename);
char* get_symbol(const char* symbol);
char* get_symbol_wrapped(const char* symbol);
void patch_library_got(const char* symbol, void* new_got);

#endif