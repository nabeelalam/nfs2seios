// SPDX-License-Identifier: MIT
// Exact title.qfs owner408558. No original operation is suppressed.
#pragma once
#include "IntroLayerAbi.h"
#include "IntroArtProvider.h"
#include "IntroSourceCatalog.h"
#include "GuestCopyTransaction.h"
#include "TextureDigest.h"
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/SmallLogoPrepare.h"
#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cerrno>
namespace nfs_intro_ui_live {
constexpr unsigned Pixels=640*480;
struct Frame {nfs_logo_capture::Target target{};std::vector<uint16_t>original;uint32_t archive=0,locale=0;uint64_t epoch=0,serial=0,artGeneration=0;};
struct State {Game*game=nullptr;uintptr_t thread=0;uint32_t archive=0,locale=0;uint64_t epoch=0,serial=0,artGeneration=0;unsigned owner=0,presenting=0;bool failed=false;std::shared_ptr<const Frame>published;std::shared_ptr<const std::vector<uint16_t>>background;nfs_logo_capture::Target backgroundTarget{};};
inline State&state(){static thread_local State s;return s;}
inline uintptr_t threadToken(){static thread_local char value;return reinterpret_cast<uintptr_t>(&value);}
inline State*current(Game&g){auto&s=state();return s.owner==1&&!s.failed&&s.game==&g&&s.thread==threadToken()?&s:nullptr;}
struct Preserve {Game&g;std::array<unsigned char,sizeof(CPU)>cpu{};int error;explicit Preserve(Game&x):g(x),error(errno){std::memcpy(cpu.data(),static_cast<CPU*>(&g),sizeof(CPU));}~Preserve(){std::memcpy(static_cast<CPU*>(&g),cpu.data(),sizeof(CPU));errno=error;}};
inline void reject(State&s){s.failed=true;s.published.reset();s.background.reset();}
template<class F>inline void safe(Game&g,F f)noexcept{Preserve preserve(g);try{f();}catch(...){reject(state());}}
inline nfs_logo_capture::Target target(){nfs_logo_capture::Target t;nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);if(!nfs_small_logo_live::target(t,640)||t.w[3]||t.w[4]||t.w[5]!=640||t.w[6]!=480)throw 1;return t;}
inline std::vector<uint16_t>plane(const nfs_logo_capture::Target&t){std::vector<uint16_t>v(Pixels);nfs::GuestCopyTransaction tx;std::memcpy(v.data(),nfs_host_address(t.w[8]+16,Pixels*2),Pixels*2);return v;}
inline uint32_t resource(uint32_t archive,unsigned index){
 if(!archive||index>=7||uint64_t(archive)+NFS_INTRO_ARCHIVE_BYTES>UINT64_C(0x100000000))throw 1;
 nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(archive)!=0x49504853u||tx.load<uint32_t>(archive+4)!=NFS_INTRO_ARCHIVE_BYTES||tx.load<uint32_t>(archive+8)!=7)throw 1;
 const auto&r=nfs_intro_sources[index];unsigned found=0;for(unsigned k=0;k<7;++k){const auto at=archive+16+8*k;bool same=true;for(unsigned n=0;n<4;++n)if(tx.load<uint8_t>(at+n)!=uint8_t(r.name[n]))same=false;if(same){if(tx.load<uint32_t>(at+4)!=r.offset)throw 1;++found;}}
 if(found!=1||uint64_t(r.offset)+r.span>NFS_INTRO_ARCHIVE_BYTES)throw 1;const uint32_t source=archive+r.offset;unsigned char sha[32];if(!nfs_texture_sha256(static_cast<const unsigned char*>(nfs_host_address(source,r.span)),r.span,sha)||std::memcmp(sha,r.sha,32))throw 1;return source;
}
inline bool sourceCurrent(const Frame&f){if(f.locale>=6||!f.artGeneration||nfs_intro_art_generation()!=f.artGeneration)return false;{nfs::GuestCopyTransaction tx;if(tx.load<uint8_t>(byte_512DFB)!=f.locale)return false;}return resource(f.archive,0)&&resource(f.archive,f.locale+1);}
struct OwnerScope {Game&g;bool entered=false;explicit OwnerScope(Game&x)noexcept:g(x){const char*p=std::getenv("NFS_PRIVATE_INTRO_UI");const uint64_t art=nfs_intro_art_generation();if(!p||p[0]!='1'||p[1]||!art)return;safe(g,[&]{auto&s=state();if(s.owner){reject(s);return;}uint32_t locale;{nfs::GuestCopyTransaction tx;locale=tx.load<uint8_t>(byte_512DFB);}if(locale>=6)return;const uint32_t archive=uint32_t(g.eax);(void)resource(archive,0);(void)resource(archive,locale+1);static std::atomic<uint64_t>epochs{0};const auto epoch=epochs.fetch_add(1)+1;if(!epoch)throw 1;s=State{};s.game=&g;s.thread=threadToken();s.archive=archive;s.locale=locale;s.artGeneration=art;s.epoch=epoch;s.owner=1;entered=true;});}~OwnerScope(){if(entered){Preserve p(g);state()=State{};}}};
struct BackgroundScope {Game&g;State*s=nullptr;std::shared_ptr<std::vector<uint16_t>>pixels;nfs_logo_capture::Target before{};int unwind=0;explicit BackgroundScope(Game&x)noexcept:g(x){s=current(g);if(!s)return;safe(g,[&]{s->published.reset();s->background.reset();const uint32_t source=resource(s->archive,0);if(uint32_t(g.eax)!=source||nfs_intro_sources[0].left||nfs_intro_sources[0].top)throw 1;before=target();pixels=std::make_shared<std::vector<uint16_t>>(Pixels);{nfs::GuestCopyTransaction tx;std::memcpy(pixels->data(),nfs_host_address(source+16,Pixels*2),Pixels*2);}unwind=nfs_logo_publisher::exceptions();});}~BackgroundScope()noexcept{if(!pixels||!s||s!=current(g))return;safe(g,[&]{if(nfs_logo_publisher::exceptions()>unwind||!nfs_logo_capture::same(before,target())||plane(before)!=*pixels)throw 1;(void)resource(s->archive,0);s->background=pixels;s->backgroundTarget=before;});}};
struct CopyrightScope {Game&g;State*s=nullptr;std::shared_ptr<Frame>frame;int unwind=0;explicit CopyrightScope(Game&x)noexcept:g(x){s=current(g);if(!s)return;safe(g,[&]{s->published.reset();if(!s->background||s->presenting)throw 1;const auto&r=nfs_intro_sources[s->locale+1];const uint32_t source=resource(s->archive,s->locale+1);if(uint32_t(g.eax)!=source||r.left<0||r.top<0||r.left+int(r.width)>640||r.top+int(r.height)>480)throw 1;auto f=std::make_shared<Frame>();f->target=target();if(!nfs_logo_capture::same(f->target,s->backgroundTarget)||plane(f->target)!=*s->background)throw 1;f->original=*s->background;{nfs::GuestCopyTransaction tx;const auto*p=static_cast<const uint16_t*>(nfs_host_address(source+16,size_t(r.width)*r.height*2));for(unsigned y=0;y<r.height;++y)for(unsigned x=0;x<r.width;++x){const auto value=p[y*r.width+x];if(value)f->original[(unsigned(r.top)+y)*640+unsigned(r.left)+x]=value;}}if(s->serial==UINT64_MAX)throw 1;f->archive=s->archive;f->locale=s->locale;f->epoch=s->epoch;f->serial=++s->serial;f->artGeneration=s->artGeneration;frame=std::move(f);unwind=nfs_logo_publisher::exceptions();});}~CopyrightScope()noexcept{if(!frame||!s||s!=current(g))return;safe(g,[&]{if(nfs_logo_publisher::exceptions()>unwind||!sourceCurrent(*frame)||!nfs_logo_capture::same(frame->target,target())||plane(frame->target)!=frame->original)throw 1;s->published=std::move(frame);});}};
inline std::shared_ptr<const Frame>acquire(Game&g){auto*s=current(g);if(!s||s->presenting!=1||!s->published)return {};std::shared_ptr<const Frame>out;safe(g,[&]{auto f=s->published;if(f->epoch!=s->epoch||!sourceCurrent(*f)||!nfs_logo_capture::same(f->target,target())||f->original!=plane(f->target))throw 1;out=std::move(f);});return out;}
struct Presentation {Game&g;State*s=nullptr;bool entered=false;explicit Presentation(Game&x):g(x){s=current(g);if(!s||!s->published)return;safe(g,[&]{if(s->presenting)throw 1;s->presenting=1;entered=true;});}~Presentation(){if(entered&&s==current(g))s->presenting=0;}};
class Lease final {std::atomic<unsigned>refs_{1};std::shared_ptr<const Frame>frame_;static void retain(void*p){static_cast<Lease*>(p)->refs_.fetch_add(1);}static void release(void*p){auto*s=static_cast<Lease*>(p);if(s->refs_.fetch_sub(1)==1)delete s;}public:static std::shared_ptr<const Lease>create(std::shared_ptr<const Frame>f){if(!f)return {};auto p=std::shared_ptr<Lease>(new Lease,[](const Lease*x){release(const_cast<Lease*>(x));});p->frame_=std::move(f);return p;}NfsIntroLiveView view(uint64_t request)const{NfsIntroLiveView v{};if(!request||!frame_)return v;const auto&f=*frame_;v.schema=1;v.width=640;v.height=480;v.locale=f.locale;v.request=request;v.content=f.serial;v.artGeneration=f.artGeneration;v.original=f.original.data();v.pixels=f.original.size();v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;return v;}};
inline void retire(Game&g,uint32_t owner)noexcept{auto*s=current(g);if(!s)return;Preserve p(g);if(owner==s->archive)reject(*s);}
}
