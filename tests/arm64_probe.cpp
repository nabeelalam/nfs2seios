#include "GameMemory.h"
extern "C" bool nfs_address_roundtrip(uint8_t *host, uint32_t *result) {
    nfs::GameMemory memory;
    return memory.map(0x80000000, host, 64) && memory.store32(0x80000001, 0x12345678) &&
        memory.load32(0x80000001, *result);
}
