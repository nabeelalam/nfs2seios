#include "FormatWrappers.h"
#include "GuestAccess.h"
#include "GameMemory.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
static nfs::GameMemory memory;
extern "C" void *nfs_host_address(uint32_t address,size_t size) { void *p=memory.resolve(address,size); if(!p)throw std::out_of_range("Bad format span"); return p; }
extern "C" uint32_t nfs_guest_address(const void *p) { uint32_t address=0; if(!memory.encode(p,address))throw std::out_of_range("Bad format pointer");return address; }
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main() {
    alignas(16) uint8_t bytes[2048]; std::memset(bytes,0xcc,sizeof(bytes));
    CHECK(memory.map(0x80000000,bytes,sizeof(bytes)) && reinterpret_cast<uintptr_t>(bytes)>UINT32_MAX);
    const int32_t format=int32_t(0x80000003u),args=int32_t(0x80000103u),output=int32_t(0x80000203u);
    const auto run=[&](const char *pattern) {std::strcpy(reinterpret_cast<char *>(bytes+3),pattern);return vsprintf_wrap(output,format,args);};
    std::strcpy(reinterpret_cast<char *>(bytes+128),"fedata/pc/");
    guest_store<uint32_t>(uint32_t(args),0x80000080);
    guest_store<int32_t>(uint32_t(args)+4,-12);
    CHECK(run("%s%d.qfs")==17 && std::strcmp(reinterpret_cast<char *>(bytes+515),"fedata/pc/-12.qfs")==0);
    guest_store<int32_t>(uint32_t(args),8); guest_store<int32_t>(uint32_t(args)+4,3); guest_store<uint32_t>(uint32_t(args)+8,0x80000080);
    CHECK(run("[%*.*s]")==10 && std::strcmp(reinterpret_cast<char *>(bytes+515),"[     fed]")==0);
    guest_store<int32_t>(uint32_t(args),2); guest_store<double>(uint32_t(args)+4,3.125);
    CHECK(run("%.*f")==4 && std::strcmp(reinterpret_cast<char *>(bytes+515),"3.12")==0);
    guest_store<int32_t>(uint32_t(args),INT32_MIN); guest_store<uint32_t>(uint32_t(args)+4,UINT32_MAX);
    CHECK(run("%ld %08x")==20 && std::strcmp(reinterpret_cast<char *>(bytes+515),"-2147483648 ffffffff")==0);
    guest_store<int64_t>(uint32_t(args),INT64_MIN);
    CHECK(run("%lld")==20 && std::strcmp(reinterpret_cast<char *>(bytes+515),"-9223372036854775808")==0);
    guest_store<int32_t>(uint32_t(args),-1); guest_store<uint32_t>(uint32_t(args)+4,UINT32_MAX);
    guest_store<int64_t>(uint32_t(args)+8,INT64_MIN);
    CHECK(run("%zd %tu %jd")==34 && std::strcmp(reinterpret_cast<char *>(bytes+515),"-1 4294967295 -9223372036854775808")==0);
    guest_store<int32_t>(uint32_t(args),-8); guest_store<int32_t>(uint32_t(args)+4,-1);
    guest_store<uint32_t>(uint32_t(args)+8,0x80000080);
    CHECK(run("[%*.*s]")==12 && std::strcmp(reinterpret_cast<char *>(bytes+515),"[fedata/pc/]")==0);
    guest_store<uint32_t>(uint32_t(args),0x80000783);
    CHECK(run("100%%%n")==4 && guest_load<int32_t>(0x80000783u)==4);
    CHECK(bytes[514]==0xcc);
    guest_store<uint32_t>(uint32_t(args),0x800007ff); bool rejected=false;
    try{run("%s");}catch(const std::out_of_range &){rejected=true;} CHECK(rejected);
    guest_store<int32_t>(uint32_t(args),20000); rejected=false;
    try{run("%*d");}catch(const std::out_of_range &){rejected=true;} CHECK(rejected);
    std::strcpy(reinterpret_cast<char *>(bytes+3),"%f"); rejected=false;
    try{vsprintf_wrap(output,format,int32_t(0x800007fdu));}catch(const std::out_of_range &){rejected=true;} CHECK(rejected);
    std::strcpy(reinterpret_cast<char *>(bytes+3),"boundary"); rejected=false;
    try{vsprintf_wrap(int32_t(0x800007fdu),format,args);}catch(const std::out_of_range &){rejected=true;} CHECK(rejected);
    CHECK(bytes[2045]==0xcc && bytes[2046]==0xcc && bytes[2047]==0xcc);
    std::puts("PASS: 32-bit guest varargs, nested string pointers above 4GB, dynamic width/precision, unaligned double, integer lengths, percent/count, bounds");
}
