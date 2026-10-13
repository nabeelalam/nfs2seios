#include "RuntimeServices.h"
#include "GuestAccess.h"
#include "GameMemory.h"
#include "PlatformError.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <thread>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static nfs::GameMemory memory;static unsigned initialized=0,notifications=0;static uint32_t exit_code=0;
extern "C" void *nfs_host_address(uint32_t a,size_t size){auto p=memory.resolve(a,size);if(!p)throw std::out_of_range("Runtime text span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t a=0;if(!memory.encode(p,a))throw std::out_of_range("Runtime text pointer");return a;}
static void initialize(){++initialized;}
static void exited(uint32_t code){++notifications;exit_code=code;uint32_t reported=0;if(!nfs::runtime_services().exit_status(reported)||reported!=code)throw std::runtime_error("Exit callback cannot inspect status");}
static int32_t message(const std::string &text,const std::string &caption,uint32_t flags){if(text!="Original asset missing" || caption!="NFS II SE" || flags!=4)throw std::runtime_error("Message marshalling");return 7;}
extern "C" int32_t WSAStartup_wrap(int32_t,int32_t);
extern "C" int32_t WSAGetLastError_wrap();
extern "C" int32_t recv_wrap(int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t GetCommState_wrap(int32_t,int32_t);
int main(){
    uint8_t bytes[512]{};CHECK(reinterpret_cast<uintptr_t>(bytes)>UINT32_MAX && memory.map(0x80000000,bytes,sizeof(bytes)));
    nfs::RuntimeServices services({initialize,exited,message});nfs::bind_runtime_services(&services);
    std::thread first([]{WrapperInit();}),second([]{WrapperInit();});first.join();second.join();CHECK(initialized==1 && WrapperInit()==0);
    std::strcpy(reinterpret_cast<char *>(bytes+3),"Original asset missing");std::strcpy(reinterpret_cast<char *>(bytes+100),"NFS II SE");
    CHECK(MessageBoxA_wrap(0,int32_t(0x80000003),int32_t(0x80000064),4)==7);
    bool stopped=false;try{ExitProcess_wrap(23);}catch(const nfs::GameExited &exit){stopped=exit.code==23;}CHECK(stopped && notifications==1 && exit_code==23);
    stopped=false;try{ExitProcess_wrap(42);}catch(const nfs::GameExited &exit){stopped=exit.code==23;}CHECK(stopped && notifications==1);
    bool rejected=false;try{WrapperInit();}catch(const std::logic_error &){rejected=true;}CHECK(rejected);
    CHECK(GetKeyboardType_wrap(0)==0 && GetModuleHandleA_wrap(0)==0 && SetThreadPriority_wrap(0,-15)==0 && nfs::platform_error()==50);
    CHECK(WSAStartup_wrap(0x101,int32_t(0x80000100))==10091 && WSAGetLastError_wrap()==10091 && bytes[256]==0);
    CHECK(recv_wrap(0,int32_t(0x80000100),20,0)==-1 && WSAGetLastError_wrap()==10093 && bytes[256]==0);
    CHECK(GetCommState_wrap(0,int32_t(0x80000100))==0 && nfs::platform_error()==50 && bytes[256]==0);
    nfs::bind_runtime_services(nullptr);std::puts("PASS: once-only app initialization, bounded guest error text, managed original exit/status notification and explicit unavailable Windows services");
}
