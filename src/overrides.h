#ifndef OVERRIDES_H
#define OVERRIDES_H

extern int syscall_override[];

void* sym_override(const char* symname);

#endif