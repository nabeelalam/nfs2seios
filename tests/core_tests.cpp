#include "GameMemory.h"
#include "FPU.h"
#include <cstdio>
#include <cstring>
#include <new>
#include <limits>

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)
static void callback(void *p) { ++*static_cast<int *>(p); }
static void other_callback(void *p) { *static_cast<int *>(p)+=2; }
int main() {
    uint8_t bytes[64]{};
    nfs::GameMemory memory;
    CHECK(memory.map(0x80000000u, bytes, sizeof(bytes)));
    CHECK(!memory.map(0x80000010u, bytes + 32, 16));
    CHECK(!memory.map(0x1000, bytes + 1, 4));
    CHECK(!memory.map(0xfffffff0, bytes, 32));
    CHECK(!memory.map(0, bytes, 1));
    CHECK(memory.resolve(0) == nullptr);
    CHECK(memory.resolve(0x80000040u) == nullptr);
    CHECK(memory.resolve(0x8000003fu, 2) == nullptr);
    CHECK(memory.resolve(0x80000001u, SIZE_MAX) == nullptr);
    CHECK(memory.store32(0x80000001u, 0xfedcba98u)); // Unaligned, bit 31 set.
    CHECK(bytes[1] == 0x98 && bytes[4] == 0xfe);
    uint32_t value = 0;
    CHECK(memory.load32(0x80000001u, value) && value == 0xfedcba98u);
    CHECK(!memory.store32(0x8000003du, 1));
    CHECK(memory.encode(bytes + 31, value) && value == 0x8000001fu);
    CHECK(!memory.encode(bytes + 64, value));
    CHECK(memory.encode(nullptr, value) && value == 0);
    uint8_t end[4]{};
    CHECK(memory.map(0xfffffffcu, end, 4));
    CHECK(memory.store32(0xfffffffcu, 0x12345678));
    CHECK(memory.load32(0xfffffffcu, value) && value == 0x12345678);
    nfs::Functions functions;
    CHECK(functions.add(0x401080, callback));
    CHECK(!functions.add(0x401080, callback));
    CHECK(functions.find(0x401081) == nullptr);
    int called = 0;
    functions.find(0x401080)(&called);
    CHECK(called == 1);
    // Fill in a permuted order and verify identities, holes and capacity.
    // Native callback addresses must survive registry ordering unchanged.
    nfs::Functions mixed;
    CHECK(!mixed.add(0,callback) && !mixed.add(1,nullptr));
    for(unsigned i=0;i<4096;++i){
        const uint32_t id=1+(i*173u)%4096;
        CHECK(mixed.add(id,id%2?callback:other_callback));
    }
    for(uint32_t id=1;id<=4096;++id)CHECK(mixed.find(id)==(id%2?callback:other_callback));
    CHECK(!mixed.add(4097,callback) && !mixed.add(1,other_callback));
    CHECK(mixed.find(0)==nullptr && mixed.find(4097)==nullptr && mixed.find(UINT32_MAX)==nullptr);
    CHECK(mixed.find(1)==callback && mixed.find(4096)==other_callback);
    // Construct on poisoned memory so the missing upstream top initialization fails.
    alignas(FPU) unsigned char storage[sizeof(FPU)];
    std::memset(storage, 0xa5, sizeof(storage));
    FPU *fpu = new (storage) FPU;
    CHECK(fpu->status.top == 0);
    CHECK(fpu->control.word == 0x037f);
    fpu->push(1.25); fpu->push(2.5);
    CHECK(fpu->st(0) == 2.5 && fpu->st(1) == 1.25);
    fpu->pop(); CHECK(fpu->st(0) == 1.25);
    fpu->compare(1, 2); CHECK(fpu->status.flags == FPU::C0);
    fpu->compare(2, 1); CHECK(fpu->status.flags == 0);
    fpu->compare(2, 2); CHECK(fpu->status.flags == FPU::C3);
    fpu->compare(std::numeric_limits<double>::quiet_NaN(), 1);
    CHECK(fpu->status.flags == (FPU::C0 | FPU::C2 | FPU::C3));
    fpu->init(); CHECK(fpu->status.top == 0);
    fpu->~FPU();
    std::puts("PASS: address boundaries, pointer round trips, endian/unaligned access, callbacks, upstream FPU regression");
}
