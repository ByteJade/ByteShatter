#include "memory.h"
#include "cache.h"
#include "core.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

static uint32_t* host = NULL;
/*
TODO: for multithreading,
local hp and gp for each thread
and host mutex
*/
uint32_t hostsz = 0;

void setup_context(Context* context, uint64_t gp) {
    Anchor* current = cache_get_anchor(gp);
    context->block = NULL;
    context->offsets = current->offsets + current->offsets_p;
    context->guest = (uint8_t*)(gp & (~(uint64_t)UINT32_MAX));
    context->host = current->host;
    context->gp = gp;
    context->hp = current->host_p;
    context->loffp = 0;
    print("Setup context %p-%x", context->guest, context->gp);
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
    hostsz = guest_size;
    host = mmap(
        NULL, hostsz,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANON | MAP_PRIVATE,
        -1, 0
    );
    if (host == MAP_FAILED) {
        panic("MMAP::FAIL");
    }
    success("host mmap %li", hostsz);
    return guest;
}
/* get pointer to host memory */
uint32_t* get_host(void) {return host;}