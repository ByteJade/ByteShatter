#include "cache.h"
#include "memory.h"
#include "core.h"
#include "decoder.h"
#include "patcher.h"
#include "printer_arm.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_BLOCKS 512
#define MAX_JUMPS 256
#define MAX_OFFSETS 24576

Anchor* anchor;

void cahce_init(uint64_t guest, uint32_t* host, uint32_t size) {
    anchor = (Anchor*) malloc(sizeof(Anchor));
    anchor->gp_hi = guest >> 32;
    anchor->offsets_p = 0;
    anchor->offsets_c = MAX_OFFSETS;
    anchor->offsets = (OffsetUnit*) malloc(anchor->offsets_c * sizeof(OffsetUnit));
    anchor->blocks_p = 0;
    anchor->blocks_c = MAX_BLOCKS;
    anchor->blocks = (CacheUnit*) malloc(anchor->blocks_c * sizeof(CacheUnit));
    anchor->host_p = 0;
    anchor->host_c = size;
    anchor->host = host;
    anchor->jumps_p = 0;
    anchor->jumps_c = MAX_JUMPS;
    anchor->jumps = (PatchUnit*) malloc(anchor->jumps_c * sizeof(PatchUnit));
    anchor->jumps_reuse_p = 0;
    anchor->jumps_reuse_c = MAX_JUMPS;
    anchor->jumps_reuse = (uint32_t*) malloc(anchor->jumps_c * sizeof(uint32_t));
}
void cahce_fini(void) {
    if (anchor->blocks) free(anchor->blocks);
    if (anchor->jumps) free(anchor->jumps);
    if (anchor->offsets) free(anchor->offsets);
    if (anchor) free(anchor);
}
void my_realloc(void** data, uint32_t* size, uint32_t element_size) {
    *size *= 2;
    *data = realloc(*data, (size_t)*size * element_size);
}

