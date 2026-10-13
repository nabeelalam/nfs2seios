// SPDX-License-Identifier: MIT
#include "DirectoryBridge.h"
#include "FileBridge.h"
#include "GuestAccess.h"
#include "GameMemory.h"
#include <filesystem>
#include <cstdio>
#include <cstring>
#include <stdexcept>
static nfs::GameMemory memory;
extern "C" void *nfs_host_address(uint32_t address,size_t size){auto *p=memory.resolve(address,size);if(!p)throw std::out_of_range("directory span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t address;if(!memory.encode(p,address))throw std::out_of_range("directory pointer");return address;}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(int argc,char **argv){
    CHECK(argc==2);const auto root=std::filesystem::absolute(std::string(argv[1])+"/directory-fixture");std::filesystem::create_directories(root/"fedata"/"pc");
    for(const char *name:{"car.dat","track.dat","readme"}){auto *file=std::fopen((root/"fedata"/"pc"/name).string().c_str(),"wb");CHECK(file);std::fwrite("abc",1,3,file);std::fclose(file);}
    nfs::FileBridge files(root.string());nfs::DirectoryBridge directories(files.sandbox());nfs::bind_file_bridge(&files);nfs::bind_directory_bridge(&directories);
    alignas(16) uint8_t data[1024];std::memset(data,0xcc,sizeof(data));CHECK(memory.map(0x80000000,data,sizeof(data)));
    CHECK(GetCurrentDirectoryA_wrap(0,0)==11);std::strcpy(reinterpret_cast<char *>(data),"FEDATA\\PC");CHECK(SetCurrentDirectoryA_wrap(int32_t(0x80000000u)));
    const auto needed=GetCurrentDirectoryA_wrap(1,int32_t(0x80000060u));CHECK(needed==21 && data[96]==0xcc);
    CHECK(GetCurrentDirectoryA_wrap(64,int32_t(0x80000060u))==20 && !std::strcmp(reinterpret_cast<char *>(data+96),"C:\\NFSIISE\\fedata\\pc"));
    auto h=files.open("CAR.DAT","rb");CHECK(h && files.close(h));
    h=files.open("C:\\NFSIISE\\FEDATA\\PC\\TRACK.DAT","rb");CHECK(h && files.close(h));
    std::strcpy(reinterpret_cast<char *>(data),"*.DAT");h=uint32_t(FindFirstFileA_wrap(int32_t(0x80000000u),int32_t(0x80000103u)));CHECK(h!=UINT32_MAX);
    auto entry=guest_load<nfs::GameFindData>(0x80000103u);CHECK(!std::strcmp(entry.name,"car.dat") && entry.words[8]==3 && entry.words[0]==0x20 && data[258]==0xcc && data[579]==0xcc);
    CHECK(FindNextFileA_wrap(int32_t(h),int32_t(0x80000103u)));entry=guest_load<nfs::GameFindData>(0x80000103u);CHECK(!std::strcmp(entry.name,"track.dat"));
    CHECK(!FindNextFileA_wrap(int32_t(h),int32_t(0x80000103u)) && GetLastError_wrap()==18 && FindClose_wrap(int32_t(h)) && !FindClose_wrap(int32_t(h)));
    std::vector<nfs::DirectoryEntry> entries;CHECK(files.sandbox().list("*.*",entries) && entries.size()==3);
    CHECK(!files.sandbox().change("../../..") && !files.sandbox().resolve("D:\\escape",entries[0].name));
    std::strcpy(reinterpret_cast<char *>(data),"car.dat");CHECK(DeleteFileA_wrap(int32_t(0x80000000u)) && !files.open("car.dat","rb"));
    CHECK(files.sandbox().change("..") && files.sandbox().change("C:\\NFSIISE") && files.sandbox().current()=="C:\\NFSIISE");
    // Native symlink components must not escape or redirect imported asset paths.
    std::error_code error;std::filesystem::create_directory_symlink(root/"fedata",root/"redirect",error);
    if(!error){std::string native;CHECK(!files.sandbox().resolve("redirect/pc/track.dat",native));}
    nfs::bind_directory_bridge(nullptr);nfs::bind_file_bridge(nullptr);
    std::puts("PASS: virtual cwd/absolute paths, shared file namespace, 320-byte unaligned find records/canaries, wildcard discovery, metadata/stale handles, traversal/symlink rejection and file deletion");
}
