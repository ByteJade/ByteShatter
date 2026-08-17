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
#define MAX_JUMPS 255
#define MAX_OFFSETS 24576

CacheUnit* last_block;
Anchor* anchor;

int overflow = 0;

void cahce_init(void) {
    anchor = malloc(sizeof(Anchor));
    anchor->blocks_p = 0;
    anchor->blocks_c = MAX_BLOCKS * sizeof(CacheUnit);
    anchor->blocks = (CacheUnit*) malloc(anchor->blocks_c);
    anchor->jumps_p = 0;
    anchor->jumps_c = MAX_JUMPS * sizeof(PatchUnit);
    anchor->jumps = (PatchUnit*) malloc(anchor->jumps_c);
    anchor->offsets_p = 0;
    anchor->offsets_c = MAX_OFFSETS * sizeof(OffsetUnit);
    anchor->offsets = (OffsetUnit*) malloc(anchor->offsets_c);
}
void cahce_fini(void) {
    if (anchor->blocks) free(anchor->blocks);
    if (anchor->jumps) free(anchor->jumps);
    if (anchor->offsets) free(anchor->offsets);
    if (anchor) free(anchor);
}

void cache_clear(void) {
    anchor->offsets_p = 0;
    anchor->blocks_p = 0;
    anchor->jumps_p = 0;
}
uint16_t cache_block_start(Context* context) {
    last_block = anchor->blocks + anchor->blocks_p;
    last_block->offsets = 0;
    last_block->gp_lo = get_gp();
    last_block->hp = get_hp();
    anchor->blocks_p++;
    if (anchor->blocks_p >= MAX_BLOCKS) {
        panic("CACHE::BLOCKS::OVERFLOW");
    }
    return anchor->blocks_p-1;
}
void cache_block_point(struct Context* context) {
    uint16_t goff = get_gp() - last_block->gp_lo;
    if (goff == 0) return;
    uint16_t hogg = get_hp() - last_block->hp;
    if (goff > UINT8_MAX || hogg > UINT8_MAX) {
        warning("CACHE::BLOCKS::BAD_OFFSET");
        overflow = 1;
        cache_block_end(context);
        cache_block_start(context);
        goff = get_gp() - last_block->gp_lo;
        hogg = get_hp() - last_block->hp;
    }
    OffsetUnit* offset = &anchor->offsets[anchor->offsets_p+context->loffp];
    offset->goff = goff;
    offset->hoff = hogg;
    context->loffp++;
    if (context->loffp == UINT8_MAX) {
        warning("CACHE::OFFSET::OVERFLOW");
        cache_block_end(context);
        cache_block_start(context);
    }
}
void cache_block_end(struct Context* context) {
    last_block->end = get_gp() - last_block->gp_lo;
    last_block->offsets = anchor->offsets_p;
    last_block->offsetssz = context->loffp;
    anchor->offsets_p += context->loffp;
    context->loffp = 0;

    uint32_t* start = get_host() + last_block->hp;
    uint32_t* end = get_host() + get_hp();
    print("flush cache %x-%x;", last_block->hp, (end - start)*4);
    __builtin___clear_cache(start, end);
}
uint16_t cache_patch_point(uint8_t type, int offset) {
    if (offset < INT32_MIN || offset > INT32_MAX) {
        /* I don't know yet how to
           work with such jumps */
        panic("CACHE::JUMPS::BAD_OFFSET");
    }
    PatchUnit* jump = anchor->jumps + anchor->jumps_p;
    jump->type = type;
    // where to jump (relative to the start of the block)
    jump->guest_off = get_gp() + offset;
    return ++anchor->jumps_p;
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

    panic("CACHE::MISTMATCH");
    /* but some programs may jump
       to the center of instruction
       which will cause this exception */
    return 0;
}
uint32_t* cache_search(uint32_t gp) {
    // TODO: better cache search
    for (int i = 0; i < anchor->blocks_p; i++) {
        CacheUnit* cache = anchor->blocks + i;
        if (cache->offsets == 0) continue;
        if (gp == cache->gp_lo) return get_host() + cache->hp;
        if (gp > cache->gp_lo && gp <  cache->gp_lo + cache->end) {
            return get_host() + block_cache_search(gp, cache);
        }
    }
    return NULL;
}
uint32_t cache_search_block(uint32_t hp) {
    for (int i = anchor->blocks_p-1; i >= 0; i--) {
        if (hp >= anchor->blocks[i].hp) return i;
    }
    return UINT32_MAX;
}
PatchUnit* cache_get_patch(uint16_t patch_id) {
    patch_id--;
    if (patch_id >= anchor->jumps_p) {
        panic("CACHE::PATCH::BAD_ID %x", patch_id);
    }
    return anchor->jumps + patch_id;
}
CacheUnit* cache_get_block(uint16_t block_id) {
    if (block_id >= anchor->blocks_p) {
        return NULL;
    }
    return anchor->blocks + block_id;
}
Anchor* cache_get_anchor(uint64_t guest) {
    ((void)guest);
    return anchor;
}

void cache_back() {
    //(local_offsets)[loffp-1].hoff--;
}
uint32_t cache_usage(void) {
    return anchor->blocks_p * sizeof(CacheUnit) +
        anchor->jumps_p * sizeof(PatchUnit) + 
        anchor->offsets_p * sizeof(OffsetUnit);
}
void cache_print(int block) {
    CacheUnit* unit = anchor->blocks + block;
    printf("%X Block: %i\n", unit->hp, block);
    uint32_t* host = get_host() + unit->hp;
    for (int x = 0; x < unit->offsetssz; x++) {
        OffsetUnit* offsets = (anchor->offsets + unit->offsets);
        Instruction buf;
        set_gp(unit->gp_lo + offsets[x].goff);
        decode_instr(&buf);
        char out[64];
        int end;
        int start = offsets[x].hoff;
        if (x+1 == unit->offsetssz) end = start+4;
        else end = offsets[x+1].hoff;
        for (int y = start; y < end; y++) {
            sprint_arm(out, host[y]);
            printf("%x %s", host[y], out);
            if ((uint64_t)&host[y] == get_pc()) {
                printf(" <-\n");
            } else printf("\n");
        }
    }
}
int cache_bp(void) {
    return anchor->blocks_p;
}
OffsetUnit* cache_offsets(void) {
    return anchor->offsets;
}
int cache_overflow(void) {
    int out = overflow;
    overflow = 0;
    return out;
}