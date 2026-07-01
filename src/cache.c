#include "cache.h"
#include "memory.h"
#include "core.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BLOCKS 255
#define MAX_JUMPS 255
#define MAX_OFFSETS 255

CacheUnit* blocks_cache = NULL;
PatchUnit* jumps_cache = NULL;
uint16_t bp = 0;
uint16_t pp = 0;

CacheUnit* last_block = NULL;
OffsetUnit local_offsets[MAX_OFFSETS];
uint8_t loffp = 0;
uint32_t offset_usage = 0;

void cahce_init() {
    blocks_cache = (CacheUnit*) malloc(
        MAX_BLOCKS * sizeof(CacheUnit)
    );
    jumps_cache = (PatchUnit*) malloc(
        MAX_JUMPS * sizeof(PatchUnit)
    );
    success("cache init");
}
void cahce_fini() {
    for (int i = 0; i < bp; i++) {
        CacheUnit* unit = blocks_cache + i;
        if (unit->offsets) free(unit->offsets);
    }
    if (blocks_cache) free(blocks_cache);
    if (jumps_cache) free(jumps_cache);
}

void cache_clear() {
    for (int i = 0; i < bp; i++) {
        CacheUnit* unit = blocks_cache + i;
        if (unit->offsets) free(unit->offsets);
    }
    bp = 0;
    pp = 0;
}
uint16_t cache_block_start() {
    last_block = blocks_cache + bp;
    last_block->gp = get_gp();
    last_block->hp = get_hp();
    bp++;
    if (bp >= MAX_BLOCKS) {
        panic("CACHE::BLOCKS::OVERFLOW");
    }
    return bp-1;
}
void cache_block_point() {
    uint16_t goff = get_gp() - last_block->gp;
    uint16_t hogg = get_hp() - last_block->hp;
    local_offsets[loffp].goff = goff;
    local_offsets[loffp].hoff = hogg;
    offset_usage += sizeof(OffsetUnit);
    loffp++;
    if (loffp >= MAX_OFFSETS) {
        warning("CACHE::OFFSET::OVERFLOW");
        cache_block_end();
        cache_block_start();
    }
    if (goff > UINT8_MAX || hogg > UINT8_MAX) {
        /* I don't yet know how to
           invalidate an instruction that
           has gone beyond the limit */
        panic("CACHE::BLOCKS::BAD_OFFSET");
    }
}
void cache_block_end() {
    last_block->end = get_gp() - last_block->gp;
    uint32_t size = loffp * sizeof(OffsetUnit);
    last_block->offsets = (OffsetUnit*)malloc(size);
    memcpy(last_block->offsets, local_offsets, size);
    last_block->offsetssz = loffp;
    loffp = 0;
}
uint16_t cache_patch_point(uint8_t type, uint8_t meta, int offset) {
    if (offset < INT16_MIN || offset > INT16_MAX) {
        /* I don't know yet how to
           work with such jumps */
        panic("CACHE::JUMPS::BAD_OFFSET");
    }
    PatchUnit* jump = jumps_cache + pp;
    uint16_t block = bp - 1;
    jump->type = type;
    jump->meta = meta;
    jump->block = block;
    // where to jump (relative to the start of the block)
    jump->guest_off = get_gp() - blocks_cache[block].gp + offset;
    return ++pp;
}
uint32_t block_cache_search(uint32_t gp, CacheUnit* cache) {
    gp -= cache->gp;
    OffsetUnit* offsets = cache->offsets;
    // binary search
    int left = 0, right = cache->offsetssz - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        uint8_t goff = offsets[mid].goff;
        if (goff == gp) return cache->hp + offsets[mid].hoff;
        if (goff < gp) left = mid + 1; 
        else right = mid - 1;
    }

    panic("CACHE::MISTMATCH");
    /* but some programs may jump
       to the center of instruction
       which will cause this exception */
    return 0;
}
uint8_t* cache_search(uint32_t gp) {
    // TODO: better cache search
    for (int i = 0; i < bp; i++) {
        CacheUnit* cache = blocks_cache + i;
        if (gp == cache->gp) return get_host() + cache->hp;
        if (gp > cache->gp && gp <  cache->gp + cache->end) {
            return get_host() + block_cache_search(gp, cache);
        }
    }
    return NULL;
}
PatchUnit* cache_get_patch(uint16_t patch_id) {
    patch_id--;
    if (patch_id >= pp) {
        panic("CACHE::PATCH::BAD_ID %x", patch_id);
    }
    return jumps_cache + patch_id;
}
CacheUnit* cache_get_block(uint16_t block_id) {
    if (block_id >= bp) {
        panic("CACHE::BLOCKS::BAD_ID");
    }
    return blocks_cache + block_id;
}

void cache_flush(uint16_t block_id) {
    CacheUnit* unit = blocks_cache + block_id;
    void* code = get_host() + unit->hp;
    uint32_t size = unit->offsets[unit->offsetssz-1].hoff+16;
    print("flush cache %x-%x; block %x", unit->hp, unit->hp+size, block_id);
    __builtin___clear_cache(code, code + size);
}
uint32_t cache_usage() {
    return bp * sizeof(CacheUnit) + pp * sizeof(PatchUnit) + offset_usage;
}
void cache_print() {
    print("Cache:");
    for (int i = 0; i < bp; i++) {
        CacheUnit* unit = blocks_cache + i;
        uint32_t size = unit->offsets[unit->offsetssz-1].hoff/4+4;
        print("\n%lX Block: %i %i", unit->hp, i, size);
        uint32_t* host = (uint32_t*)(&get_host()[unit->hp]);
        for (int x = 0; x < size; x++) {
            print("%x",host[x]);
        }
    }
}