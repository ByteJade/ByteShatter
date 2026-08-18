#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

typedef struct {
    uint8_t hoff;
    uint8_t goff;
} OffsetUnit;

typedef struct {
    uint32_t guest_off;
    uint8_t type;
} PatchUnit;

typedef struct {
    uint32_t gp_lo;
    uint32_t hp;
    uint32_t offsets;
    uint8_t end;
    uint8_t offsetssz;
} CacheUnit;

typedef struct {
    uint32_t gp_hi;
    OffsetUnit* offsets;
    uint32_t offsets_p;
    uint32_t offsets_c;
    CacheUnit* blocks;
    uint32_t blocks_p;
    uint32_t blocks_c;
    uint32_t* host;
    uint32_t host_p;
    uint32_t host_c;
    PatchUnit* jumps;
    uint32_t jumps_p;
    uint32_t jumps_c;
} Anchor;

void cahce_init(uint64_t gp);
void cahce_fini(void);

typedef struct Context Context;
/* clear all data */
void cache_clear(void);
/* set start point of code block cache */
uint16_t cache_block_start(Context* context);
/* set instruction point in block  
   needed for jumping inside */
void cache_block_point(Context* context);
/* set end point of code block cache */
void cache_block_end(Context* context);
/* save patch in cache for patching */
uint16_t cache_patch_point(Context* context, uint8_t type, int offset);

/* get pointer to host instruction at guest pointer */
uint32_t* cache_search(uint32_t gp);
/* get pointer to host instruction at guest pointer */
uint32_t cache_search_block(uint32_t hp);
/* get pointer to patch data */
PatchUnit* cache_get_patch(uint16_t patch_id);
/* get pointer to block data */
CacheUnit* cache_get_block(uint16_t block_id);

Anchor* cache_get_anchor(uint64_t guest);

/* fix instruction pointer if patch */
void cache_back();
/* how much memory used */
uint32_t cache_usage(void);
/* show recompiled block */
void cache_print(int block);
/* get current block pointer */
int cache_bp(void);
/* get current block pointer */
OffsetUnit* cache_offsets(void);

#endif