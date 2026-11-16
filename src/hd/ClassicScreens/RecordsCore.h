// PRIVATE immutable Records display lease. Live source construction stays in Adapter.
#pragma once
#include "ClassicTextDisplay.h"
#include "RecordsFont.h"
#include "RecordsLayerAbi.h"
#include "hd/OriginalUiRecords.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include "hd/Main343OriginLifetime.h"
namespace nfs_records_ui_live {
struct Adapter;
struct Payload {
 nfs_logo_capture::Target target{};
 std::vector<uint16_t>clean,original;
 std::vector<nfs_soft::TextCommand>commands;
 std::array<uint8_t,620>records{};
 uint32_t recordsAddress=0,handlerStack=0;bool postrace=false;NfsRecordsBackdropView backdrop{};
 uint64_t epoch=0,serial=0;uintptr_t game=0;std::shared_ptr<std::atomic<uint64_t>>live;std::shared_ptr<const nfs_main_sidebar_origin::GameLife>gameLife;
};
class Frame final {
 friend struct Adapter;std::shared_ptr<const Payload>data_;std::shared_ptr<const nfs_classic_ui::PreparedText>display_;
 Frame()=default;
public:
 const Payload&data()const noexcept{return *data_;}
 const std::shared_ptr<const nfs_classic_ui::PreparedText>&display()const noexcept{return display_;}
};
class Lease final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const Frame>frame_;Lease()=default;
 static void retain(void*p){static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){auto*s=static_cast<Lease*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
 static int current(void*p){const auto*l=static_cast<const Lease*>(p);if(!l||!l->frame_)return 0;const auto&d=l->frame_->data();const uint64_t observed=d.live?d.live->load(std::memory_order_acquire):0;const bool gameCurrent=d.gameLife&&d.gameLife->current();const bool ok=d.game&&d.epoch&&d.serial&&d.live&&observed==d.serial&&gameCurrent;if(!ok){const int saved=errno;const char*flag=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(flag&&flag[0]=='1'&&!flag[1]){static std::atomic<unsigned>lines{0};unsigned line=lines.load(std::memory_order_relaxed);while(line<16&&!lines.compare_exchange_weak(line,line+1,std::memory_order_relaxed)){}if(line<16)std::fprintf(stderr,"RECORDS WIDE SOURCE refused epoch=%llu expectedSerial=%llu observedSerial=%llu token=%u gameLife=%u\n",(unsigned long long)d.epoch,(unsigned long long)d.serial,(unsigned long long)observed,unsigned(bool(d.live)),unsigned(gameCurrent));}errno=saved;}return ok;}
public:
 static std::shared_ptr<const Lease>create(std::shared_ptr<const Frame>f)noexcept{if(!f||!f->display())return {};try{auto p=std::shared_ptr<Lease>(new Lease,[](const Lease*x){release(const_cast<Lease*>(x));});p->frame_=std::move(f);return p;}catch(...){return {};}}
 NfsRecordsLiveView view(uint64_t request)const noexcept{NfsRecordsLiveView v{};if(!request||!frame_)return v;const auto&d=frame_->data();v.schema=1;v.width=640;v.height=480;v.request=request;v.content=d.serial;v.backdrop=d.backdrop;v.clean=d.clean.data();v.original=d.original.data();v.pixels=d.original.size();v.text=frame_->display()->layer();v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;v.sourceGame=d.game;v.sourceEpoch=d.epoch;v.current=current;return v;}
};
}
