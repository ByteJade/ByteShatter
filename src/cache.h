#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

typedef struct {
    uint8_t hoff;
    uint8_t goff;
} OffsetUnit;

typedef struct {
    uint16_t block;
    int16_t guest_off;
    uint8_t type;
    uint8_t meta;
} PatchUnit;

typedef struct {
    OffsetUnit* offsets;
    uint32_t hp;
    uint32_t gp;
    uint8_t end;
    uint8_t offsetssz;
} CacheUnit;

void cahce_init(void);
void cahce_fini(void);

/* clear all data */
void cache_clear(void);
/* set start point of code block cache */
uint16_t cache_block_start(void);
/* set instruction point in block  
   needed for jumping inside */
void cache_block_point(void);
/* set end point of code block cache */
void cache_block_end(void);
/* save patch in cache for patching */
uint16_t cache_patch_point(uint8_t type, uint8_t meta, int offset);

/* get pointer to host instruction at guest pointer */
uint8_t* cache_search(uint32_t gp);
/* get pointer to patch data */
PatchUnit* cache_get_patch(uint16_t patch_id);
/* get pointer to block data */
CacheUnit* cache_get_block(uint16_t block_id);

/* update code after patching */
void cache_flush(uint16_t block_id);
/* how much memory used */
uint32_t cache_usage(void);
/* show recompiled block */
void cache_print(int block);
/* get current block pointer */
int cache_bp(void);
/* was there a block overflow */
int cache_overflow(void);

#endif