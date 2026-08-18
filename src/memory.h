#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "cache.h"

typedef struct Context {
   CacheUnit* block;
   OffsetUnit* offsets;
   uint8_t* guest;
   uint32_t* host;
   uint32_t gp;
   uint32_t hp;
   uint8_t loffp;
} Context;

void setup_context(Context* context, uint64_t gp);

void memory_init(uint32_t guest_size);
void memory_fini(void);

void* mmap_guest(uint32_t guest_size);

/* emit 4 bytes to host memory */
inline void emit32(Context* cotnext, uint32_t data) {
    cotnext->host[cotnext->hp++] = data;
}
/* replace 32 bytes */
inline void patch(Context* cotnext, uint32_t n) {
    cotnext->hp -= n;
}

/* fetch byte from guest memory */
inline uint8_t fetch8(Context* cotnext) {
    uint8_t* src = cotnext->guest + cotnext->gp;
    cotnext->gp += 1;
    return *src;
}
/* fetch 2 bytes from guest memory */
inline uint16_t fetch16(Context* cotnext) {
    uint16_t* src = (uint16_t*)(cotnext->guest + cotnext->gp);
    cotnext->gp += 2;
    return *src;
}
/* fetch 4 bytes from guest memory */
inline uint32_t fetch32(Context* cotnext) {
    uint32_t* src = (uint32_t*)(cotnext->guest + cotnext->gp);
    cotnext->gp += 4;
    return *src;
}
/* fetch 8 bytes from guest memory */
inline uint32_t fetch64(Context* cotnext) {
    uint64_t* src = (uint64_t*)(cotnext->guest + cotnext->gp);
    cotnext->gp += 8;
    return *src;
}

/* get pointer to host memory */
uint32_t* get_host(void);

#endif