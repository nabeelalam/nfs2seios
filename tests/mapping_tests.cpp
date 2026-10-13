// SPDX-License-Identifier: MIT
#include "FileMappings.h"
#include "FileBridge.h"
#include "GuestAccess.h"
#include "HeapArena.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
static nfs::GameMemory memory;static nfs::HeapArena heap;
alignas(16) static uint8_t bytes[2*1024*1024];
extern "C" void *malloc32(size_t size){return heap.allocate(size);}
extern "C" void free32(void *p){if(!heap.release(p))throw std::runtime_error("Mapping free");}
extern "C" void *nfs_host_address(uint32_t address,size_t size){auto *p=memory.resolve(address,size);if(!p || !heap.contains(address,size))throw std::out_of_range("Mapping span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t address;if(!memory.encode(p,address))throw std::out_of_range("Mapping pointer");return address;}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(int argc,char **argv){
    CHECK(argc==2 && heap.initialize(memory,0x10000000,bytes,sizeof(bytes)));
    const std::string path=std::string(argv[1])+"/mapping.dat";auto *file=std::fopen(path.c_str(),"wb");CHECK(file);for(int i=0;i<32;++i)std::fputc(i,file);std::fclose(file);
    nfs::FileBridge files(argv[1]);nfs::FileMappings mappings(files,1024);nfs::bind_file_bridge(&files);nfs::bind_file_mappings(&mappings);
    const size_t free_before=heap.available_bytes();auto h=files.open("MAPPING.DAT","rb");CHECK(h && files.seek(h,7,0)==7);
    auto committed=uint32_t(CreateFileMappingA_wrap(int32_t(h),0,0x08000002,0,0,0));CHECK(committed);
    auto committed_view=uint32_t(MapViewOfFile_wrap(int32_t(committed),4,0,0,0));CHECK(committed_view && guest_load<uint8_t>(committed_view+31)==31);
    CHECK(!MapViewOfFile_wrap(int32_t(committed),2,0,0,0) && GetLastError_wrap()==5);
    CHECK(UnmapViewOfFile_wrap(int32_t(committed_view)) && CloseHandle_wrap(int32_t(committed)) && heap.available_bytes()==free_before);
    CHECK(!CreateFileMappingA_wrap(int32_t(h),0,0x18000002,0,0,0) && GetLastError_wrap()==50);
    auto m=uint32_t(CreateFileMappingA_wrap(int32_t(h),0,2,0,0,0));CHECK(m && (m&0xff000000)==0xa0000000 && files.seek(h,0,1)==7);
    CHECK(!MapViewOfFile_wrap(int32_t(m),2,0,0,0) && GetLastError_wrap()==5);
    auto view=uint32_t(MapViewOfFile_wrap(int32_t(m),4,0,17,9));CHECK(view && guest_load<uint8_t>(view)==17 && guest_load<uint8_t>(view+8)==25);
    CHECK(!MapViewOfFile_wrap(int32_t(m),4,0,31,2));
    CHECK(CloseHandle_wrap(int32_t(h)) && CloseHandle_wrap(int32_t(m)) && !CloseHandle_wrap(int32_t(m)));
    CHECK(guest_load<uint8_t>(view+8)==25 && !UnmapViewOfFile_wrap(int32_t(view+1)));
    CHECK(UnmapViewOfFile_wrap(int32_t(view)) && !UnmapViewOfFile_wrap(int32_t(view)) && heap.available_bytes()==free_before);
    h=files.open("mapping.dat","r+b");CHECK(h && files.seek(h,3,0)==3);m=mappings.create(h,4,64);CHECK(m && files.size(h,nullptr)==64 && files.seek(h,0,1)==3);
    view=mappings.map(m,6,10,2);CHECK(view);auto alias=mappings.map(m,4,10,2);CHECK(alias==view);guest_store<uint8_t>(view,0xa5);
    CHECK(mappings.unmap(view) && guest_load<uint8_t>(alias)==0xa5 && mappings.unmap(alias) && files.seek(h,0,1)==3);
    CHECK(mappings.close(m) && files.close(h) && heap.available_bytes()==free_before);
    file=std::fopen(path.c_str(),"rb");CHECK(file);std::fseek(file,10,SEEK_SET);CHECK(std::fgetc(file)==0xa5);std::fseek(file,63,SEEK_SET);CHECK(std::fgetc(file)==0);std::fclose(file);
    h=files.open("mapping.dat","rb");m=mappings.create(h,8,0);CHECK(m);view=mappings.map(m,1,4,8);CHECK(view);guest_store<uint8_t>(view,0xff);CHECK(mappings.unmap(view));
    view=mappings.map(m,4,4,8);CHECK(guest_load<uint8_t>(view)==4);CHECK(mappings.unmap(view) && mappings.close(m) && files.close(h) && heap.available_bytes()==free_before);
    CHECK(!CreateFileMappingA_wrap(-1,0,2,0,0,0) && GetLastError_wrap()==50);
    h=files.open("mapping.dat","r+b");m=mappings.create(h,4,0);view=mappings.map(m,6,5,2);CHECK(view && mappings.map(m,4,5,2)==view);guest_store<uint8_t>(view,0xb6);
    CHECK(mappings.close(m) && files.close(h) && mappings.flush_and_unmap_all() && heap.available_bytes()==free_before);
    file=std::fopen(path.c_str(),"rb");CHECK(file);std::fseek(file,5,SEEK_SET);CHECK(std::fgetc(file)==0xb6);std::fclose(file);
    nfs::bind_file_mappings(nullptr);nfs::bind_file_bridge(nullptr);
    std::puts("PASS: high-address file views, retained file/mapping lifetime, offset/length and cursor preservation, alias references, writable persistence/extension, private copies, stale views and heap cleanup");
}
