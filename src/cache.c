#include "cache.h"
#include "armdef.h"
#include "memory.h"
#include "core.h"
#include "decoder.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
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

int overflow = 0;

void cahce_init(void) {
    blocks_cache = (CacheUnit*) malloc(
        MAX_BLOCKS * sizeof(CacheUnit)
    );
    jumps_cache = (PatchUnit*) malloc(
        MAX_JUMPS * sizeof(PatchUnit)
    );
}
void cahce_fini(void) {
    for (int i = 0; i < bp; i++) {
        CacheUnit* unit = blocks_cache + i;
        if (unit->offsets) free(unit->offsets);
    }
    if (blocks_cache) free(blocks_cache);
    if (jumps_cache) free(jumps_cache);
}

void cache_clear(void) {
    for (int i = 0; i < bp; i++) {
        CacheUnit* unit = blocks_cache + i;
        if (unit->offsets) free(unit->offsets);
    }
    bp = 0;
    pp = 0;
}
uint16_t cache_block_start(void) {
    last_block = blocks_cache + bp;
    last_block->offsets = NULL;
    last_block->gp = get_gp();
    last_block->hp = get_hp();
    bp++;
    if (bp >= MAX_BLOCKS) {
        panic("CACHE::BLOCKS::OVERFLOW");
    }
    return bp-1;
}
void cache_block_point(void) {
    uint16_t goff = get_gp() - last_block->gp;
    uint16_t hogg = get_hp() - last_block->hp;
    if (goff > UINT8_MAX || hogg > UINT8_MAX) {
        warning("CACHE::BLOCKS::BAD_OFFSET");
        overflow = 1;
        cache_block_end();
        cache_block_start();
        goff = get_gp() - last_block->gp;
        hogg = get_hp() - last_block->hp;
    }
    local_offsets[loffp].goff = goff;
    local_offsets[loffp].hoff = hogg/4;
    offset_usage += sizeof(OffsetUnit);
    loffp++;
    if (loffp >= MAX_OFFSETS) {
        warning("CACHE::OFFSET::OVERFLOW");
        cache_block_end();
        cache_block_start();
    }
}
void cache_block_end(void) {
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
        if (goff == gp) return cache->hp + offsets[mid].hoff*4;
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
        if (cache->offsets == NULL) continue;
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
    uint8_t* code = get_host() + unit->hp;
    uint32_t size = unit->offsets[unit->offsetssz-1].hoff*4+32;
    print("flush cache %x-%x; block %i", unit->hp, unit->hp+size, block_id);
    __builtin___clear_cache(code, code + size);
}
uint32_t cache_usage(void) {
    return bp * sizeof(CacheUnit) + pp * sizeof(PatchUnit) + offset_usage;
}
void cache_print(int block) {
    CacheUnit* unit = blocks_cache + block;
    printf("%X Block: %i\n", unit->hp, block);
    uint32_t* host = (uint32_t*)(&get_host()[unit->hp]);
    for (int x = 0; x < unit->offsetssz; x++) {
        X64_instruction buf;
        set_gp(unit->gp + unit->offsets[x].goff);
        decode_instr(&buf);
        char out[32];
        sprint_instr(out, &buf);
        printf(" : %s\n", out);
        int end;
        int start = unit->offsets[x].hoff;
        if (x+1 == unit->offsetssz) end = start+4;
        else end = unit->offsets[x+1].hoff;
        for (int y = start; y < end; y++) {
            printf("%x\n",host[y]);
        }
    }
}
int cache_bp(void) {
    return bp;
}
int cache_overflow(void) {
    int out = overflow;
    overflow = 0;
    return out;
}