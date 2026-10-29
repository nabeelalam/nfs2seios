// SPDX-License-Identifier: MIT
#pragma once
#include "LoadingRuntimeAbi.h"
#include "LoadingWordSourceCatalog.h"
#include "LoadingBackgroundCatalogData.h"
#include "GuestCopyTransaction.h"
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/SmallLogoPrepare.h"
#include <atomic>
#include <array>
#include <memory>
#include <vector>
#include <cstring>
#include <cerrno>
#include <cstdlib>
namespace nfs_loading_ui_live {
constexpr unsigned Pixels=640*480;
struct Frame {nfs_logo_capture::Target target{};std::vector<uint16_t>original,clean;NfsLoadingWordView word{};uint64_t epoch=0,serial=0;uint32_t source=0,span=0,handlerStack=0,locale=0;};
struct State {Game*game=nullptr;uintptr_t thread=0;uint64_t epoch=0,serial=0;unsigned owner=0,presenting=0;bool failed=false,active=false;std::shared_ptr<const Frame>published;std::shared_ptr<const std::vector<uint16_t>>backgroundPixels;nfs_logo_capture::Target backgroundTarget{};};
inline State&state(){static thread_local State s;return s;}
inline uintptr_t threadToken(){static thread_local char value;return reinterpret_cast<uintptr_t>(&value);}
inline State*current(Game&g){auto&s=state();return s.owner==1&&!s.failed&&s.game==&g&&s.thread==threadToken()?&s:nullptr;}
struct Preserve {Game&g;std::array<unsigned char,sizeof(CPU)>cpu{};int error;explicit Preserve(Game&x):g(x),error(errno){std::memcpy(cpu.data(),static_cast<CPU*>(&g),sizeof(CPU));}~Preserve(){std::memcpy(static_cast<CPU*>(&g),cpu.data(),sizeof(CPU));errno=error;}};
inline void reject(State&s){s.failed=true;s.active=false;s.published.reset();s.backgroundPixels.reset();}
template<class F>inline void safe(Game&g,F f)noexcept{Preserve preserve(g);try{f();}catch(...){reject(state());}}
inline nfs_logo_capture::Target target(){nfs_logo_capture::Target t;nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);if(!nfs_small_logo_live::target(t,640)||t.w[3]||t.w[4]||t.w[5]!=640||t.w[6]!=480)throw 1;return t;}
inline std::vector<uint16_t>plane(const nfs_logo_capture::Target&t){std::vector<uint16_t>v(Pixels);nfs::GuestCopyTransaction tx;std::memcpy(v.data(),nfs_host_address(t.w[8]+16,Pixels*2),Pixels*2);return v;}
inline bool sourceCurrent(const Frame&f){if(f.word.index>=48||!f.source||uint64_t(f.source)+f.span>UINT64_C(0x100000000))return false;const auto&a=nfs_loading_word_sources[f.word.index];if(!f.handlerStack||f.locale>=6)return false;nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(f.handlerStack+0x1B0)!=f.locale)return false;uint32_t name=tx.load<uint32_t>(f.handlerStack+0x194+4*f.locale);for(unsigned k=0;k<4;++k)if(tx.load<uint8_t>(name+k)!=uint8_t(a.name[k]))return false;unsigned char sha[32];return f.span==a.span&&nfs_texture_sha256(static_cast<const unsigned char*>(nfs_host_address(f.source,f.span)),f.span,sha)&&!std::memcmp(sha,a.sha,32);}
struct OwnerScope {Game&g;bool entered=false;explicit OwnerScope(Game&x)noexcept:g(x){const char*p=std::getenv("NFS_PRIVATE_LOADING_UI");if(!p||p[0]!='1'||p[1])return;safe(g,[&]{auto&s=state();if(s.owner){reject(s);return;}static std::atomic<uint64_t>epochs{0};const auto epoch=epochs.fetch_add(1)+1;if(!epoch)throw 1;s=State{};s.game=&g;s.thread=threadToken();s.epoch=epoch;s.owner=1;entered=true;});}~OwnerScope(){if(entered){Preserve p(g);state()=State{};}}};
struct BackgroundScope {
 Game&g;State*s=nullptr;std::shared_ptr<std::vector<uint16_t>>pixels;nfs_logo_capture::Target before{};uint32_t source=0;unsigned index=8;int unwind=0;
 explicit BackgroundScope(Game&x)noexcept:g(x){s=current(g);if(!s)return;safe(g,[&]{if(s->active||s->presenting)throw 1;s->published.reset();s->backgroundPixels.reset();source=uint32_t(g.eax);uint32_t format=0,wh=0,pos=0;unsigned char sha[32];
  {nfs::GuestCopyTransaction tx;format=tx.load<uint32_t>(source)&255;wh=tx.load<uint32_t>(source+4);pos=tx.load<uint32_t>(source+12);if(format!=0x78||wh!=((480u<<16)|640u)||uint64_t(source)+614416>UINT64_C(0x100000000))return;if(!nfs_texture_sha256(static_cast<const unsigned char*>(nfs_host_address(source,614416)),614416,sha))throw 1;}
  for(unsigned k=0;k<8;++k){const auto&a=nfs_loading_background_sources[k];if(a.type==format&&a.width==640&&a.height==480&&a.span==614416&&!std::memcmp(sha,a.sha,32)){if((int32_t(pos<<20)>>20)!=a.left||(int32_t(pos<<4)>>20)!=a.top||int32_t(g.edx)!=a.left||int32_t(g.ebx)!=a.top)throw 1;index=k;break;}}if(index>=8)return;
  before=target();pixels=std::make_shared<std::vector<uint16_t>>(Pixels);{nfs::GuestCopyTransaction tx;std::memcpy(pixels->data(),nfs_host_address(source+16,Pixels*2),Pixels*2);}unwind=nfs_logo_publisher::exceptions();
 });}
 ~BackgroundScope()noexcept{if(!pixels||!s||s!=current(g))return;safe(g,[&]{if(nfs_logo_publisher::exceptions()>unwind||!nfs_logo_capture::same(before,target()))throw 1;unsigned char sha[32];{nfs::GuestCopyTransaction tx;if(!nfs_texture_sha256(static_cast<const unsigned char*>(nfs_host_address(source,614416)),614416,sha)||std::memcmp(sha,nfs_loading_background_sources[index].sha,32))throw 1;}if(plane(before)!=*pixels)throw 1;s->backgroundTarget=before;s->backgroundPixels=pixels;});}
};

