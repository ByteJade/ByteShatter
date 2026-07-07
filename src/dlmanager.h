#ifndef DLMANAGER_H
#define DLMANAGER_H

#include "loader.h"
#include <stdint.h>

uint32_t my_hash(const char* str);

int is_external_offset(uint32_t offset);
ExeMeta* load_object(const char* filename);
void load_library(const char* filename);
void* get_native_symbol(const char* symname);
void* get_wrapped_symbol(const char* symname);

#endif