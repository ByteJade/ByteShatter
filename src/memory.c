#include "memory.h"
#include "cache.h"
#include "core.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>


void setup_context(Context* context, uint64_t gp) {
    Anchor* current = cache_get_anchor(gp);
    context->block = NULL;
    context->offsets = current->offsets + current->offsets_p;
    context->guest = (uint8_t*)(gp & (~(uint64_t)UINT32_MAX));
    context->host = current->host;
    context->gp = gp & UINT32_MAX;
    context->hp = current->host_p;
    context->loffp = 0;
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