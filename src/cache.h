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
} JumpUnit;

typedef struct {
    OffsetUnit* offsets;
    uint32_t hp;
    uint32_t gp;
    uint8_t end;
    uint8_t offsetssz;
} CacheUnit;

void cahce_init();
void cahce_fini();

/* clear all data */
void cache_clear();
/* set start point of code block cache */
void cache_block_start();
/* set instruction point in block  
   needed for jumping inside */
void cache_block_point();
/* set end point of code block cache */
void cache_block_end();
/* save jump in cache for patching */
uint16_t cache_jump_point(uint8_t type, int offset);

/* get pointer to host instruction at guest pointer */
uint8_t* cache_search(uint32_t gp);
/* get pointer to jump data */
JumpUnit* cache_get_jump(uint16_t jump_id);
/* get pointer to block data */
CacheUnit* cache_get_block(uint16_t block_id);

/* update code after patching */
void cache_flush(uint16_t block_id);
/* how much memory used */
uint32_t cache_usage();
/* show recompiled blocks */
void cache_print();

#endif