#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void memory_init(uint32_t guest_size);
void memory_fini(void);

void* mmap_guest(uint32_t guest_size);
void memory_clear_host(void);

/* emit 4 bytes to host memory */
void emit32(uint32_t data);
/* replace 32 bytes */
void patch32();

/* fetch byte from guest memory */
uint8_t fetch8(void);
/* fetch 2 bytes from guest memory */
uint16_t fetch16(void);
/* fetch 4 bytes from guest memory */
uint32_t fetch32(void);
/* fetch 8 bytes from guest memory */
uint32_t fetch64(void);

/* set guest */
void set_guest(uint64_t new_guest);
void set_gp(uint32_t new_gp);
/* set host pointer  
   only for code patching! */
void set_hp(uint32_t new_hp);

/* get host pointer */
uint64_t get_hp(void);
/* get guest pointer */
uint64_t get_gp(void);

/* get pointer to host memory */
uint32_t* get_host(void);
/* get pointer to guest memory */
uint8_t* get_guest(void);

#endif