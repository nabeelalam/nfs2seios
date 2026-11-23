// SPDX-License-Identifier: MIT
#pragma once
#include "SetupSceneLiveGameHooks.inc"
#include "OptionsSceneLiveGameHooks.inc"
#include "ChildInitialSidebarGameHooks.inc"
#include "SetupInitialFullProof.h"
namespace nfs_setup_initial_full_live {
namespace bg=nfs_background_scene_cpu;
struct ErrnoRestore{int value=errno;~ErrnoRestore(){errno=value;}};
inline bool readPlane(bg::Plane&p)noexcept{uint32_t error=0;p.valid=0;nfs_logo_publisher::plane(p,error);return !error&&p.valid;}
inline bool readBytes(uint32_t at,size_t n,std::vector<uint8_t>&out)noexcept{try{out.resize(n);{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(at,n);std::memcpy(out.data(),nfs_host_address(at,n),n);}return true;}catch(...){return false;}}
struct Proof {std::shared_ptr<const nfs_options_sidebar_origin::Origin>optionsReturn;bg::Plane returnedSide;uint32_t previousForm=0;std::shared_ptr<const nfs_child_initial_sidebar::Proof>child;bg::Plane full;uintptr_t game=0,thread=0;uint64_t epoch=0,baseSequence=0;uint32_t archive=0,mapRole=0,form=0;};
struct State{std::shared_ptr<const Proof>proof;std::shared_ptr<Proof>pending;bg::Plane before;std::array<uint32_t,9>table{};std::vector<uint8_t>raw;unsigned depth=0,rows=0;uint32_t key=0;bool failed=false;const char*reason="not-started";uint32_t form=0;};
inline State*&slot()noexcept{struct Storage{State*p=nullptr;~Storage(){delete p;}};static thread_local Storage s;return s.p;}
// Default-off first-failure scalar observations; never read guest memory for tracing.
inline void trace(const char*stage,const char*reason,uint32_t form,const Game*g,const State*s,unsigned a=0,unsigned b=0,unsigned c=0,unsigned d=0)noexcept{ErrnoRestore preserve;const char*on=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(!on||on[0]!='1'||on[1])return;
 struct Key{const char*stage;const char*reason;uint32_t form;unsigned count;};static thread_local Key keys[64];static thread_local unsigned used=0,total=0;if(total>=128)return;unsigned k=0;for(;k<used;++k)if(!std::strcmp(keys[k].stage,stage)&&!std::strcmp(keys[k].reason,reason)&&keys[k].form==form)break;if(k==used){if(used==64)return;keys[used++]={stage,reason,form,0};}if(keys[k].count++>=2)return;++total;
 std::fprintf(stderr,"INITIAL FULL stage=%s reason=%s form=%x depth=%u rows=%u failed=%u pending=%u proof=%u eax=%x edx=%x ebx=%x ecx=%x esi=%x a=%u b=%u c=%u d=%u\n",stage,reason,form,s?s->depth:0,s?s->rows:0,unsigned(s&&s->failed),unsigned(s&&bool(s->pending)),unsigned(s&&bool(s->proof)),g?uint32_t(g->eax):0,g?uint32_t(g->edx):0,g?uint32_t(g->ebx):0,g?uint32_t(g->ecx):0,g?uint32_t(g->esi):0,a,b,c,d);
}
// Separate fixed policies; neither collector is cast to the other form.
struct SetupPolicy{static constexpr uint32_t form=0x4d2790u;static uint32_t descriptor(){return uint32_t(guest_address(off_4D2790));}static nfs_setup_scene_live::State*current(Game&g){return nfs_setup_scene_live::current(g);}static bool archive(nfs_setup_scene_live::State&s){return nfs_setup_scene_live::archiveCurrent(s);}};
struct OptionsPolicy{static constexpr uint32_t form=0x4d3a88u;static uint32_t descriptor(){return uint32_t(guest_address(off_4D3A88));}static nfs_options_scene_live::State*current(Game&g){return nfs_options_scene_live::current(g);}static bool archive(nfs_options_scene_live::State&s){return nfs_options_scene_live::archiveCurrent(s);}};
inline const bg::Plane&sidebar(const Proof&p)noexcept{return p.child?p.child->post():p.returnedSide;}
inline bool nestedOptionsReturn(Game&g,uint32_t&previous)noexcept{try{nfs::GuestCopyTransaction tx;previous=tx.load<uint32_t>(dword_4D243C);return previous==uint32_t(guest_address(off_4D3348))||previous==uint32_t(guest_address(off_4D37C8))||previous==uint32_t(guest_address(off_4D38D8))||previous==uint32_t(guest_address(off_4D2C80))||previous==uint32_t(guest_address(off_4D39A8));}catch(...){return false;}}
inline bool returnSidebarMatches(Game&g,const Proof&p,const std::vector<uint8_t>&raw)noexcept{
 if(!p.optionsReturn||!p.optionsReturn->matches(p.game,p.thread,p.archive,raw.data(),raw.size()))return false;
 try{nfs::GuestCopyTransaction tx;bg::Target actual{};nfs_logo_publisher::target(tx,actual,p.returnedSide.target.owner);return nfs_main_sidebar_origin::sameRetainedTarget(actual,p.returnedSide.target)&&actual.w[8]==p.returnedSide.target.w[8];}catch(...){return false;}
}
template<class Policy>inline bool ownedFor(Game&g,const Proof&p)noexcept{
 auto*s=Policy::current(g);if(p.form!=Policy::form||!s||s->invalid||s->iteration||!s->baseVerified||s->ownerEpoch!=p.epoch||s->game!=p.game||s->thread!=p.thread||s->archive!=p.archive||s->base.event.ownerEpoch!=p.epoch||s->base.event.iteration||s->base.event.sequence!=p.baseSequence||s->base.mapRole!=p.mapRole||!Policy::archive(*s))return false;
 if(p.optionsReturn){uint32_t previous=0;return Policy::form==OptionsPolicy::form&&!p.child&&p.optionsReturn->current()&&p.optionsReturn->catalog==s->catalogue&&p.optionsReturn->game==p.game&&p.optionsReturn->thread==p.thread&&p.optionsReturn->archive==p.archive&&bg::plane(p.returnedSide,343)&&nestedOptionsReturn(g,previous)&&previous==p.previousForm;}
 auto*c=nfs_child_initial_sidebar_live::current(g);return c&&c->form==Policy::form&&c->descriptor==Policy::descriptor()&&c->pending==p.child&&p.child&&p.child->current()&&p.child->catalogMatches(s->catalogue)&&p.child->binding().form==Policy::form&&p.child->binding().game==p.game&&p.child->binding().thread==p.thread&&p.child->binding().archive==p.archive;
}
template<class Policy>inline unsigned ownerFlags(Game&g)noexcept{auto*s=Policy::current(g);return s?(1u|(unsigned(s->invalid)<<1)|(unsigned(s->iteration!=0)<<2)|(unsigned(s->baseVerified)<<3)):0;}
inline bool rawMatches(const Proof&p,const std::vector<uint8_t>&raw)noexcept{if(raw.size()!=16+bg::Full*2)return false;for(unsigned k=0;k<4;++k)if(bg::le32(raw.data()+4*k)!=p.full.target.header[k])return false;for(unsigned k=0;k<bg::Full;++k)if(bg::le16(raw.data()+16+2*k)!=p.full.pixels[k])return false;return true;}
inline bool owned(Game&g,const Proof&p)noexcept{return p.form==SetupPolicy::form?ownedFor<SetupPolicy>(g,p):p.form==OptionsPolicy::form?ownedFor<OptionsPolicy>(g,p):false;}
// Installed only around40C160 initial/refresh405B60 calls; each occurrence
// creates a new proof from the current verified base and exact retained343.
struct CopyScope{Game&g;State*s=nullptr;bool entered=false;int unwind=0;
 template<class Policy>void start()noexcept{
 auto*live=Policy::current(g);auto*child=nfs_child_initial_sidebar_live::current(g);
 if(!live||live->invalid||live->iteration||!live->baseVerified||uint32_t(g.esi)!=Policy::descriptor()||((!child||child->form!=Policy::form||!child->pending)&&Policy::form!=OptionsPolicy::form)){trace("copy-entry","owner-base-child",Policy::form,&g,slot(),unsigned(live!=nullptr),unsigned(live&&live->invalid),live?unsigned(live->iteration):0,unsigned(live&&live->baseVerified));return;}
 auto*&storage=slot();if(!storage)storage=new(std::nothrow)State;if(!storage)return;s=storage;s->form=Policy::form;s->reason="nested";s->proof.reset();if(s->depth++){s->pending.reset();s->failed=true;entered=true;return;}entered=true;unwind=nfs_logo_publisher::exceptions();s->pending.reset();s->rows=0;s->failed=false;
 try{s->reason="allocation";auto p=std::make_shared<Proof>();p->child=child&&child->form==Policy::form?child->pending:nullptr;p->form=Policy::form;p->game=live->game;p->thread=live->thread;p->epoch=live->ownerEpoch;p->archive=live->archive;p->baseSequence=live->base.event.sequence;p->mapRole=live->base.mapRole;
 if(!p->child&&Policy::form==OptionsPolicy::form){
 s->reason="return-previous-form";if(!nestedOptionsReturn(g,p->previousForm))return;s->reason="return-sidebar-read";if(!readBytes(uint32_t(g.eax),16+bg::Side*2,s->raw))return;s->reason="return-sidebar-origin";
 p->optionsReturn=nfs_options_sidebar_origin::acquire(p->game,p->thread,p->archive,s->raw.data(),s->raw.size());if(!p->optionsReturn||p->optionsReturn->catalog!=live->catalogue){auto prior=nfs_options_sidebar_origin::slot();unsigned flags=0;if(prior){flags=1u|(unsigned(prior->current())<<1)|(unsigned(prior->game==p->game)<<2)|(unsigned(prior->thread==p->thread)<<3)|(unsigned(prior->archive==p->archive)<<4)|(unsigned(prior->catalog==live->catalogue)<<5)|(unsigned(prior->raw==s->raw)<<6);}trace("options-return","origin-mismatch",Policy::form,&g,s,flags,prior?prior->archive:0,p->archive,p->previousForm);return;}s->reason="return-sidebar-target";
 {nfs::GuestCopyTransaction tx;for(uint32_t at:{tx.load<uint32_t>(dword_4D231C),tx.load<uint32_t>(dword_4D2320)})if(at&&tx.load<uint32_t>(at+0x20)==uint32_t(g.eax)){nfs_logo_publisher::target(tx,p->returnedSide.target,at);break;}}
 if(!p->returnedSide.target.owner||p->returnedSide.target.w[8]!=uint32_t(g.eax))return;p->returnedSide.width=343;p->returnedSide.height=480;p->returnedSide.valid=1;
 for(unsigned k=0;k<bg::Side;++k)p->returnedSide.pixels[k]=bg::le16(s->raw.data()+16+2*k);
 if(!bg::plane(p->returnedSide,343)||!returnSidebarMatches(g,*p,s->raw))return;
 }
 s->reason="owner-source-args";if(!ownedFor<Policy>(g,*p)||uint32_t(g.eax)!=sidebar(*p).target.w[8]||uint32_t(g.edx)!=0x2800||g.ebx||g.ecx||p->mapRole>1)return;
 s->reason="initial-sidebar-current";if(!readBytes(uint32_t(g.eax),16+bg::Side*2,s->raw)||(p->child?nfs_child_initial_sidebar_live::acquire(g,uint32_t(g.eax),s->raw)!=p->child:!returnSidebarMatches(g,*p,s->raw)))return;
 s->reason="source-table-overlap";{nfs::GuestCopyTransaction tx;nfs_logo_publisher::table(tx,s->table,true);if(sidebar(*p).target.header[3]||nfs_small_logo_live::overlap(uint32_t(g.eax),s->raw.size(),live->base.post.target.w[8],16+bg::Full*2))return;}
 s->reason="base-before";if(!readPlane(s->before)||!bg::plane(s->before,640)||!bg::retainedBaseTarget(live->base.post.target,s->before.target)||s->before.pixels!=live->base.post.pixels)return;s->pending=std::move(p);s->reason="admitted";trace("copy-entry",s->reason,s->form,&g,s);
 }catch(...){s->pending.reset();}}
 explicit CopyScope(Game&game)noexcept:g(game){ErrnoRestore preserve;if(uint32_t(g.esi)==SetupPolicy::descriptor())start<SetupPolicy>();else if(uint32_t(g.esi)==OptionsPolicy::descriptor())start<OptionsPolicy>();}
 ~CopyScope(){ErrnoRestore preserve;if(!entered)return;if(s->depth==1){auto p=std::move(s->pending);std::array<uint32_t,9>after{};bool sourceOkay=false;try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::table(tx,after,true);}catch(...){s->reason="post-table-read";s->failed=true;}
 std::vector<uint8_t>raw;if(p&&readBytes(sidebar(*p).target.w[8],16+bg::Side*2,raw))sourceOkay=raw==s->raw&&(p->child?nfs_child_initial_sidebar_live::acquire(g,sidebar(*p).target.w[8],raw)==p->child:returnSidebarMatches(g,*p,raw));
 const char*reason=nullptr;if(!p)reason=s->reason;else if(s->failed)reason=s->reason;else if(s->rows!=480)reason="row-count";else if(!sourceOkay)reason="source-after";else if(!nfs_small_logo_live::tables(s->table,after,true,0,0x7d))reason="table-after";else if(nfs_logo_publisher::exceptions()!=unwind)reason="unwind";else if(!owned(g,*p))reason="owner-after";else if(!readPlane(p->full))reason="post-plane";else if(!nfs_setup_initial_full::copied(s->before,sidebar(*p),p->full,s->key))reason="post-replay";else if(!owned(g,*p))reason="owner-recheck";
 if(!reason)s->proof=std::move(p);else s->proof.reset();trace("copy-exit",reason?reason:"sealed",s->form,&g,s,unsigned(sourceOkay),s->key);}if(s->depth)--s->depth;}
};
// Observe every actual483070 converted row, including converter and key.
inline void row(Game&g)noexcept{ErrnoRestore preserve;auto*s=slot();if(!s||s->depth!=1||!s->pending||s->failed)return;auto p=s->pending;
 if(!owned(g,*p)||s->rows>=480||uint32_t(g.ebx)!=343||uint32_t(g.edx)!=uint64_t(s->before.target.w[8])+s->before.target.y[s->rows]+s->before.target.x[0]){s->reason="row-owner-count-destination";s->failed=true;trace("row",s->reason,s->form,&g,s);return;}
 try{s->reason="row-converter-key-payload-target";nfs::GuestCopyTransaction tx;const uint32_t source=uint32_t(g.eax),key=tx.load<uint32_t>(dword_4DD03C);s->reason="row-converter";if(tx.load<uint32_t>(dword_571264)!=0xf0001f44u)throw 1;s->reason="row-callback";if(tx.load<uint32_t>(dword_571260)!=uint32_t(game_id_sub_483070))throw 1;s->reason="row-key";if(s->rows&&key!=s->key)throw 1;s->reason="row-overlap";if(nfs_small_logo_live::overlap(source,343*4,s->before.target.w[8],16+bg::Full*2)||nfs_small_logo_live::overlap(source,343*4,sidebar(*p).target.w[8],16+bg::Side*2))throw 1;
 s->reason="row-payload";nfs_logo_publisher::checked(source,343*4);for(unsigned x=0;x<343;++x)if(tx.load<uint32_t>(source+4*x)!=nfs_small_logo_live::expanded(sidebar(*p).pixels[s->rows*343+x],key))throw 1;
 s->reason="row-target";bg::Target now{};nfs_logo_publisher::target(tx,now);if(!nfs_logo_capture::same(now,s->before.target))throw 1;s->key=key;++s->rows;s->reason="row-accepted";
 }catch(...){s->failed=true;trace("row",s->reason,s->form,&g,s);}}
inline std::shared_ptr<const Proof>acquire(Game&g,uint32_t source,const std::vector<uint8_t>&raw)noexcept{ErrnoRestore preserve;
 auto*s=slot();if(!s||s->depth||!s->proof){trace("acquire","proof-missing",s?s->form:0,&g,s);return {};}auto p=s->proof;if(!owned(g,*p)||source!=p->full.target.w[8]||raw.size()!=16+bg::Full*2){const char*on=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(on&&on[0]=='1'&&!on[1])trace("acquire","owner-source-span",s->form,&g,s,unsigned(rawMatches(*p,raw)),p->form==SetupPolicy::form?ownerFlags<SetupPolicy>(g):ownerFlags<OptionsPolicy>(g),source,p->full.target.w[8]);return {};}
 for(unsigned k=0;k<4;++k)if(bg::le32(raw.data()+4*k)!=p->full.target.header[k])return {};
 for(unsigned k=0;k<bg::Full;++k)if(bg::le16(raw.data()+16+2*k)!=p->full.pixels[k])return {};
 const bool okay=owned(g,*p);trace("acquire",okay?"accepted":"owner-recheck",s->form,&g,s);return okay?p:std::shared_ptr<const Proof>{};
}
}
