#include "memory.h"
#include "cache.h"
#include "core.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h> 

#define START_OFFSETS 256
#define START_BLOCKS 256

static Context* contexts = NULL;
static int max_process = 0;
static int usage = 0;

void context_init() {
    max_process = sysconf(_SC_NPROCESSORS_ONLN);
    if (max_process) {
        success("Setup %i contexts", max_process);
    } else {
        warning("_SC_NPROCESSORS_ONLN failed. Setup only 1 context");
        max_process = 1;
    }
    usage += sizeof(Context)*max_process;
    contexts = (Context*)calloc(max_process, sizeof(Context));
}
Context* context_search() {
    for (int i = 0; i < max_process; i++) {
        Context* context = contexts + i;
        _Bool expected = 0;
        if (atomic_compare_exchange_strong(&context->in_use, &expected, 1)) {
            return context;
        }
    }
    return NULL;
}
Context* context_pull(uint64_t gp) {
    Context* context = context_search();
    if (!context) panic("CONTEXT::NO_FREE");
    Anchor* current = cache_get_anchor(gp);
    if (!contexts->offsets) {
        context->offsets_c = START_OFFSETS;
        contexts->offsets = (OffsetUnit*)malloc(START_OFFSETS*sizeof(OffsetUnit));
    }
    if (!contexts->blocks) {
        context->blocks_c = START_OFFSETS;
        contexts->blocks = (CacheUnit*)malloc(START_BLOCKS*sizeof(CacheUnit));
    }
    context->guest = (uint8_t*)(gp & (~(uint64_t)UINT32_MAX));
    context->host = current->host;
    context->gp = gp;
    print("Setup context %p-%x", context->guest, context->gp);
    return context;
}
void context_free(Context* context) {
    atomic_store(&context->in_use, 0);
}
void context_push_jump(Context* context, uint32_t jump) {
    if (context->jumps_p == context->jumps_c) {
        context->jumps_c *= 2;
        context->jumps = realloc(
            context->jumps,
            context->jumps_c*sizeof(uint32_t)
        );
    }
    context->jumps[context->jumps_p++] = jump;
}
int context_usage() {
    return usage;
}

void context_block_start(Context* context) {
    if (context->blocks_p+1 >= context->blocks_c) {
        panic("CONTEXT::BLOCKS::OVERFLOW");
    }
    // sort array
    uint32_t insert_pos = 0;
    while (insert_pos < context->blocks_p && 
           context->blocks[insert_pos].gp_lo < context->gp) {
        insert_pos++;
    }
    for (uint32_t rp = context->blocks_p; rp > insert_pos; rp--) {
        context->blocks[rp] = context->blocks[rp-1];
    }
    print("start block %x", insert_pos);
    CacheUnit* block = context->blocks + insert_pos;
    block->offsets = context->offsets_p;
    block->offsetssz = 0;
    block->end = 1;
    block->gp_lo = context->gp;
    block->hp = 0;
    context->block = block;
    context->blocks_p++;
}
void context_block_point(Context* context) {
    uint16_t goff = context->gp - context->block->gp_lo;
    if (goff == 0) return;
    if (goff > UINT8_MAX) {
        warning("CACHE::BLOCKS::BAD_OFFSET");
        context_block_end(context);
        context_block_start(context);
        goff = context->gp - context->block->gp_lo;
    }
    OffsetUnit* offset = &context->offsets[context->offsets_p++];
    offset->goff = goff;
    if (context->offsets_p >= context->offsets_c)
        panic("CACHE::OFFSET_OVERFLOW");
    if (context->offsets_p - context->block->offsets == UINT8_MAX) {
        context_block_end(context);
        context_block_start(context);
    }
}
void context_block_end(Context* context) {
    CacheUnit* block = context->block;
    block->end = context->gp - block->gp_lo;
    block->offsetssz = context->offsets_p - block->offsets;
}

void* mmap_guest(uint32_t guest_size) {
    void* guest = mmap(
        NULL, guest_size,
        PROT_READ | PROT_WRITE,
        MAP_ANON | MAP_PRIVATE,
        -1, 0
    );
    if (guest == MAP_FAILED) {
        panic("MMAP::FAIL");
    }
    return guest;
}