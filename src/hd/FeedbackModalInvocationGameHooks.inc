// SPDX-License-Identifier: MIT
#pragma once
// One real unavailable-Feedback lexical invocation shared by independent
// backdrop/text collectors. No public constructor, numeric factory or GPU right.
#include "ControlsTextGameHooks.inc"
#include "FeedbackModalBackdropReplay.h"
#include <atomic>
namespace nfs_feedback_modal_live {
namespace bd=nfs_feedback_backdrop;
inline bool span(uint32_t p,size_t n)noexcept{return p&&n&&uint64_t(p)+n<=UINT64_C(0x100000000);}
inline bool read(uint32_t p,void*out,size_t n)noexcept{try{if(!out||!span(p,n))return false;nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(p,n);std::memcpy(out,nfs_host_address(p,n),n);return true;}catch(...){return false;}}
inline std::array<unsigned char,32>hex(const char*p)noexcept{std::array<unsigned char,32>v{};auto n=[](char c){return c>='0'&&c<='9'?unsigned(c-'0'):c>='a'&&c<='f'?unsigned(c-'a'+10):256u;};for(unsigned i=0;i<32;++i){unsigned a=n(p[2*i]),b=n(p[2*i+1]);if(a>15||b>15)return {};v[i]=static_cast<unsigned char>(a*16+b);}return v;}
inline std::array<bd::SourceIdentity,2>expectedSources()noexcept{
 std::array<bd::SourceIdentity,2>v{};
 // Authenticated game-resource identities, not credentials.
 const uint64_t kBanner=0x3e2cd1c484d81c59ULL,kEmblem=0x39106b41a9a949dbULL;
 {auto&i=v[0];i.providerKey=kBanner;i.tag=0x72616264;i.span=17224;i.width=344;i.height=47;i.paletteOffset=16184;i.paletteCount=256;i.archiveOffset=9832;i.hotX=0;i.hotY=0;i.header={{0x3f387bu,0x2f0158u,0x0u,0x6d00c1u}};i.paletteHeader={{0x2au,0x10100u,0x100u,0xf00000u}};i.sha=hex("591cd884c4d12c3e9d81ba1730ea645e78a979e9954d6c6da0b511f33adced26");i.archiveSha=hex("b1ffe450ed6822bd748cbfc73dfacd3fa5b3e6fa2e7e374b9b64e2ff73bfb918");}
 {auto&i=v[1];i.providerKey=kEmblem;i.tag=0x676f6c64;i.span=8196;i.width=117;i.height=61;i.paletteOffset=7156;i.paletteCount=256;i.archiveOffset=30096;i.hotX=-112;i.hotY=-103;i.header={{0x1bf47bu,0x3d0075u,0xff99ff90u,0x670070u}};i.paletteHeader={{0x2au,0x10100u,0x100u,0xf00000u}};i.sha=hex("db49a9a9416b10397e478db2a62f8b3919aa95f06a6291c8c4083fac875191ec");i.archiveSha=hex("b1ffe450ed6822bd748cbfc73dfacd3fa5b3e6fa2e7e374b9b64e2ff73bfb918");}
 return v;
}
inline bool mainTarget(const bd::Target&t,uint32_t owner)noexcept{
 const auto&w=t.w;if(!owner||t.owner!=owner||t.ownerState[3]!=owner||w[1]!=640||w[2]!=480||w[3]||w[4]||w[5]!=640||w[6]!=480||(w[7]&255)!=16||w[10]!=1280||w[13]||!span(w[8],614416)||t.header!=std::array<uint32_t,4>{{0x78,640u|(480u<<16),0,0}})return false;
 for(unsigned k=0;k<14;++k)if(t.ownerWords[k]!=w[k])return false;for(unsigned x=0;x<640;++x)if(t.x[x]!=2*x)return false;for(unsigned y=0;y<480;++y)if(t.y[y]!=16+1280*y)return false;return true;
}
inline bool explicitMain(bd::Target&t,bd::Plane&pixels)noexcept{try{
 nfs::GuestCopyTransaction tx;const uint32_t owner=tx.load<uint32_t>(dword_4D4D24);if(!span(owner,56))return false;nfs_logo_publisher::target(tx,t,owner);if(!mainTarget(t,owner))return false;pixels.resize(bd::Pixels);nfs_logo_publisher::checked(t.w[8]+16,bd::Pixels*2);std::memcpy(pixels.data(),nfs_host_address(t.w[8]+16,bd::Pixels*2),bd::Pixels*2);bd::Target after{};nfs_logo_publisher::target(tx,after,owner);return tx.load<uint32_t>(dword_4D4D24)==owner&&nfs_logo_capture::same(t,after);
 }catch(...){return false;}}
struct Resources {nfs_controls_dialog_word::Authority authority{};uint32_t array=0,table=0,archive=0;std::array<std::array<uint8_t,12>,2>headers{};std::array<std::array<uint8_t,96>,2>texts{};std::array<uint32_t,2>pointers{};
 bool operator==(const Resources&o)const noexcept{return authority==o.authority&&array==o.array&&table==o.table&&archive==o.archive&&headers==o.headers&&texts==o.texts&&pointers==o.pointers;}};
inline bool resources(uint32_t array,Resources&r)noexcept{try{
 if(!span(array,8)||!nfs_controls_text::readDialogAuthority(r.authority)||r.authority.selectedIndex!=2)return false;r.array=array;r.table=r.authority.table;nfs::GuestCopyTransaction tx;if(tx.load<uint8_t>(byte_512ED3))return false;r.archive=tx.load<uint32_t>(dword_4D4C78);if(!r.archive)return false;
 const char*names[2]={"force feedback","device not found"};for(unsigned i=0;i<2;++i){const uint64_t at=uint64_t(r.table)+12*(i?0x301:0x14a);if(at+12>UINT64_C(0x100000000))return false;for(unsigned k=0;k<12;++k)r.headers[i][k]=tx.load<uint8_t>(uint32_t(at)+k);const uint64_t p=uint64_t(r.table)+nfs_controls_dialog_word::le32(r.headers[i].data()+8);if(!p||p+96>UINT64_C(0x100000000)||tx.load<uint32_t>(array+4*i)!=p)return false;r.pointers[i]=uint32_t(p);const size_t n=std::strlen(names[i]);for(size_t k=0;k<=n;++k){auto ch=tx.load<uint8_t>(uint32_t(p)+uint32_t(k));if(ch!=uint8_t(names[i][k]))return false;r.texts[i][k]=ch;}}
 return !tx.load<uint8_t>(byte_512ED3)&&tx.load<uint32_t>(dword_4D4AE0)==r.table&&tx.load<uint32_t>(dword_4D4C78)==r.archive;
 }catch(...){return false;}}
class Invocation;
struct Slot {std::shared_ptr<const Invocation>invocation;unsigned depth=0;bool invalid=false;};
inline Slot&slot()noexcept{static thread_local Slot s;return s;}
struct ArmScope;
class Invocation final {
 uintptr_t game_=0,thread_=0;bd::Binding binding_{};Resources resources_{};bd::Target initial_{};bd::Plane before_;std::shared_ptr<const bd::SourceCatalog>catalog_;
 Invocation()=default;friend struct ArmScope;
public:
 uintptr_t game()const noexcept{return game_;}uintptr_t thread()const noexcept{return thread_;}
 uint64_t modalEpoch()const noexcept{return binding_.modalEpoch;}uint64_t resourceEpoch()const noexcept{return binding_.resourceEpoch;}uint64_t parentEpoch()const noexcept{return binding_.parentEpoch;}uint64_t lifetime()const noexcept{return binding_.lifetime;}
 const bd::Binding&binding()const noexcept{return binding_;}const auto&authority()const noexcept{return resources_.authority;}const auto&resources()const noexcept{return resources_;}const auto&initialTarget()const noexcept{return initial_;}const auto&before()const noexcept{return before_;}const auto&catalog()const noexcept{return catalog_;}
 bool matchesOwner(Game&g)const noexcept{auto*c=nfs_controls_text::current(g);if(!c||game_!=reinterpret_cast<uintptr_t>(&g)||thread_!=nfs_logo_publisher::threadToken()||c->ownerEpoch!=binding_.parentEpoch||c->lifetime!=binding_.lifetime)return false;Resources now;if(!nfs_feedback_modal_live::resources(binding_.owner,now)||!(now==resources_))return false;try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D4D24)==binding_.targetOwner;}catch(...){return false;}}
};
inline std::shared_ptr<const Invocation>activeInvocation(Game&g)noexcept{auto&s=slot();auto p=s.invocation;return s.depth==1&&!s.invalid&&p&&p->matchesOwner(g)?p:std::shared_ptr<const Invocation>{};}
struct ArmScope {
 Game&g;bool entered=false;int unwind=0;
 explicit ArmScope(Game&game)noexcept:g(game){auto&s=slot();if(s.depth++){s.invalid=true;entered=true;return;}entered=true;s.invalid=false;s.invocation.reset();unwind=nfs_logo_publisher::exceptions();try{
  auto*c=nfs_controls_text::current(g);if(!c||!c->ownerEpoch||!c->lifetime||uint32_t(g.eax)!=2)return;auto p=std::shared_ptr<Invocation>(new Invocation);if(!resources(uint32_t(g.edx),p->resources_)||!explicitMain(p->initial_,p->before_))return;
  const auto expected=expectedSources();std::array<uint32_t,4>header{};if(!read(p->resources_.archive,header.data(),16)||header[0]!=0x49504853||header[1]<16||header[1]>64*1024*1024||!span(p->resources_.archive,header[1]))return;std::vector<uint8_t>archive(header[1]);if(!read(p->resources_.archive,archive.data(),archive.size())||!nfs_small_logo_live::hashMatches(archive.data(),archive.size(),expected[0].archiveSha))return;std::array<std::vector<uint8_t>,2>sources;for(unsigned k=0;k<2;++k){auto&i=expected[k];if(uint64_t(i.archiveOffset)+i.span>archive.size())return;sources[k].assign(archive.begin()+i.archiveOffset,archive.begin()+i.archiveOffset+i.span);}p->catalog_=bd::SourceCatalog::create(expected,sources);if(!p->catalog_)return;
  static std::atomic<uint64_t>epochs{0};uint64_t old=epochs.load(std::memory_order_relaxed);do{if(old==UINT64_MAX)return;}while(!epochs.compare_exchange_weak(old,old+1,std::memory_order_relaxed));p->game_=reinterpret_cast<uintptr_t>(&g);p->thread_=nfs_logo_publisher::threadToken();auto&b=p->binding_;b.owner=p->resources_.array;b.parent=p->resources_.authority.parentMenu;b.form=nfs_controls_text::dialogSource().form;b.archive=p->resources_.archive;b.targetOwner=p->initial_.owner;b.titleResource=p->resources_.pointers[0];b.messageResource=p->resources_.pointers[1];b.ownerEpoch=b.modalEpoch=b.resourceEpoch=old+1;b.parentEpoch=c->ownerEpoch;b.lifetime=c->lifetime;b.surface.lifetime=b.lifetime;std::copy(p->initial_.w.begin(),p->initial_.w.end(),b.surface.geometry.begin());auto&v=b.surface.geometry;v[14]=p->initial_.x[0];v[15]=p->initial_.x[1];v[16]=p->initial_.x[639];v[17]=p->initial_.y[0];v[18]=p->initial_.y[1];v[19]=p->initial_.y[479];for(unsigned k=0;k<2;++k){b.sources[k]=b.archive+expected[k].archiveOffset;b.providerKeys[k]=expected[k].providerKey;}if(!b.valid()||!p->matchesOwner(g))return;s.invocation=std::move(p);
 }catch(...){s.invalid=true;s.invocation.reset();}}
 ~ArmScope(){if(!entered)return;auto&s=slot();if(nfs_logo_publisher::exceptions()>unwind)s.invalid=true;if(s.depth)--s.depth;if(!s.depth){s.invocation.reset();s.invalid=false;}}
 ArmScope(const ArmScope&)=delete;ArmScope&operator=(const ArmScope&)=delete;
};
} // namespace nfs_feedback_modal_live
