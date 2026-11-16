// SPDX-License-Identifier: MIT
#pragma once
#include "ChildInitialSidebarProof.h"
#include "SetupCaptionGameHooks.inc"
#include "OptionsCaptionGameHooks.inc"
#include "Main343OriginGameHooks.inc"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
namespace nfs_child_initial_sidebar_live {
namespace child=nfs_child_initial_sidebar;namespace bg=nfs_background_scene_cpu;
struct State {
 child::Store<child::SetupPolicy>setup;child::Store<child::OptionsPolicy>options;
 std::shared_ptr<const child::Proof>pending;std::shared_ptr<const bg::SourceCatalog>catalog;
 bg::Workspace workspace;bg::Plane actual;uintptr_t game=0,thread=0;uint32_t form=0,descriptor=0;uint64_t epoch=0,next=0;unsigned depth=0;bool invalid=false;
 void clear()noexcept{setup.clear();options.clear();pending.reset();catalog.reset();}
};
inline State*&slot(){struct Storage{State*p=nullptr;~Storage(){delete p;}};static thread_local Storage storage;return storage.p;}
inline State*current(Game&g){auto*s=slot();return s&&!s->invalid&&s->depth==1&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==nfs_logo_publisher::threadToken()?s:nullptr;}
struct OwnerScope {State*s=nullptr;bool entered=false;
 explicit OwnerScope(Game&g)noexcept{const uint32_t descriptor=uint32_t(g.eax);uint32_t form=descriptor==uint32_t(guest_address(off_4D2790))?child::SetupPolicy::form:descriptor==uint32_t(guest_address(off_4D3A88))?child::OptionsPolicy::form:0;
  if(!form||!nfs_logo_publisher::enabled())return;auto*&p=slot();if(!p)p=new(std::nothrow)State;if(!p)return;s=p;entered=true;if(s->depth++){s->invalid=true;s->clear();return;}s->clear();s->invalid=false;if(s->next==UINT64_MAX){s->invalid=true;return;}s->epoch=++s->next;s->form=form;s->descriptor=descriptor;s->game=reinterpret_cast<uintptr_t>(&g);s->thread=nfs_logo_publisher::threadToken();s->catalog=nfs_background_scene_runtime::acquireCatalog();
 }
 ~OwnerScope(){if(!entered)return;if(s->depth)--s->depth;if(!s->depth){s->clear();s->game=s->thread=0;s->form=s->descriptor=0;}}
};
// Default-off source diagnostics; scalar observations only, no guest writes.
struct CaptionDiagnostic{unsigned state=0,failure=0,packet=0,proof=0,resource=0,route=0,initialDepth=0;};
template<class T>inline CaptionDiagnostic captionDiagnostic(const T*c)noexcept{CaptionDiagnostic d;if(c){d.state=1;d.failure=unsigned(c->failure);d.packet=bool(c->packet);d.proof=bool(c->proof);d.resource=c->resourceComplete;d.route=unsigned(c->evidence.route);d.initialDepth=c->initialDepth;}return d;}
inline void trace(const char*stage,const char*reason,const State*s)noexcept{
 struct ErrnoRestore{int value=errno;~ErrnoRestore(){errno=value;}} preserve;
 static thread_local unsigned lines=0;const char*enabled=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(!enabled||enabled[0]!='1'||enabled[1]||lines>=32)return;++lines;
 const auto c=s?(s->form==child::SetupPolicy::form?captionDiagnostic(nfs_setup_caption::state()):captionDiagnostic(nfs_options_caption::state())):CaptionDiagnostic{};
 std::fprintf(stderr,"CHILD INITIAL stage=%s reason=%s state=%u form=%x depth=%u invalid=%u epoch=%llu pending=%u captionState=%u captionFailure=%u packet=%u proof=%u resource=%u route=%u initialDepth=%u\n",stage,reason,unsigned(s!=nullptr),s?s->form:0,s?s->depth:0,unsigned(s&&s->invalid),(unsigned long long)(s?s->epoch:0),unsigned(s&&bool(s->pending)),c.state,c.failure,c.packet,c.proof,c.resource,c.route,c.initialDepth);
}
inline void completeInitialCaption(Game&g)noexcept{
 auto*s=current(g);trace("complete-entry",s?"owner-current":"owner-missing",s);if(!s||!s->catalog||!s->epoch||uint32_t(g.esi)!=s->descriptor)return;s->pending.reset();s->setup.clear();s->options.clear();
 auto caption=s->form==child::SetupPolicy::form?nfs_setup_caption::acquireCurrentProof(g):nfs_options_caption::acquireCurrentProof(g);
 if(!caption||!caption->current()||caption->evidence().route!=nfs_compact_caption::Route::InitialMain||!caption->origin()){trace("complete","caption-proof",s);return;}
 s->actual.valid=0;uint32_t error=0;nfs_logo_publisher::plane(s->actual,error);if(error||!bg::plane(s->actual,343))return;
 child::Binding b;b.form=s->form;b.formGuest=s->descriptor;b.archive=caption->origin()->archive();b.game=s->game;b.thread=s->thread;b.epoch=s->epoch;b.captionEvent={s->epoch,1,0};
 if(s->form==child::SetupPolicy::form)s->pending=s->setup.accept(caption->origin(),caption,b,s->actual,s->workspace,s->catalog);
 else s->pending=s->options.accept(caption->origin(),caption,b,s->actual,s->workspace,s->catalog);
 trace("complete",s->pending?"sealed":"source-join",s);
}
inline std::shared_ptr<const child::Proof>acquire(Game&g,uint32_t source,const std::vector<uint8_t>&raw)noexcept{
 auto*s=current(g);if(!s||!s->pending||!s->pending->current()||!s->catalog){if(s)trace("acquire","pending-missing",s);return {};}const auto p=s->pending;
 if(raw.size()!=16+bg::Side*2||p->binding().form!=s->form||p->binding().formGuest!=s->descriptor||p->binding().epoch!=s->epoch||source!=p->post().target.w[8])return {};
 try{nfs::GuestCopyTransaction tx;nfs_logo_capture::Target target{};nfs_logo_publisher::target(tx,target,p->post().target.owner);
  if(!nfs_main_sidebar_origin::sameRetainedTarget(target,p->post().target)||tx.load<uint32_t>(dword_4D40B8)!=p->binding().archive)return {};
  for(unsigned k=0;k<4;++k)if((uint32_t(raw[4*k])|(uint32_t(raw[4*k+1])<<8)|(uint32_t(raw[4*k+2])<<16)|(uint32_t(raw[4*k+3])<<24))!=target.header[k])return {};
  for(unsigned k=0;k<bg::Side;++k)if(bg::le16(raw.data()+16+2*k)!=p->post().pixels[k])return {};
  return p->current()?p:std::shared_ptr<const child::Proof>{};
 }catch(...){return {};}
}
} // namespace nfs_child_initial_sidebar_live
