// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
#include "Main343OriginLifetime.h"
#include "MainUploadedSourceAbi.h"
#include <atomic>
#include <memory>
namespace nfs_main_uploaded_source {
struct OwnerLife {std::atomic<bool>alive{true};void revoke()noexcept{alive.store(false,std::memory_order_release);}bool current()const noexcept{return alive.load(std::memory_order_acquire);}};
inline std::shared_ptr<OwnerLife>newLife()noexcept{try{return std::make_shared<OwnerLife>();}catch(...){return {};}}
struct Owner final {
 std::atomic<unsigned>refs{1},nativeStatus{0};std::shared_ptr<const nfs_background_scene_cpu::Prepared>frame;
 std::shared_ptr<OwnerLife>phase;std::shared_ptr<const nfs_main_sidebar_origin::GameLife>gameLife;
 std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>catalog;
 NfsMainUploadedDomain domain{};std::atomic<uint64_t>*ledger=nullptr;
 std::array<uint32_t,300>descriptors{};std::array<std::array<uint32_t,7>,300>profiles{};
 NfsMainUploadedSeal seal{};uint64_t completedIteration=0;std::atomic<bool> completion{false};bool sealed=false;
 static void retain(void*p){if(p)static_cast<Owner*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){if(p&&static_cast<Owner*>(p)->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete static_cast<Owner*>(p);}
 static int current(void*p){auto*s=static_cast<Owner*>(p);return s&&s->frame&&s->frame->valid&&s->phase&&s->phase->current()&&s->gameLife&&s->gameLife->current()&&s->ledger&&s->domain.uploadEvent&&s->ledger->load(std::memory_order_acquire)==s->domain.uploadEvent;}
 static void status(void*p,unsigned value){if(p)static_cast<Owner*>(p)->nativeStatus.store(value,std::memory_order_release);}
 NfsMainUploadedView view(uint64_t serial){NfsMainUploadedView v{};v.schema=1;v.domain=domain;v.frame=frame->view();v.frame.serial=serial;v.owner=this;v.retain=retain;v.release=release;v.current=current;v.status=status;return v;}
};
inline std::shared_ptr<Owner>adopt(Owner*p){return std::shared_ptr<Owner>(p,[](Owner*q){Owner::release(q);});}
} // namespace
