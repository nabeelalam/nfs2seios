// SPDX-License-Identifier: MIT
#include "HeapArena.h"
#include <cassert>
#include <thread>
#include <atomic>
#include <cstdio>
int main(){
 nfs::HeapArena unbound;assert(!unbound.resolve_owned_chunk(0x10000000,1));
 alignas(16) uint8_t storage[8208];nfs::GameMemory map;nfs::HeapArena arena;assert(arena.initialize(map,0x10000000,storage,sizeof(storage)));auto *large=arena.allocate(8192);assert(large==storage);
 unsigned checked=0;
 for(size_t offset:{size_t(0),size_t(1),size_t(4095),size_t(4096),size_t(8191),size_t(8192),size_t(8207)})for(size_t length:{size_t(0),size_t(1),size_t(2),size_t(16),size_t(4096),SIZE_MAX}){
  auto *actual=arena.resolve_owned_chunk(uint32_t(0x10000000+offset),length);bool valid=offset<8192&&length&&length<=4096-offset%4096;assert(actual==(valid?storage+offset:nullptr));++checked;
 }
 std::atomic<unsigned> valid_reads{0};std::thread reader([&]{for(unsigned i=0;i<20000;++i){assert(arena.resolve_owned_chunk(0x10000000+(i&2047),16)==storage+(i&2047));}valid_reads=20000;});
 for(unsigned i=0;i<10000;++i){void *tail=arena.allocate(16);assert(tail==storage+8192);assert(!arena.resolve_owned_chunk(0x10002000,16));assert(arena.release(tail));}reader.join();assert(valid_reads==20000);
 assert(arena.release(large));assert(!arena.resolve_owned_chunk(0x10000000,1));assert(!arena.resolve_owned_chunk(0x10001000,1));void *small=arena.allocate(16);assert(small==storage);assert(arena.contains(0x10000000,16)&&!arena.resolve_owned_chunk(0x10000000,16));assert(!arena.resolve_owned_chunk(0x10001000,1));
 alignas(16) uint8_t high_storage[4096];nfs::GameMemory high_map;nfs::HeapArena high;assert(high.initialize(high_map,0xfffff000,high_storage,sizeof(high_storage)));assert(high.allocate(4096)==high_storage);assert(high.resolve_owned_chunk(0xffffffff,1)==high_storage+4095);assert(!high.resolve_owned_chunk(0xffffffff,2));assert(!high.resolve_owned_chunk(0xffffffff,SIZE_MAX));assert(!high.resolve_owned_chunk(0,1));
 std::printf("{\"passed\":true,\"boundaryCases\":%u,\"concurrentStableOwnedReads\":20000,\"unrelatedAllocFreeCycles\":10000,\"freedAndPartiallyReusedChunksRefused\":true,\"topOfGuestAddressSpaceChecked\":true}\n",checked);
}
