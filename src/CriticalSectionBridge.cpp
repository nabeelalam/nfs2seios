// SPDX-License-Identifier: MIT
#include "CriticalSectionBridge.h"
#include "GuestAccess.h"
#include "WorkerControl.h"
#include <stdexcept>
namespace nfs {
std::shared_ptr<CriticalSectionBridge::Section> CriticalSectionBridge::find(uint32_t address) {
    std::lock_guard<std::mutex> lock(registry_mutex);
    auto it=sections.find(address);
    if(it==sections.end()) throw std::out_of_range("Unknown game critical section");
    return it->second;
}
void CriticalSectionBridge::initialize(uint32_t address) {
    std::lock_guard<std::mutex> lock(registry_mutex);
    if(sections.count(address)) throw std::logic_error("Critical section already initialized");
    if(sections.size()==1024) throw std::out_of_range("Game critical section capacity");
    sections.emplace(address,std::make_shared<Section>());
}
void CriticalSectionBridge::enter(uint32_t address) {
    auto section=find(address);
    std::unique_lock<std::mutex> lock(section->mutex);
    const auto self=std::this_thread::get_id();
    ++section->waiters;
    section->changed.wait(lock,[&]{return section->deleted || worker_stop_ready() || !section->depth || section->owner==self;});
    --section->waiters;
    check_worker_stop();
    if(section->deleted) throw std::out_of_range("Deleted game critical section");
    if(section->depth==UINT32_MAX) throw std::overflow_error("Critical section recursion overflow");
    section->owner=self; ++section->depth;++owned_critical_depth();
}
void CriticalSectionBridge::leave(uint32_t address) {
    auto section=find(address);
    std::lock_guard<std::mutex> lock(section->mutex);
    if(section->deleted || !section->depth || section->owner!=std::this_thread::get_id())
        throw std::logic_error("Critical section leave by non-owner");
    --owned_critical_depth();
    if(!--section->depth) { section->owner={};section->changed.notify_one(); }
}
void CriticalSectionBridge::remove(uint32_t address) {
    std::lock_guard<std::mutex> registry_lock(registry_mutex);
    auto it=sections.find(address);
    if(it==sections.end()) throw std::out_of_range("Unknown game critical section");
    auto section=it->second;
    std::lock_guard<std::mutex> lock(section->mutex);
    if(section->depth || section->waiters) throw std::logic_error("Deleting an active game critical section");
    section->deleted=true; section->changed.notify_all(); sections.erase(it);
}
static CriticalSectionBridge *active=nullptr;
void CriticalSectionBridge::wake_workers(){
    std::lock_guard<std::mutex> guard(registry_mutex);
    for(auto &entry:sections){std::lock_guard<std::mutex> lock(entry.second->mutex);entry.second->changed.notify_all();}
}
static void wake(){if(active)active->wake_workers();}
void CriticalSectionBridge::retire_current_thread(){
    std::lock_guard<std::mutex> guard(registry_mutex);
    for(auto &entry:sections){auto &section=*entry.second;std::lock_guard<std::mutex> lock(section.mutex);
        if(section.depth && section.owner==std::this_thread::get_id()){section.depth=0;section.owner={};section.changed.notify_all();}}
    owned_critical_depth()=0;
}
static void retire(){if(active)active->retire_current_thread();}
void bind_critical_sections(CriticalSectionBridge *bridge) { active=bridge;critical_worker_wake()=bridge?wake:nullptr;critical_worker_retire()=bridge?retire:nullptr; }
static CriticalSectionBridge &current(uint32_t address) {
    nfs_host_address(address,24);
    if(!active) throw std::logic_error("Critical section runtime is unbound");
    return *active;
}
}
extern "C" int32_t InitializeCriticalSection_wrap(int32_t address) {
    nfs::current(uint32_t(address)).initialize(uint32_t(address));
    const struct { uint32_t words[6]; } opaque{};
    guest_store(uint32_t(address),opaque);
    return 0;
}
extern "C" int32_t EnterCriticalSection_wrap(int32_t address) {
    nfs::current(uint32_t(address)).enter(uint32_t(address));return 0;
}
extern "C" int32_t LeaveCriticalSection_wrap(int32_t address) {
    nfs::current(uint32_t(address)).leave(uint32_t(address));nfs::check_worker_stop();return 0;
}
extern "C" int32_t DeleteCriticalSection_wrap(int32_t address) {
    nfs::current(uint32_t(address)).remove(uint32_t(address));return 0;
}
