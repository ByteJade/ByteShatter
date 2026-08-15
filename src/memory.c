#include "memory.h"
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
uint32_t hp = 0;

void memory_init(uint32_t guest_size) {
    hostsz = guest_size * 1.5;
    // host code ~1.5 times larger, than guest
    host = mmap(
        NULL, hostsz,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANON | MAP_PRIVATE,
        -1, 0
    );
    if (host == MAP_FAILED) {
        panic("MMAP::FAIL");
    }
    success("host and guest mmap");
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
void memory_clear_host(void) {
    hp = 0;
}
// TODO: check host overflow
void emit32(uint32_t data) {
    host[hp++] = data;
}
void patch32() {
    hp -= 1;
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

void set_guest(uint64_t new_guest) { guest = (uint8_t*)new_guest; }

void set_gp(uint32_t new_gp) { gp = new_gp; }
void set_hp(uint32_t new_hp) { hp = new_hp; }

uint64_t get_hp(void) { return hp; }
uint64_t get_gp(void) { return gp; }

uint32_t* get_host(void) { return host; }
uint8_t* get_guest(void) { return guest; }