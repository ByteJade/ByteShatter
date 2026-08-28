#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

typedef struct {
    uint32_t guest_off;
    uint8_t type;
} PatchUnit;

typedef struct {
    uint32_t gp_lo;
    uint32_t hp_lo;
} Block;

typedef struct {
    uint32_t gp_hi;
    uint32_t* host;
    uint32_t host_p;
    uint32_t host_c;
    Block* blocks;
    uint32_t blocks_p;
    uint32_t blocks_c;
    PatchUnit* jumps;
    uint32_t jumps_p;
    uint32_t jumps_c;
    uint32_t* jumps_reuse;
    uint32_t jumps_reuse_p;
    uint32_t jumps_reuse_c;
} Anchor;

void cahce_init(uint64_t guest, uint32_t* host, uint32_t size);
void cahce_fini(void);

typedef struct Context Context;
/* clear all data */
void cache_clear(void);
uint32_t cache_patch_point(Context* context, uint8_t type, int offset);
void cache_start_block(Context* context);
void cache_end_block(Context* context);

/* get pointer to host instruction at guest pointer */
uint32_t* cache_search(uint64_t gp);
/* get pointer to host instruction at guest pointer */
uint32_t cache_search_block(uint64_t hp);
/* get pointer to patch data */
PatchUnit cache_get_patch(uint32_t patch_id);

Anchor* cache_get_anchor(uint64_t guest);

/* how much memory used */
uint32_t cache_usage(void);
/* show recompiled block */
void cache_print(int block);

#endif