#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdatomic.h>
#include "cache.h"
#include "decoder.h"

typedef struct {
    uint8_t hoff;
    uint8_t goff;
} OffsetUnit;

typedef struct {
    uint32_t gp_lo;
    uint32_t hp;
    uint32_t offsets;
    uint8_t offsetssz;
    uint8_t end;
} CacheUnit;

typedef struct Context {
    uint8_t* guest;
    uint32_t gp;
    uint32_t* host;
    uint32_t hp;
    CacheUnit* block;
    Block* c_block;
    atomic_bool in_use;
/* to switch to static compilation*/
    Instruction* buffer;
    uint32_t buffer_p;
    uint32_t buffer_c;
    uint32_t* jumps;
    uint32_t jumps_p;
    uint32_t jumps_c;
    OffsetUnit* offsets;
    uint32_t offsets_p;
    uint32_t offsets_c;
    CacheUnit* blocks;
    uint32_t blocks_p;
    uint32_t blocks_c;
} Context;

void context_init();

Context* context_pull(uint64_t gp);
void context_free(Context* context);
int context_usage();

void context_push_jump(Context* context, uint32_t jump);
uint32_t* context_pull_jump(Context* context);
Instruction* context_pull_buffer(Context* context);

void context_block_start(Context* context);
void context_block_point(Context* context);
void context_block_end(Context* context);

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
inline uint64_t fetch64(Context* cotnext) {
    uint64_t* src = (uint64_t*)(cotnext->guest + cotnext->gp);
    cotnext->gp += 8;
    return *src;
}

#endif