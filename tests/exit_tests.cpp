// SPDX-License-Identifier: MIT
#include "ExitCallbacks.h"
#include "GameContexts.h"
#include "GuestAccess.h"
#include <vector>
#include <cstdio>
#include <stdexcept>
static nfs::ExitCallbacks *registry;
static void first(void *p){static_cast<std::vector<int> *>(p)->push_back(1);if(!registry->add(3))throw std::runtime_error("Nested exit registration");}
static void second(void *p){static_cast<std::vector<int> *>(p)->push_back(2);throw std::runtime_error("Cleanup fixture");}
static void third(void *p){nfs::check_worker_stop();static_cast<std::vector<int> *>(p)->push_back(3);}
static void stop_callback(void *p){static_cast<std::vector<int> *>(p)->push_back(5);nfs::current_worker_control()->stop=true;}
static void fourth(void *p){static_cast<std::vector<int> *>(p)->push_back(4);}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t id){switch(id){case 1:return first;case 2:return second;case 3:return third;case 4:return fourth;case 5:return stop_callback;default:throw std::out_of_range("Exit function ID");}}
extern "C" void nfs_dispatch_game_exit(void *context,uint32_t id){nfs_lookup_guest_function(id)(context);}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(){
    nfs::ExitCallbacks callbacks;registry=&callbacks;nfs::bind_exit_callbacks(&callbacks);
    CHECK(!WrapperAtExit(4) && !WrapperAtExit(1) && !WrapperAtExit(2));
    bool invalid=false;try{WrapperAtExit(99);}catch(const std::out_of_range &){invalid=true;}CHECK(invalid);
    std::vector<int> order;nfs::WorkerControl control;control.stop=true;nfs::current_worker_control()=&control;
    bool reported=false;try{callbacks.drain(&order);}catch(const std::runtime_error &){reported=true;}
    CHECK(reported && order==std::vector<int>({2,1,3,4}) && nfs::worker_stop_ready());
    nfs::current_worker_control()=nullptr;callbacks.drain(&order);CHECK(order.size()==4 && WrapperAtExit(4)==-1);
    nfs::ExitCallbacks capacity;for(int i=0;i<512;++i)CHECK(capacity.add(4));CHECK(!capacity.add(4));capacity.drain(&order);CHECK(order.size()==516);
    nfs::bind_exit_callbacks(nullptr);CHECK(WrapperAtExit(4)==-1);

    nfs::WorkerControl optional;optional.stop=false;nfs::current_worker_control()=&optional;
    {
        nfs::ExitCallbacks c;registry=&c;std::vector<int> observed;
        CHECK(c.add(4) && c.add(1));c.drain_cancellable(&observed);
        CHECK(observed==std::vector<int>({1,3,4}));c.drain(&observed);CHECK(observed.size()==3 && !c.add(4));
    }
    {
        nfs::ExitCallbacks c;std::vector<int> observed;CHECK(c.add(4) && c.add(2));
        bool threw=false;try{c.drain_cancellable(&observed);}catch(const std::runtime_error &){threw=true;}
        CHECK(threw && observed==std::vector<int>({2}) && !c.add(4));c.drain(&observed);CHECK(observed.size()==1);
    }
    {
        nfs::ExitCallbacks c;std::vector<int> observed;CHECK(c.add(4));optional.stop=true;
        bool stopped=false;try{c.drain_cancellable(&observed);}catch(const nfs::WorkerStopped &){stopped=true;}
        CHECK(stopped && observed.empty() && !c.add(4));optional.stop=false;c.drain(&observed);CHECK(observed.empty());
    }
    {
        nfs::ExitCallbacks c;std::vector<int> observed;CHECK(c.add(4) && c.add(5));optional.stop=false;
        bool stopped=false;try{c.drain_cancellable(&observed);}catch(const nfs::WorkerStopped &){stopped=true;}
        CHECK(stopped && observed==std::vector<int>({5}) && !c.add(4));optional.stop=false;c.drain(&observed);CHECK(observed.size()==1);
    }
    nfs::current_worker_control()=nullptr;
    std::puts("PASS cancellable exit callbacks: healthy nested/LIFO,first-error closes,stop before/between callbacks,closed idempotence;legacy policy retained");
    std::puts("PASS: exit IDs, LIFO/new registrations, callback error continues cleanup, stop deferral/restoration, capacity and idempotent closed drain");
}
