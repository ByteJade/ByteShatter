#include "memory.h"
#include "cache.h"
#include "core.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h> 

static Context* contexts = NULL;
static int max_process = 0;

void context_init() {
    max_process = sysconf(_SC_NPROCESSORS_ONLN);
    if (max_process) {
        success("Setup %i contexts", max_process);
    } else {
        warning("_SC_NPROCESSORS_ONLN failed. Setup only 1 context");
        max_process = 1;
    }
    contexts = (Context*)malloc(sizeof(Context)*max_process);
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
    context->block = NULL;
    context->offsets = current->offsets + current->offsets_p;
    context->guest = (uint8_t*)(gp & (~(uint64_t)UINT32_MAX));
    context->host = current->host;
    context->gp = gp;
    context->hp = current->host_p;
    context->loffp = 0;
    print("Setup context %p-%x", context->guest, context->gp);
    return context;
}
void context_free(Context* context) {
    atomic_store(&context->in_use, 0);
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