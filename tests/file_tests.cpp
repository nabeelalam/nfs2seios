#include "FileBridge.h"
#include "GuestAccess.h"
#include "GameMemory.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); return 1; } } while(0)
static nfs::GameMemory memory;
extern "C" void *nfs_host_address(uint32_t address,size_t length) {
    void *p=memory.resolve(address,length);
    if (!p) throw std::out_of_range("Bad guest file buffer");
    return p;
}
extern "C" uint32_t nfs_guest_address(const void *p) {
    uint32_t address=0;
    if (!memory.encode(p,address)) throw std::out_of_range("Unmapped file pointer");
    return address;
}
int main(int argc,char **argv) {
    CHECK(argc==2);
    alignas(16) uint8_t data[512]{};
    CHECK(reinterpret_cast<uintptr_t>(data)>UINT32_MAX);
    CHECK(memory.map(0x80000000,data,sizeof(data)));
    std::string normalized;
    CHECK(nfs::FileBridge::relative_path(".\\GAMEDATA\\CAR.DAT",normalized) && normalized=="gamedata/car.dat");
    for (const auto &path:{"../x","x/../../y","/x","C:\\x","\\\\server\\x"})
        CHECK(!nfs::FileBridge::relative_path(path,normalized));
    const std::string filename="file-bridge-"+std::to_string(reinterpret_cast<uintptr_t>(data))+".bin";
    const std::string absolute=std::string(argv[1])+"/"+filename;
    struct Cleanup { std::string path; ~Cleanup(){std::remove(path.c_str()); nfs::bind_file_bridge(nullptr);} } cleanup{absolute};
    nfs::FileBridge files(argv[1]); nfs::bind_file_bridge(&files);
    std::strcpy(reinterpret_cast<char *>(data),filename.c_str());
    const int32_t path=int32_t(0x80000000u), buffer=int32_t(0x80000100u), output=int32_t(0x80000183u);
    const int32_t handle=CreateFileA_wrap(path,int32_t(0xc0000000u),0,0,2,0,0);
    CHECK(handle<0 && handle!=-1);
    std::memcpy(data+256,"original asset bytes",20);
    CHECK(WriteFile_wrap(handle,buffer,20,output,0)==1 && guest_load<uint32_t>(output)==20);
    CHECK(FlushFileBuffers_wrap(handle)==1);
    CHECK(GetFileSize_wrap(handle,output)==20 && guest_load<uint32_t>(output)==0);
    CHECK(SetFilePointer_wrap(handle,0,0,0)==0);
    std::memset(data+256,0,32);
    CHECK(ReadFile_wrap(handle,buffer,32,output,0)==1 && guest_load<uint32_t>(output)==20);
    CHECK(std::memcmp(data+256,"original asset bytes",20)==0);
    CHECK(ReadFile_wrap(handle,0,0,output,0)==1 && guest_load<uint32_t>(output)==0);
    bool invalid_overlapped=false;
    try{ReadFile_wrap(handle,buffer,1,output,123);}catch(const std::out_of_range &){invalid_overlapped=true;}
    CHECK(invalid_overlapped && SetFilePointer_wrap(handle,0,0,1)==20);
    CHECK(CloseHandle_wrap(handle)==1 && CloseHandle_wrap(handle)==0);
    CHECK(ReadFile_wrap(handle,buffer,1,output,0)==0 && GetLastError_wrap()==6);
    const int32_t next=CreateFileA_wrap(path,int32_t(0x80000000u),0,0,3,0,0);
    CHECK(next!=-1 && next!=handle);
    CHECK(ReadFile_wrap(handle,buffer,1,output,0)==0); // Generation rejects stale ID after reuse.
    CHECK(CloseHandle_wrap(next)==1);
    std::vector<uint32_t> handles;
    for (unsigned i=0;i<256;++i) { const auto id=files.open(filename,"rb"); CHECK(id); handles.push_back(id); }
    CHECK(!files.open(filename,"rb"));
    for (const auto id:handles) CHECK(files.close(id));
    std::strcpy(reinterpret_cast<char *>(data+128),"rb");
    int32_t stream=fopen_wrap(path,int32_t(0x80000080u)); CHECK(stream);
    CHECK(fclose_wrap(stream)==0);
    const int32_t numeric=CreateFileA_wrap(path,int32_t(0xc0000000u),0,0,2,0,0);
    CHECK(numeric!=-1);
    std::strcpy(reinterpret_cast<char *>(data+256),"-123 2147483647 2147483648");
    CHECK(WriteFile_wrap(numeric,buffer,int32_t(std::strlen(reinterpret_cast<char *>(data+256))),output,0)==1);
    CHECK(SetFilePointer_wrap(numeric,0,0,0)==0);
    std::strcpy(reinterpret_cast<char *>(data+128),"%d\n");
    const int32_t format=int32_t(0x80000080u);
    CHECK(fscanf_wrap(numeric,format,output)==1 && guest_load<int32_t>(output)==-123);
    CHECK(fscanf_wrap(numeric,format,output)==1 && guest_load<int32_t>(output)==INT32_MAX);
    CHECK(fscanf_wrap(numeric,format,output)==0 && guest_load<int32_t>(output)==INT32_MAX);
    CHECK(CloseHandle_wrap(numeric)==1);
    bool rejected=false;
    try { ReadFile_wrap(stream,int32_t(0x800001ffu),4,output,0); } catch(const std::out_of_range &){rejected=true;}
    CHECK(rejected);
    std::puts("PASS: native file-handle IDs, stale/exhausted handles, sandbox path normalization, guest read/write buffers and unaligned outputs, EOF, bounds");
}
