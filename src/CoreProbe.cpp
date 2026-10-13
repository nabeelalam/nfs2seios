// SPDX-License-Identifier: MIT
#include "GameMemory.h"
extern "C" bool nfs_core_probe() {
    uint8_t storage[64]{};
    nfs::GameMemory memory;
    uint32_t address = 0, value = 0;
    return memory.map(0x80000000, storage, sizeof(storage)) &&
        memory.encode(storage + 1, address) && address == 0x80000001 &&
        memory.store32(address, 0xabcdef98) && memory.load32(address, value) &&
        value == 0xabcdef98 && !memory.resolve(0x8000003f, 2);
}
