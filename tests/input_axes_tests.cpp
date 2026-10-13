#include "InputAxes.h"
#include "GameMemory.h"
#include "GuestAccess.h"
#include <cstdio>
#include <stdexcept>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static nfs::GameMemory memory;
extern "C" void *nfs_host_address(uint32_t address,size_t size){auto p=memory.resolve(address,size);if(!p)throw std::out_of_range("Axis span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t address=0;if(!memory.encode(p,address))throw std::out_of_range("Axis pointer");return address;}
int main(){
    using nfs::calibrated_axis;
    CHECK(calibrated_axis(0x684e,0x8000,false)==0);
    CHECK(calibrated_axis(0x684e,0,false)==255);
    CHECK(calibrated_axis(0x98b2,65535,false)==255);
    CHECK(calibrated_axis(0x98b2,0x8000,false)==0);
    CHECK(calibrated_axis(0x684e,0x7000,true)==0);
    CHECK(calibrated_axis(0x98b2,0x9000,true)==0);
    CHECK(calibrated_axis(0x14b6,0,false)==0);
    CHECK(calibrated_axis(0x14b6,65535,false)==255);
    CHECK(calibrated_axis(0xb614,65535,false)==0);
    CHECK(calibrated_axis(0xb614,0,false)==255);
    CHECK(calibrated_axis(0x1414,123,true)==0);
    CHECK(calibrated_axis(0x684e,INT32_MIN,false)==255);
    CHECK(calibrated_axis(0x98b2,INT32_MAX,false)==255);
    for(int axis=0;axis<=65535;++axis){
        const int left=axis<32768?(32768-axis)/102:0;
        const int right=axis>32768?(axis-32768)/103:0;
        CHECK(calibrated_axis(0x684e,axis,false)==(left>255?255:left));
        CHECK(calibrated_axis(0x98b2,axis,false)==(right>255?255:right));
        CHECK(calibrated_axis(0x14b6,axis,false)==calibrated_axis(0xb614,65535-axis,false));
    }
    uint8_t bytes[1024]{};CHECK(reinterpret_cast<uintptr_t>(bytes)>UINT32_MAX);
    CHECK(memory.map(0x80000000,bytes,sizeof(bytes)));
    guest_store<int32_t>(0x80000003+247*4,65535);
    CHECK(nfs::read_calibrated_axis(0x00f798b2,0x80000003,false)==255);
    CHECK(nfs::read_calibrated_axis(0x010098b2,0x80000003,false)==0);
    CHECK(nfs::read_calibrated_axis(0x98b2,0xfffffffe,false)==0);
    bool rejected=false;try{nfs::read_calibrated_axis(0x98b2,1,false);}catch(const std::out_of_range &){rejected=true;}CHECK(rejected);
    std::puts("PASS: original axis calibration endpoints, dead zones, reversed ranges, all 65536 values, bounded unaligned guest reads");
}
