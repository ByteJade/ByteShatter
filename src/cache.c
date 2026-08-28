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

#define MAX_JUMPS 256
#define MAX_BLOCKS 256

Anchor* anchor;

void cahce_init(uint64_t guest, uint32_t* host, uint32_t size) {
    anchor = (Anchor*) malloc(sizeof(Anchor));
    anchor->gp_hi = guest >> 32;
    anchor->host_p = 0;
    anchor->host_c = size;
    anchor->host = host;
    anchor->blocks_p = 0;
    anchor->blocks_c = MAX_BLOCKS;
    anchor->blocks = (Block*) malloc(anchor->blocks_c * sizeof(Block));
    anchor->jumps_p = 0;
    anchor->jumps_c = MAX_JUMPS;
    anchor->jumps = (PatchUnit*) malloc(anchor->jumps_c * sizeof(PatchUnit));
    anchor->jumps_reuse_p = 0;
    anchor->jumps_reuse_c = MAX_JUMPS;
    anchor->jumps_reuse = (uint32_t*) malloc(anchor->jumps_reuse_c* sizeof(uint32_t));
}
void cahce_fini(void) {
    if (anchor->jumps) free(anchor->jumps);
    if (anchor->blocks) free(anchor->blocks);
    if (anchor) free(anchor);
}
void my_realloc(void** data, uint32_t* size, uint32_t element_size) {
    *size *= 2;
    *data = realloc(*data, (size_t)*size * element_size);
}

void cache_clear(void) {
    anchor->host_p = 0;
    anchor->jumps_p = 0;
    anchor->blocks_p = 0;
    anchor->jumps_reuse_p = 0;
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
void cache_create_block(Context* context) {
    if (anchor->blocks_p+1 >= anchor->blocks_c) {
        panic("CONTEXT::BLOCKS::OVERFLOW");
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
    print("start cache block %x", insert_pos);
    Block* block = anchor->blocks + insert_pos;
    block->gp_lo = context->gp;
    block->hp_lo = context->hp;
    anchor->blocks_p++;
    context->host = anchor->host;
    context->hp = anchor->host_p;
}
uint32_t* cache_search(uint64_t gp) {
    // TODO: better cache search
    uint32_t gp_lo = gp;
    Anchor* anchor = cache_get_anchor(gp);
    if (anchor->blocks_p == 0) return NULL;
    Block* cache = NULL;
    int left = 0, right = anchor->blocks_p - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        cache = anchor->blocks + mid;
        if (gp_lo == cache->gp_lo)
            return anchor->host + cache->hp_lo;
        if (gp_lo > cache->gp_lo) left = mid + 1; 
        else right = mid - 1;
    }
    return NULL;
}
uint32_t cache_search_block(uint64_t hp) {
    hp -= (uint64_t)anchor->host;
    hp /= 4;
    /*
    for (int i = 0; i < anchor->blocks_p; i++) {
        uint32_t start = anchor->blocks[i].hp;
        uint32_t end = start + anchor->blocks[i].end;
        if (hp >= start && hp < end) return i;
    }
    */
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
    /*if (block_id >= anchor->blocks_p) {
        return NULL;
    }
    return anchor->blocks + block_id;*/
    return NULL;
}
Anchor* cache_get_anchor(uint64_t guest) {
    ((void)guest);
    return anchor;
}

uint32_t cache_usage(void) {
    /*
    return sizeof(Anchor) +
        anchor->blocks_p * sizeof(CacheUnit) +
        anchor->jumps_p * sizeof(PatchUnit) + 
        anchor->offsets_p * sizeof(OffsetUnit) + 
        anchor->jumps_reuse_p * sizeof(uint32_t);
    */
    return 0;
}
void cache_print(int block) {
    /*
    CacheUnit* unit = anchor->blocks + block;
    printf("%X Block: %i\n", unit->hp, block);
    uint32_t* host = anchor->host + unit->hp;
    Context* context = context_pull(
        ((uint64_t)anchor->gp_hi<<32) + unit->gp_lo
    );
    int start = 0;
    for (int x = 0; x <= unit->offsetssz; x++) {
        OffsetUnit* offsets = (anchor->offsets + unit->offsets);
        Instruction buf;
        decode_instr(context, &buf);
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
    context_free(context);
    */
}