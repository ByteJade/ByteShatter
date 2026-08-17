#include "memory.h"
#include "cache.h"
#include "core.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

static uint8_t* guest = NULL;
static uint32_t* host = NULL;
/*
TODO: for multithreading,
local hp and gp for each thread
and host mutex
*/
uint32_t hostsz = 0;
uint32_t gp = 0;

void setup_context(Context* context, uint64_t gp) {
    Anchor* current = cache_get_anchor(gp);
    context->block = NULL;
    context->offsets = current->offsets + current->offsets_p;
    context->guest = (uint8_t*)(gp & (~UINT32_MAX));
    context->host = current->host;
    context->gp = gp;
    context->hp = current->host_p;
    context->loffp = 0;
}

void memory_init(uint32_t guest_size) {
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
}
void memory_fini(void) {
    if (host) munmap(host, hostsz);
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

uint8_t fetch8(void) {
    uint8_t* src = (uint8_t*)(guest + gp);
    gp += 1;
    return *src;
}
uint16_t fetch16(void) {
    uint16_t* src = (uint16_t*)(guest + gp);
    gp += 2;
    return *src;
}
uint32_t fetch32(void) {
    uint32_t* src = (uint32_t*)(guest + gp);
    gp += 4;
    return *src;
}
uint32_t fetch64(void) {
    uint64_t* src = (uint64_t*)(guest + gp);
    gp += 8;
    return *src;
}

void set_guest(uint64_t new_guest) { guest = (uint8_t*)new_guest; }

void set_gp(uint32_t new_gp) { gp = new_gp; }

uint64_t get_gp(void) { return gp; }

uint32_t* get_host(void) { return host; }
uint8_t* get_guest(void) { return guest; }