// SPDX-License-Identifier: MIT
#include "RendererSurface.h"
#undef true
#undef false
#include "GameMemory.h"
#include "HeapArena.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
static nfs::GameMemory memory;static nfs::HeapArena heap;
alignas(16) static uint8_t storage[2*1024*1024];
static unsigned reads,writes;static bool fail_read,fail_write;
extern "C" void *malloc32(size_t size){return heap.allocate(size);}
extern "C" void free32(void *p){if(!heap.release(p))throw std::runtime_error("Bad surface free");}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t id;if(!memory.encode(p,id))throw std::out_of_range("surface pointer");return id;}
extern "C" void *nfs_host_address(uint32_t id,size_t size){auto *p=memory.resolve(id,size);if(!p)throw std::out_of_range("surface span");return p;}
static BOOL read(GrBuffer_t,GrOriginLocation_t origin,void *p){++reads;if(fail_read)return 0;std::memset(p,0,NFS_SURFACE_BYTES);static_cast<uint16_t *>(p)[origin==GR_ORIGIN_UPPER_LEFT?0:640*479]=0xf800;return 1;}
static BOOL write(GrBuffer_t,GrOriginLocation_t,const void *p){++writes;if(fail_write)return 0;if(static_cast<const uint16_t *>(p)[1]!=0x07e0)throw std::runtime_error("Lost surface pixels");return 1;}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(){
    CHECK(heap.initialize(memory,0x10000000,storage,sizeof(storage)));
    struct {uint32_t before;GrHwConfiguration caps;uint32_t after;} record{0x12345678,{},0x87654321};
    CHECK(nfs_glide_query_hardware(&record.caps) && record.caps.num_sst==1 && record.before==0x12345678 && record.after==0x87654321);
    CHECK(record.caps.SSTs[0].sstBoard.VoodooConfig.nTexelfx==1 && record.caps.SSTs[0].sstBoard.VoodooConfig.tmuConfig[0].tmuRam==2 && !record.caps.SSTs[1].sstBoard.VoodooConfig.fbRam);
    GrLfbInfo_t info{};info.size=20;const NfsSurfaceOperations ops{read,write};const auto available=heap.available_bytes();
    CHECK(nfs_glide_lfb_lock(1,1,0,0,0,&info,ops));CHECK(info.size==20 && info.strideInBytes==1280 && info.origin==0 && info.writeMode==0 && info.lfbPtr);
    auto *p=static_cast<uint16_t *>(nfs_host_address(info.lfbPtr,NFS_SURFACE_BYTES));CHECK(p[0]==0xf800);p[1]=0x07e0;
    CHECK(!nfs_glide_lfb_lock(1,1,0,0,0,&info,ops) && !nfs_glide_lfb_unlock(1,0));
    fail_write=true;CHECK(!nfs_glide_lfb_unlock(1,1) && heap.available_bytes()<available);fail_write=false;
    CHECK(nfs_glide_lfb_unlock(1,1) && heap.available_bytes()==available && writes==2);
    CHECK(!nfs_glide_lfb_unlock(1,1));
    CHECK(nfs_glide_lfb_lock(0,0,0,1,0,&info,ops));CHECK(info.origin==1);CHECK(nfs_glide_lfb_unlock(0,0) && writes==2);
    fail_read=true;CHECK(!nfs_glide_lfb_lock(1,1,0,0,0,&info,ops) && heap.available_bytes()==available);fail_read=false;
    info.size=19;CHECK(!nfs_glide_lfb_lock(1,1,0,0,0,&info,ops));info.size=20;
    CHECK(!nfs_glide_lfb_lock(1,2,0,0,0,&info,ops) && !nfs_glide_lfb_lock(1,1,5,0,0,&info,ops) && !nfs_glide_lfb_lock(1,1,0,0,1,&info,ops));
    CHECK(nfs_glide_lfb_lock(1,1,0,0,0,&info,ops));nfs_glide_lfb_reset();CHECK(heap.available_bytes()==available);
    std::puts("PASS: Glide capabilities, exact RGB565 metadata, preserved partial writes, lock ownership, read/write errors, unsupported modes and heap retirement");
}
