#ifndef DLMANAGER_H
#define DLMANAGER_H

#include "loader.h"

int is_external_offset(uint32_t offset);
ExeMeta* load_object(const char* filename);
void load_library(const char* filename);
void* get_symbol(const char* symname);

#endif