void cache_clear(void) {
    anchor->offsets_p = 0;
    anchor->blocks_p = 0;
    anchor->host_p = 0;
    anchor->jumps_p = 0;
    anchor->jumps_reuse_p = 0;
}
void cache_block_start(Context* context) {
    if (anchor->blocks_p+1 >= anchor->blocks_c) {
        panic("CACHE::BLOCKS::OVERFLOW");
    }
    // sort array
    uint32_t insert_pos = 0;
    while (insert_pos < anchor->blocks_p && 
           anchor->blocks[insert_pos].gp_lo < context->gp) {
        insert_pos++;
    }
    for (uint32_t rp = anchor->blocks_p; rp > insert_pos; rp--) {
        anchor->blocks[rp] = anchor->blocks[rp-1];
    }
    print("start block %x", insert_pos);
    CacheUnit* block = anchor->blocks + insert_pos;
    block->offsets = 0;
    block->offsetssz = 0;
    block->end = 1;
    block->gp_lo = context->gp;
    block->hp = context->hp;
    anchor->blocks_p++;
    
    context->block = block;
}
void cache_block_point(struct Context* context) {
    uint16_t goff = context->gp - context->block->gp_lo;
    if (goff == 0) return;
    uint16_t hogg = context->hp - context->block->hp;
    if (goff > UINT8_MAX || hogg > UINT8_MAX) {
        warning("CACHE::BLOCKS::BAD_OFFSET");
        cache_block_end(context);
        cache_block_start(context);
        goff = context->gp - context->block->gp_lo;
        hogg = context->hp - context->block->hp;
    }
    OffsetUnit* offset = &anchor->offsets[anchor->offsets_p+context->loffp];
    offset->goff = goff;
    offset->hoff = hogg;
    context->loffp++;
    if (anchor->offsets_p + context->loffp >= anchor->offsets_c)
        panic("CACHE::OFFSET_OVERFLOW");
    if (context->loffp == UINT8_MAX) {
        cache_block_end(context);
        cache_block_start(context);
    }
}
void cache_block_end(struct Context* context) {
    context->block->end = context->hp - context->block->hp;
    context->block->offsets = anchor->offsets_p;
    context->block->offsetssz = context->loffp;
    anchor->offsets_p += context->loffp;
    context->loffp = 0;

    uint32_t* start = anchor->host + context->block->hp;
    uint32_t* end = anchor->host + context->hp;
    print("flush cache %x-%x;", context->block->hp, end - anchor->host);
    __builtin___clear_cache(start, end);
    anchor->host_p = context->hp;
    if (anchor->host_p >= anchor->host_c) {
        panic("host overflow");
    }
}
uint32_t cache_patch_point(Context* context, uint8_t type, int offset) {
    uint32_t ret = 0;
    if (anchor->jumps_reuse_p) {
        ret = anchor->jumps_reuse[--anchor->jumps_reuse_p];
    } else {
        ret = anchor->jumps_p;
        anchor->jumps_p++;
        if (anchor->jumps_p >= anchor->jumps_c)
            my_realloc((void*)&anchor->jumps, &anchor->jumps_c, sizeof(PatchUnit));
    }
    PatchUnit* jump = anchor->jumps + ret;
    jump->type = type;
    // where to jump (relative to the start of the anchor)
    jump->guest_off = context->gp + offset;
    
    return ret+1;
}
uint32_t block_cache_search(uint32_t gp, CacheUnit* cache) {
    gp -= cache->gp_lo;
    OffsetUnit* offsets = anchor->offsets + cache->offsets;
    // binary search
    int left = 0, right = cache->offsetssz - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        uint8_t goff = offsets[mid].goff;
        if (goff == gp) return cache->hp + offsets[mid].hoff;
        if (goff < gp) left = mid + 1; 
        else right = mid - 1;
    }

    // warning("CACHE::MISTMATCH");
    /* but some programs may jump
       to the center of instruction
       which will cause this exception */
    return 0;
}
uint32_t* cache_search(uint64_t gp) {
    // TODO: better cache search
    uint32_t gp_lo = gp;
    Anchor* anchor = cache_get_anchor(gp);
    if (anchor->blocks_p == 0) return NULL;
    CacheUnit* cache = NULL;
    int left = 0, right = anchor->blocks_p - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        cache = anchor->blocks + mid;
        if (gp_lo == cache->gp_lo)
            return anchor->host + cache->hp;
        if (gp_lo > cache->gp_lo) left = mid + 1; 
        else right = mid - 1;
    }
    if (right < 0) return NULL;
    cache = anchor->blocks + right;
    if (cache->offsets == 0) return NULL;
    uint32_t loff = block_cache_search(gp_lo, cache);
    if (!loff) return NULL;
    return anchor->host + loff;
}
uint32_t cache_search_block(uint64_t hp) {
    hp -= (uint64_t)anchor->host;
    hp /= 4;
    for (int i = 0; i < anchor->blocks_p; i++) {
        uint32_t start = anchor->blocks[i].hp;
        uint32_t end = start + anchor->blocks[i].end;
        if (hp >= start && hp < end) return i;
    }
    return UINT32_MAX;
}
PatchUnit cache_get_patch(uint32_t patch_id) {
    patch_id--;
    anchor->jumps_reuse[anchor->jumps_reuse_p++] = patch_id;
    if (anchor->jumps_reuse_p == anchor->jumps_reuse_c)
        panic("CACHE::REUSE_OVERFLOW");
    return anchor->jumps[patch_id];
}
CacheUnit* cache_get_block(uint32_t block_id) {
    if (block_id >= anchor->blocks_p) {
        return NULL;
    }
    return anchor->blocks + block_id;
}
Anchor* cache_get_anchor(uint64_t guest) {
    ((void)guest);
    return anchor;
}

uint32_t cache_usage(void) {
    return sizeof(Anchor) +
        anchor->blocks_p * sizeof(CacheUnit) +
        anchor->jumps_p * sizeof(PatchUnit) + 
        anchor->offsets_p * sizeof(OffsetUnit) + 
        anchor->jumps_reuse_p * sizeof(uint32_t);
}
void cache_print(int block) {
    CacheUnit* unit = anchor->blocks + block;
    printf("%X Block: %i\n", unit->hp, block);
    uint32_t* host = anchor->host + unit->hp;
    Context context;
    setup_context(&context,((uint64_t)anchor->gp_hi<<32) + unit->gp_lo);
    int start = 0;
    for (int x = 0; x <= unit->offsetssz; x++) {
        OffsetUnit* offsets = (anchor->offsets + unit->offsets);
        Instruction buf;
        decode_instr(&context, &buf);
        char out[64];
        int end;
        if (x == unit->offsetssz) end = unit->end;
        else end = offsets[x].hoff;
        for (int y = start; y < end; y++) {
            sprint_arm(out, host[y]);
            printf("%x %s", host[y], out);
            if ((uint64_t)&host[y] == get_pc()) {
                printf(" <-\n");
            } else printf("\n");
        }
        start = end;
    }
}
int cache_bp(void) {
    return anchor->blocks_p;
}
OffsetUnit* cache_offsets(void) {
    return anchor->offsets;
}