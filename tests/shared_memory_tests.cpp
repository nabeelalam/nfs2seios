#include "GuestAccess.h"
#include "GuestCopyTransaction.h"
#include "GameMemory.h"
#include <thread>
#include <atomic>
#include <cstdio>
#include <stdexcept>
#include <chrono>
static nfs::GameMemory memory;
extern "C" void *nfs_host_address(uint32_t address,size_t size){void *p=memory.resolve(address,size);if(!p)throw std::out_of_range("Bad shared span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t id=0;if(!memory.encode(p,id))throw std::out_of_range("Bad shared pointer");return id;}
int main(){
    uint8_t bytes[32]{};if(!memory.map(0x80000000,bytes,sizeof(bytes)))return 1;
    guest_store<uint64_t>(0x80000003u,UINT32_MAX);
    std::atomic<bool> done{false},valid{true};
    std::thread reader([&]{do{const uint64_t value=guest_load<uint64_t>(0x80000003u);if(uint32_t(value)!=~uint32_t(value>>32))valid=false;}while(!done);});
    for(uint32_t i=0;i<50000;++i)guest_store<uint64_t>(0x80000003u,(uint64_t(i)<<32)|~i);
    done=true;reader.join();if(!valid)return 1;
    {
        nfs::GuestCopyTransaction transaction;
        transaction.store<uint64_t>(0x80000003u,0);
        transaction.store<uint64_t>(0x80000013u,~uint64_t(0));
    }
    done=false;
    std::atomic<uint32_t> pair_reads{0};
    std::thread pair_reader([&]{do{
        nfs::GuestCopyTransaction transaction;
        const auto first=transaction.load<uint64_t>(0x80000003u);
        const auto second=transaction.load<uint64_t>(0x80000013u);
        if(first!=~second)valid=false;
        ++pair_reads;
    }while(!done);});
    for(uint64_t i=1;i<=50000;++i){
        {
            nfs::GuestCopyTransaction transaction;
            transaction.store<uint64_t>(0x80000003u,i);
            transaction.store<uint64_t>(0x80000013u,~i);
        }
        if(i==25000){
            const auto reads=pair_reads.load();
            const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
            while(pair_reads.load()==reads && std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
            if(pair_reads.load()==reads){done=true;pair_reader.join();return 1;}
        }
    }
    done=true;pair_reader.join();if(!valid)return 1;
    bool invalid=false;
    try{nfs::GuestCopyTransaction transaction;transaction.load<uint64_t>(0x8000001fu);}
    catch(const std::out_of_range &){invalid=true;}
    if(!invalid)return 1;
    nfs::WorkerControl control;auto *previous=nfs::current_worker_control();
    nfs::current_worker_control()=&control;bool stopped=false;
    try{nfs::GuestCopyTransaction transaction;control.stop=true;transaction.load<uint64_t>(0x80000003u);}
    catch(const nfs::WorkerStopped &){stopped=true;}
    nfs::current_worker_control()=previous;if(!stopped)return 1;
    bool released=false;
    std::thread check_release([&]{released=guest_copy_mutex().try_lock();if(released)guest_copy_mutex().unlock();});
    check_release.join();if(!released)return 1;
    std::puts("PASS: concurrent unaligned values, 50000 paired transactions, span failure and cancellation release");
}
