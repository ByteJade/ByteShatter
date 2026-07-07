#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void memory_init(uint32_t guest_size);
void memory_fini();

void* mmap_guest(uint32_t guest_size);

/* emit byte to host memory */
void emit8(uint8_t data);
/* emit 2 bytes to host memory */
void emit16(uint16_t data);
/* emit 4 bytes to host memory */
void emit32(uint32_t data);

/* fetch byte from guest memory */
uint8_t fetch8();
/* fetch 2 bytes from guest memory */
uint16_t fetch16();
/* fetch 4 bytes from guest memory */
uint32_t fetch32();

/* set guest */
void set_guest(uint64_t new_guest);
void set_gp(uint32_t new_gp);
/* set host pointer  
   only for code patching! */
void set_hp(uint32_t new_hp);

/* get host pointer */
uint64_t get_hp();
/* get guest pointer */
uint64_t get_gp();

/* get pointer to host memory */
uint8_t* get_host();
/* get pointer to guest memory */
uint8_t* get_guest();

#endif