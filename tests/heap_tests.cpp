#include "HeapArena.h"
#include "MemoryWrappers.h"
#include <stdexcept>
#include <cstdio>
#include <vector>
#include <thread>
#include <atomic>
#include <future>
#include <new>
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)
static nfs::GameMemory *active_memory;
static nfs::HeapArena *active_heap;
extern "C" void *malloc32(size_t size) { return active_heap->allocate(size); }
extern "C" void free32(void *p) { if (!active_heap->release(p)) throw std::out_of_range("Invalid heap pointer"); }
extern "C" void *nfs_host_address(uint32_t address, size_t length) {
    if (!active_heap->contains(address, length)) throw std::out_of_range("Unallocated heap address");
    return active_memory->resolve(address, length);
}
extern "C" uint32_t nfs_guest_address(const void *p) {
    uint32_t address = 0;
    if (!active_memory->encode(p, address)) throw std::out_of_range("Unmapped pointer");
    return address;
}
int main() {
    alignas(16) uint8_t storage[65536];
    nfs::GameMemory memory;
    CHECK(!memory.map(0x1000, reinterpret_cast<uint8_t *>(UINTPTR_MAX-1), 4));
    {
        nfs::HeapArena arena;
        CHECK(arena.initialize(memory, 0x80000000, storage, sizeof(storage)));
        active_heap = &arena; active_memory = &memory;
        CHECK(reinterpret_cast<uintptr_t>(storage) > UINT32_MAX);
        void *a = arena.allocate(31), *b = arena.allocate_zeroed(4, 16);
        CHECK(a && b && a != b && reinterpret_cast<uintptr_t>(a) % 16 == 0);
        uint32_t guest = 0;
        CHECK(memory.encode(a, guest) && guest == 0x80000000);
        CHECK(arena.contains(guest+31, 1) && !arena.contains(guest+31, 2));
        std::memset(a, 0x9b, 31);
        for (unsigned i=0; i<64; ++i) CHECK(static_cast<uint8_t *>(b)[i] == 0);
        CHECK(!arena.release(static_cast<uint8_t *>(a)+1));
        CHECK(!arena.resize(a, SIZE_MAX));
        CHECK(static_cast<uint8_t *>(a)[30] == 0x9b);
        a = arena.resize(a, 128); CHECK(a);
        for (unsigned i=0; i<31; ++i) CHECK(static_cast<uint8_t *>(a)[i] == 0x9b);
        CHECK(arena.release(a) && !arena.release(a));
        CHECK(arena.release(b));
        CHECK(!arena.allocate_zeroed(SIZE_MAX, 2));
        std::vector<void *> slots;
        while (void *p = arena.allocate(64)) slots.push_back(p);
        CHECK(slots.size() == sizeof(storage)/64);
        for (size_t i=0; i<slots.size(); i+=2) CHECK(arena.release(slots[i]));
        for (size_t i=0; i<slots.size(); ++i) {
            const uint32_t address=0x80000000+uint32_t(i*64);
            CHECK(arena.contains(address,64)==bool(i%2));
            CHECK(arena.contains(address+63,1)==bool(i%2));
            CHECK(!arena.contains(address+63,2));
        }
        CHECK(!arena.contains(0x7fffffff,1) && !arena.contains(0x80010000,1) && !arena.contains(0x80000040,SIZE_MAX));
        CHECK(!arena.allocate(128)); // Fragmentation, not exhaustion of bytes.
        for (size_t i=1; i<slots.size(); i+=2) CHECK(arena.release(slots[i]));
        CHECK(!arena.contains(0x80000000, 1));
        void *whole = arena.allocate(sizeof(storage)); CHECK(whole == storage);
        CHECK(!arena.allocate(1) && arena.release(whole));
        void *cached=arena.allocate(64);CHECK(cached==storage);
        CHECK(arena.contains(0x80000000,64));
        std::promise<void> freed;
        std::thread retire([&]{if(!arena.release(cached))std::terminate();freed.set_value();});
        freed.get_future().wait();retire.join();CHECK(!arena.contains(0x80000000,1));
        cached=arena.allocate(16);CHECK(cached==storage);
        CHECK(arena.contains(0x80000000,16) && !arena.contains(0x80000000,17));
        CHECK(arena.release(cached));
        const int32_t wrapped = calloc_wrap(8, 4);
        CHECK(uint32_t(wrapped) == 0x80000000 && wrapped < 0);
        auto *native = static_cast<uint8_t *>(nfs_host_address(uint32_t(wrapped), 32));
        CHECK(reinterpret_cast<uintptr_t>(native) > UINT32_MAX);
        for (unsigned i=0; i<32; ++i) CHECK(native[i] == 0);
        CHECK(!calloc_wrap(-1, 2) && !malloc_wrap(-1));
        CHECK(free_wrap(wrapped) == 0 && free_wrap(0) == 0);
        bool rejected = false;
        try { free_wrap(wrapped); } catch (const std::out_of_range &) { rejected = true; }
        CHECK(rejected);
        std::atomic<bool> valid{true};
        std::vector<std::thread> workers;
        for (unsigned t=0; t<4; ++t) workers.emplace_back([&] {
            for (unsigned i=0; i<1000; ++i) {
                void *p = arena.allocate_zeroed(1, 32);
                uint32_t address=0;
                if(!p || !memory.encode(p,address) || !arena.contains(address,32) ||
                   arena.contains(address,33) || arena.contains(address+31,2))valid=false;
                if(!arena.release(p))valid=false;
            }
        });
        for (auto &worker : workers) worker.join();
        CHECK(valid);
    }
    CHECK(!memory.resolve(0x80000000));
    CHECK(!memory.unmap(0x80000000));
    // A new arena can occupy exactly the same native object and guest region.
    alignas(nfs::HeapArena) uint8_t arena_bytes[sizeof(nfs::HeapArena)];
    for(size_t size:{size_t(64),size_t(16)}){
        auto *arena=new(arena_bytes)nfs::HeapArena;
        CHECK(arena->initialize(memory,0x80000000,storage,sizeof(storage)));
        CHECK(arena->allocate(size)==storage);
        CHECK(arena->contains(0x80000000,size));
        CHECK(!arena->contains(0x80000000,size+1));
        arena->~HeapArena();
    }
    std::puts("PASS: high-address heap, allocation/resize/OOM, fragmentation/coalescing, typed game malloc/calloc/free ABI, 4000 threaded allocations, teardown");
}