struct WordScope {
 Game&g;State*s=nullptr;std::shared_ptr<Frame>frame;int unwind=0;
 explicit WordScope(Game&x)noexcept:g(x){s=current(g);if(!s)return;safe(g,[&]{
  if(s->active||s->presenting)throw 1;s->published.reset();if(!s->backgroundPixels)return;const uint32_t source=uint32_t(g.eax),stack=uint32_t(g.esp);uint32_t wh=0,format=0,locale=0,pos=0;
  {nfs::GuestCopyTransaction tx;wh=tx.load<uint32_t>(source+4);format=tx.load<uint32_t>(source)&255;pos=tx.load<uint32_t>(source+12);locale=tx.load<uint32_t>(stack+0x1B0);}if(format!=0x78||locale>=6)return;
  for(unsigned k=0;k<48;++k){const auto&a=nfs_loading_word_sources[k];if((wh&65535)!=a.width||(wh>>16)!=a.height||uint64_t(source)+a.span>UINT64_C(0x100000000))continue;
   auto f=std::make_shared<Frame>();f->source=source;f->span=a.span;f->handlerStack=stack;f->locale=locale;f->word={k,a.left,a.top,a.width,a.height};if(!sourceCurrent(*f))continue;
   if((int32_t(pos<<20)>>20)!=a.left||(int32_t(pos<<4)>>20)!=a.top||int32_t(g.edx)!=a.left||int32_t(g.ebx)!=a.top)throw 1;
   f->target=target();if(!nfs_logo_capture::same(f->target,s->backgroundTarget))throw 1;f->clean=plane(f->target);if(f->clean!=*s->backgroundPixels)throw 1;f->epoch=s->epoch;if(s->serial==UINT64_MAX)throw 1;f->serial=++s->serial;frame=std::move(f);s->active=true;unwind=nfs_logo_publisher::exceptions();break;
  }
 });}
 ~WordScope()noexcept{if(!frame||!s||s!=current(g))return;safe(g,[&]{if(!s->active||nfs_logo_publisher::exceptions()>unwind||!sourceCurrent(*frame)||!nfs_logo_capture::same(frame->target,target()))throw 1;frame->original=plane(frame->target);if(frame->original!=frame->clean)s->published=frame;s->active=false;});}
};

inline std::shared_ptr<const Frame>acquire(Game&g){auto*s=current(g);if(!s||s->active||s->presenting!=1||!s->published)return {};std::shared_ptr<const Frame>out;safe(g,[&]{auto f=s->published;if(f->epoch!=s->epoch||!sourceCurrent(*f)||!nfs_logo_capture::same(f->target,target())||f->original!=plane(f->target))throw 1;out=std::move(f);});return out;}
struct Presentation {Game&g;State*s=nullptr;bool entered=false;explicit Presentation(Game&x):g(x){s=current(g);if(!s||!s->published)return;safe(g,[&]{if(s->active||s->presenting)throw 1;s->presenting=1;entered=true;});}~Presentation(){if(entered&&s==current(g))s->presenting=0;}};
class Lease final {std::atomic<unsigned>refs_{1};std::shared_ptr<const Frame>frame_;static void retain(void*p){static_cast<Lease*>(p)->refs_.fetch_add(1);}static void release(void*p){auto*s=static_cast<Lease*>(p);if(s->refs_.fetch_sub(1)==1)delete s;}public:static std::shared_ptr<const Lease>create(std::shared_ptr<const Frame>f){if(!f)return {};auto p=std::shared_ptr<Lease>(new Lease,[](const Lease*x){release(const_cast<Lease*>(x));});p->frame_=std::move(f);return p;}NfsLoadingLiveView view(uint64_t request)const{NfsLoadingLiveView v{};if(!request||!frame_)return v;const auto&f=*frame_;v.schema=1;v.width=640;v.height=480;v.request=request;v.content=f.serial;v.original=f.original.data();v.clean=f.clean.data();v.pixels=f.original.size();v.word=f.word;v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;return v;}};
}
