// SPDX-License-Identifier: MIT
#include "FileBridge.h"
#include "EventBridge.h"
#include "GameMemory.h"
#include "GuestAccess.h"
#include <cstring>
#include <cstdio>
#include <stdexcept>
static nfs::GameMemory memory;
extern "C" void *nfs_host_address(uint32_t address,size_t size){auto *p=memory.resolve(address,size);if(!p)throw std::out_of_range("Overlapped span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t id;if(!memory.encode(p,id))throw std::out_of_range("Overlapped pointer");return id;}
struct Record {uint32_t internal,bytes,offset,high,event;};
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(int argc,char **argv){
    CHECK(argc==2);alignas(16) uint8_t data[512];std::memset(data,0xcc,sizeof(data));CHECK(memory.map(0x80000000,data,sizeof(data)));
    std::strcpy(reinterpret_cast<char *>(data),"overlapped.dat");std::memcpy(data+128,"abcdef",6);
    nfs::FileBridge files(argv[1]);nfs::EventBridge events;nfs::bind_file_bridge(&files);nfs::bind_event_bridge(&events);
    auto h=CreateFileA_wrap(int32_t(0x80000000u),int32_t(0xc0000000u),0,0,2,0,0);CHECK(h!=-1);
    CHECK(WriteFile_wrap(h,int32_t(0x80000080u),6,int32_t(0x80000060u),0));CHECK(SetFilePointer_wrap(h,3,0,0)==3 && SetEndOfFile_wrap(h) && GetFileSize_wrap(h,0)==3);
    CHECK(SetFilePointer_wrap(h,8,0,0)==8 && SetEndOfFile_wrap(h) && GetFileSize_wrap(h,0)==8 && CloseHandle_wrap(h));
    h=CreateFileA_wrap(int32_t(0x80000000u),int32_t(0xc0000000u),0,0,3,int32_t(0x40000000u),0);CHECK(h!=-1 && SetFilePointer_wrap(h,2,0,0)==2);
    const auto event=events.create(true,false);Record record{0,0,1,0,event};guest_store(0x80000103u,record);
    CHECK(ReadFile_wrap(h,int32_t(0x800000a0u),3,int32_t(0x80000063u),int32_t(0x80000103u)));
    CHECK(data[160]=='b' && data[161]=='c' && data[162]==0 && guest_load<uint32_t>(0x80000063u)==3 && SetFilePointer_wrap(h,0,0,1)==2);
    record=guest_load<Record>(0x80000103u);CHECK(record.internal==0 && record.bytes==3 && record.offset==1 && data[258]==0xcc && data[279]==0xcc);
    CHECK(events.wait(&event,1,false,0)==0 && GetOverlappedResult_wrap(h,int32_t(0x80000103u),int32_t(0x80000063u),0) && guest_load<uint32_t>(0x80000063u)==3);
    record.offset=7;guest_store(0x80000103u,record);CHECK(WriteFile_wrap(h,int32_t(0x80000080u),2,0,int32_t(0x80000103u)) && GetFileSize_wrap(h,0)==9);
    record.offset=20;guest_store(0x80000103u,record);CHECK(!ReadFile_wrap(h,int32_t(0x800000a0u),2,0,int32_t(0x80000103u)) && GetLastError_wrap()==38);
    CHECK(!GetOverlappedResult_wrap(h,int32_t(0x80000103u),int32_t(0x80000063u),1) && GetLastError_wrap()==38 && guest_load<uint32_t>(0x80000063u)==0);
    CHECK(!GetOverlappedResult_wrap(h,int32_t(0x80000120u),int32_t(0x80000063u),0) && GetLastError_wrap()==87);
    record.high=1;guest_store(0x80000103u,record);CHECK(!ReadFile_wrap(h,int32_t(0x800000a0u),1,0,int32_t(0x80000103u)) && GetLastError_wrap()==50);
    bool bounds=false;try{ReadFile_wrap(h,int32_t(0x800001ffu),3,0,int32_t(0x80000103u));}catch(const std::out_of_range &){bounds=true;} // high word rejected before buffer validation
    CHECK(!bounds);record.high=0;guest_store(0x80000103u,record);try{ReadFile_wrap(h,int32_t(0x800001ffu),3,0,int32_t(0x80000103u));}catch(const std::out_of_range &){bounds=true;}CHECK(bounds);
    CHECK(CloseHandle_wrap(h));nfs::bind_event_bridge(nullptr);nfs::bind_file_bridge(nullptr);
    std::puts("PASS: file truncate/extend, 20-byte unaligned overlapped records, positioned read/write without cursor changes, completion/event signalling, EOF, unknown records, high words and bounds");
}
