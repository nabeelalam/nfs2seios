// SPDX-License-Identifier: MIT
#include "MenuResponsiveLayout.h"
#include "MenuResponsiveDebug.h"
#include "MenuResponsivePointerHooks.h"
#include "MenuResponsiveCached.h"
#include <array>
#include <atomic>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <mutex>
namespace {
std::mutex mutex;
std::atomic<bool> pointerMetadataFailed{false};
NfsMenuResponsiveLayout shown{};
uint64_t inputGeneration=1;
uint32_t sourceForm=0,sourceX=0,sourceY=0,originalOpponentAddress=0,originalOpponentValid=0;uint64_t sourceEpoch=0;bool sourceCursorValid=false;
void advanceGeneration(){++inputGeneration;}
double cursorX=0,cursorY=0; int32_t guestX=0,guestY=0;
bool valid(const NfsMenuResponsiveLayout& s){
 return s.schema==1&&s.active==1&&s.revision&&s.ownerEpoch&&s.cursorValid==1&&
 s.drawableWidth&&s.drawableHeight&&s.drawableWidth<=8192&&s.drawableHeight<=8192&&
 std::isfinite(s.logicalWidth)&&std::isfinite(s.delta)&&s.logicalWidth>640&&s.logicalWidth<=854&&
 std::fabs(s.logicalWidth-480.0*double(s.drawableWidth)/s.drawableHeight)<0.01&&
 std::fabs(s.delta-(s.logicalWidth-640))<0.01&&std::isfinite(s.sourceCursorX)&&std::isfinite(s.sourceCursorY)&&
 s.sourceCursorX>=0&&s.sourceCursorX<=639&&s.sourceCursorY>=0&&s.sourceCursorY<=479;
}
struct Applied {uint32_t x=0,y=0;};
std::array<Applied,64> applied{};unsigned appliedCount=0;
int32_t lastAppliedX=0,lastAppliedY=0,lastMenuX=0,lastMenuY=0;uint32_t pointerDevice=0,lastSequence=0;
uint64_t nextReadToken=0,nextWarpToken=0;unsigned readsInFlight=0;
struct ReadReceipt {
 uint64_t token=0,epoch=0,generation=0;int32_t before[8]{};uint32_t outX=0,outY=0,menuX=0,menuY=0;
 std::array<NfsMenuPointerEvent,32> events{};uint32_t count=0,device=0;bool delivered=false,invalid=false;
};
thread_local ReadReceipt pointerRead;
thread_local uint64_t pointerWarp=0;
struct CachedOwner {
 uint32_t form=0,descriptor=0,count=0;uintptr_t game=0;
 uint64_t producer=0,revision=0;
 bool valid=false;std::array<NfsMenuCachedWidget,NFS_MENU_CACHED_MAX_WIDGETS> rows{};
} cached;
NfsMenuCachedSnapshot shownCached{};
bool cachedForm(uint32_t form){return form==NFS_MENU_CACHED_SETUP||form==NFS_MENU_CACHED_OPTIONS;}
bool responsiveSource(){return sourceForm==0x401910u||(cachedForm(sourceForm)&&cached.form==sourceForm&&cached.producer&&cached.game);}
/* A prepared next packet does NOT supersede the displayed lease. Packet
   serial/content are immutable typed-view joins in renderer begin; source
   owner/widget identity alone decides whether the shown input map is current. */
bool cachedCurrent(const NfsMenuCachedSnapshot& p){
 return p.schema==NFS_MENU_CACHED_SCHEMA&&cached.valid&&sourceCursorValid&&sourceForm==cached.form&&p.formTag==cached.form&&p.descriptor==cached.descriptor&&p.game==cached.game&&p.producerEpoch==cached.producer&&p.sourceEpoch==sourceEpoch&&p.widgetRevision==cached.revision&&p.packetSerial&&p.contentRevision&&p.count==cached.count&&p.count&&p.count<=NFS_MENU_CACHED_MAX_WIDGETS&&!std::memcmp(p.widgets,cached.rows.data(),p.count*sizeof(NfsMenuCachedWidget));
}
void clear(){shownCached={};appliedCount=0;pointerDevice=lastSequence=0;if(shown.active){shown={};advanceGeneration();}}
void rejectPointer(){clear();++sourceEpoch;}
bool acknowledge(uint32_t x,uint32_t y){
 for(unsigned i=0;i<appliedCount;++i)if(applied[i].x==x&&applied[i].y==y){
  const unsigned consumed=i+1;for(unsigned j=consumed;j<appliedCount;++j)applied[j-consumed]=applied[j];appliedCount-=consumed;return true;
 }
 return false;
}
void rememberApplied(uint32_t x,uint32_t y){
 if(int64_t(x)==lastMenuX&&int64_t(y)==lastMenuY)return;
 if(appliedCount==applied.size()){rejectPointer();return;}
 applied[appliedCount++]={x,y};lastMenuX=int32_t(x);lastMenuY=int32_t(y);
}
}
namespace {
void updateSource(uint32_t form,uint32_t x,uint32_t y,uint32_t slot5,uint32_t observed){
 if(!form)pointerMetadataFailed.store(false,std::memory_order_release);
 if(pointerMetadataFailed.load(std::memory_order_acquire))clear();
 if(form!=sourceForm){clear();sourceForm=form;++sourceEpoch;}
 /* Only exact completed original buffered reads can acknowledge a changed
    source position. Pending host estimates are never observation authority. */
 if(shown.active&&responsiveSource()&&form==sourceForm&&(x!=sourceX||y!=sourceY)&&!acknowledge(x,y))rejectPointer();
 sourceX=x;sourceY=y;sourceCursorValid=x<=639&&y<=479;
 if(!responsiveSource()||!sourceCursorValid)clear();
 originalOpponentValid=observed==1&&responsiveSource()&&sourceCursorValid&&!(slot5&3u)&&slot5<=0x200000u-32768u;
 originalOpponentAddress=originalOpponentValid?slot5:0;
}
}
extern "C" void nfs_menu_responsive_source_form(uint32_t form,uint32_t x,uint32_t y){
 std::lock_guard<std::mutex> lock(mutex);updateSource(form,x,y,0,0);
}
extern "C" void nfs_menu_responsive_source_observation(uint32_t form,uint32_t x,uint32_t y,uint32_t slot5,uint32_t observed){
 std::lock_guard<std::mutex> lock(mutex);updateSource(form,x,y,slot5,observed);
}
extern "C" int nfs_menu_responsive_source_main(uint64_t* epoch,float* x,float* y){
 std::lock_guard<std::mutex> lock(mutex);
 if(pointerMetadataFailed.load(std::memory_order_acquire)){clear();return 0;}
 if(!epoch||!x||!y||sourceForm!=0x401910u||!sourceEpoch||!sourceCursorValid)return 0;
 *epoch=sourceEpoch;*x=float(sourceX);*y=float(sourceY);return 1;
}
extern "C" void nfs_menu_responsive_publish(const NfsMenuResponsiveLayout* value){
 std::lock_guard<std::mutex> lock(mutex);
 if(pointerMetadataFailed.load(std::memory_order_acquire)){clear();return;}
 if(!value||!valid(*value)||sourceForm!=0x401910u||!sourceCursorValid||value->ownerEpoch!=sourceEpoch){clear();return;}
 const auto& s=*value;
 if(shown.active&&shown.ownerEpoch==s.ownerEpoch&&s.revision<=shown.revision){clear();return;}
 const bool changed=!shown.active||shown.ownerEpoch!=s.ownerEpoch||shown.drawableWidth!=s.drawableWidth||shown.drawableHeight!=s.drawableHeight;
 if(changed){cursorX=s.sourceCursorX;cursorY=s.sourceCursorY;guestX=int32_t(s.sourceCursorX);guestY=int32_t(s.sourceCursorY);lastAppliedX=lastMenuX=guestX;lastAppliedY=lastMenuY=guestY;appliedCount=0;pointerDevice=lastSequence=0;advanceGeneration();}
 shown=s;shownCached={};
}
extern "C" void nfs_menu_responsive_revoke(){std::lock_guard<std::mutex> lock(mutex);clear();}
extern "C" int nfs_menu_responsive_snapshot(NfsMenuResponsiveLayout* out){std::lock_guard<std::mutex> lock(mutex);if(!out)return 0;if(pointerMetadataFailed.load(std::memory_order_acquire))clear();*out=shown;return shown.active!=0;}
extern "C" uint64_t nfs_menu_responsive_input_generation(){std::lock_guard<std::mutex> lock(mutex);return inputGeneration;}
extern "C" float nfs_menu_responsive_logical_width(){std::lock_guard<std::mutex> lock(mutex);if(pointerMetadataFailed.load(std::memory_order_acquire))clear();return shown.active?shown.logicalWidth:640.0f;}
extern "C" void nfs_menu_responsive_pointer_route(int32_t dx,int32_t dy,int click,int32_t* ox,int32_t* oy,int* accepted){
 std::lock_guard<std::mutex> lock(mutex);
 if(!ox||!oy||!accepted)return;
 if(pointerMetadataFailed.load(std::memory_order_acquire)){clear();*ox=*oy=*accepted=0;return;}
 if(!shown.active){*ox=dx;*oy=dy;*accepted=click!=0;return;}
 cursorX=std::max(0.0,std::min(double(shown.logicalWidth)-1,cursorX+dx));
 cursorY=std::max(0.0,std::min(479.0,cursorY+dy));
 if(shownCached.schema){
  if(!cachedCurrent(shownCached)){clear();*ox=*oy=*accepted=0;return;}
  // Every enrolled child control/value is one CENTER group. The immutable
  // measured row set controls clicks; a343sidebar boundary is irrelevant.
  const double inverse=cursorX-double(shown.delta)*.5;
  const int32_t x=int32_t(std::max(0.0,std::min(639.0,inverse))),y=int32_t(cursorY);
  unsigned hits=0;
  for(uint32_t i=0;i<shownCached.count;++i){const auto&r=shownCached.widgets[i];
   if(r.enabled&&r.left<x&&x<r.right&&r.top<y&&y<r.bottom)++hits;
  }
  *ox=x-guestX;*oy=y-guestY;guestX=x;guestY=y;*accepted=click&&inverse>=0&&inverse<=639&&hits==1;
 }else{
  /* Accepted original Main retains its exact existing left-menu gate. */
  const int32_t x=int32_t(std::min(639.0,cursorX)),y=int32_t(cursorY);
  *ox=x-guestX;*oy=y-guestY;guestX=x;guestY=y;*accepted=click&&cursorX<343.0;
 }
}
extern "C" int nfs_menu_responsive_pointer_snapshot(NfsMenuResponsivePointer* out){
 std::lock_guard<std::mutex> lock(mutex);if(!out)return 0;
 if(pointerMetadataFailed.load(std::memory_order_acquire))clear();
 *out={shown.revision,shown.ownerEpoch,float(cursorX),float(cursorY),shown.active};return shown.active!=0;
}

extern "C" void nfs_menu_responsive_debug_state(NfsMenuResponsiveDebugState* out){
 std::lock_guard<std::mutex> lock(mutex);if(!out)return;
 *out={3,sourceForm,sourceX,sourceY,uint32_t(sourceCursorValid),shown.active,originalOpponentAddress,originalOpponentValid,
       sourceEpoch,shown.revision,shown.ownerEpoch};
}

/* The receipt is copied only after InputDevices and guest-copy locks end.
   Replay is the original _422060 offset dispatch with its actual source bounds. */
extern "C" uint64_t nfs_menu_pointer_read_begin(const int32_t state[8],uint32_t outX,uint32_t outY,uint32_t menuX,uint32_t menuY,uint32_t outButtons){
 std::lock_guard<std::mutex> lock(mutex);
 if(pointerRead.token){pointerRead.invalid=true;rejectPointer();return 0;}
 if(!shown.active||!responsiveSource())return 0;
 if(!state||state[2]>state[4]||state[3]>state[5]||state[0]!=lastAppliedX||state[1]!=lastAppliedY||state[6]!=lastMenuX||state[7]!=lastMenuY||
    !menuX||!menuY||menuX==menuY||outButtons==menuX||outButtons==menuY){rejectPointer();return 0;}
 if(nextReadToken==UINT64_MAX){rejectPointer();return 0;}
 pointerRead={};pointerRead.token=++nextReadToken;pointerRead.epoch=sourceEpoch;pointerRead.generation=inputGeneration;
 pointerRead.outX=outX;pointerRead.outY=outY;pointerRead.menuX=menuX;pointerRead.menuY=menuY;
 std::memcpy(pointerRead.before,state,sizeof pointerRead.before);++readsInFlight;return pointerRead.token;
}
extern "C" void nfs_menu_pointer_read_receipt(const NfsMenuPointerEvent* events,uint32_t count,int32_t result,uint32_t peek,uint32_t hasDestination,uint32_t device){
 std::lock_guard<std::mutex> lock(mutex);auto& r=pointerRead;if(!r.token)return;
 if(r.delivered||peek||!hasDestination||!device||result||count>r.events.size()||(count&&!events)){r.invalid=true;return;}
 r.delivered=true;r.count=count;r.device=device;for(uint32_t i=0;i<count;++i)r.events[i]=events[i];
}
extern "C" void nfs_menu_pointer_read_finish(uint64_t token,const int32_t state[8],uint32_t complete){
 std::lock_guard<std::mutex> lock(mutex);if(!token||pointerRead.token!=token){if(token)rejectPointer();return;}
 const auto r=pointerRead;pointerRead={};if(readsInFlight)--readsInFlight;
 if(!shown.active||r.epoch!=sourceEpoch)return;
 if(!complete||!state||r.generation!=inputGeneration||r.invalid||!r.delivered||
    (pointerDevice&&r.device!=pointerDevice)||std::memcmp(r.before+2,state+2,4*sizeof(int32_t))){rejectPointer();return;}
 int32_t x=r.before[0],y=r.before[1];uint32_t priorSequence=lastSequence;
 for(uint32_t i=0;i<r.count;++i){const auto& e=r.events[i];
  if(!e.sequence||(priorSequence&&e.sequence<=priorSequence)){rejectPointer();return;}priorSequence=e.sequence;
  if(e.offset==0)x=int32_t(std::max(int64_t(state[2]),std::min(int64_t(state[4]),int64_t(x)+int32_t(e.data))));
  else if(e.offset==4)y=int32_t(std::max(int64_t(state[3]),std::min(int64_t(state[5]),int64_t(y)+int32_t(e.data))));
  else if(e.offset<12||e.offset>15){rejectPointer();return;}
 }
 if(x!=state[0]||y!=state[1]){rejectPointer();return;}
 int32_t mx=r.before[6],my=r.before[7];
 if(r.outX==r.menuX)mx=x;if(r.outX==r.menuY)my=x;
 if(r.outY==r.menuX)mx=y;if(r.outY==r.menuY)my=y;
 if(mx!=state[6]||my!=state[7]){rejectPointer();return;}
 pointerDevice=r.device;lastSequence=priorSequence;lastAppliedX=x;lastAppliedY=y;
 rememberApplied(uint32_t(mx),uint32_t(my));
}
extern "C" uint64_t nfs_menu_pointer_warp_begin(){
 std::lock_guard<std::mutex> lock(mutex);if(pointerWarp){rejectPointer();return 0;}
 if(!responsiveSource())return 0;
 if(nextWarpToken==UINT64_MAX){rejectPointer();return 0;}
 pointerWarp=++nextWarpToken;if(pointerRead.token)pointerRead.invalid=true;
 rejectPointer();return pointerWarp;
}
extern "C" void nfs_menu_pointer_warp_finish(uint64_t token,const int32_t state[8],uint32_t complete){
 std::lock_guard<std::mutex> lock(mutex);if(!token||pointerWarp!=token){if(token)rejectPointer();return;}pointerWarp=0;
 if(!complete||!state){sourceCursorValid=false;clear();return;}
 sourceX=uint32_t(state[6]);sourceY=uint32_t(state[7]);sourceCursorValid=sourceX<=639&&sourceY<=479;
 lastAppliedX=state[0];lastAppliedY=state[1];lastMenuX=state[6];lastMenuY=state[7];appliedCount=0;
}
extern "C" void nfs_menu_pointer_queue_reset(){
 std::lock_guard<std::mutex> lock(mutex);
 if(shown.active&&(readsInFlight||guestX!=lastAppliedX||guestY!=lastAppliedY))rejectPointer();
}

extern "C" void nfs_menu_pointer_metadata_failure()noexcept{pointerMetadataFailed.store(true,std::memory_order_release);}

/* Source callbacks are metadata only. All guest-copy/InputDevices locks have
   ended before these entries take the shared host layout mutex. */
extern "C" void nfs_menu_cached_owner_begin(uint32_t form,uint32_t descriptor,uint64_t producer,uintptr_t game){
 std::lock_guard<std::mutex> lock(mutex);
 if(!cachedForm(form)||!descriptor||!producer||!game){if(shownCached.schema)clear();cached={};return;}
 if(cached.form==form&&cached.descriptor==descriptor&&cached.producer==producer&&cached.game==game)return;
 clear();cached={};cached.form=form;cached.descriptor=descriptor;cached.producer=producer;cached.game=game;
 sourceForm=form;sourceCursorValid=false;originalOpponentValid=0;++sourceEpoch;
}
extern "C" void nfs_menu_cached_owner_end(uint32_t form,uint64_t producer,uintptr_t game){
 std::lock_guard<std::mutex> lock(mutex);
 if(cached.form!=form||cached.producer!=producer||cached.game!=game)return;
 clear();cached={};sourceForm=0;sourceCursorValid=false;originalOpponentValid=0;++sourceEpoch;
}
extern "C" void nfs_menu_cached_widgets(uint32_t form,uint32_t descriptor,uint32_t x,uint32_t y,const NfsMenuCachedWidget*rows,uint32_t count,uint32_t slot5,uint32_t observed){
 std::lock_guard<std::mutex> lock(mutex);
 bool okay=cachedForm(form)&&cached.form==form&&cached.descriptor==descriptor&&cached.producer&&cached.game&&rows&&count&&count<=NFS_MENU_CACHED_MAX_WIDGETS&&x<=639&&y<=479;
 for(uint32_t i=0;okay&&i<count;++i){const auto&r=rows[i];
  okay=(r.itemType==1||r.itemType==2)&&r.enabled<=1&&r.left<r.right&&r.top<r.bottom&&
   r.itemCount&&r.itemCount<=64&&r.itemId<r.itemCount&&count<=r.itemCount&&r.itemCount==rows[0].itemCount;
  for(uint32_t j=0;okay&&j<i;++j)if(rows[j].itemId==r.itemId)okay=false;
 }
 if(!okay){cached.valid=false;if(shownCached.schema)clear();return;}
 const bool changed=!cached.valid||count!=cached.count||std::memcmp(cached.rows.data(),rows,count*sizeof(*rows));
 if(changed){if(cached.revision==UINT64_MAX){cached.valid=false;clear();return;}++cached.revision;if(shownCached.schema)clear();}
 cached.rows={};std::copy(rows,rows+count,cached.rows.begin());cached.count=count;cached.valid=true;
 // Cursor, measured rows and current slot witness enter ONE host transaction.
 updateSource(form,x,y,slot5,observed);
}
extern "C" void nfs_menu_cached_source_observation(uint32_t form,uint32_t descriptor,uint32_t x,uint32_t y,uint32_t slot5,uint32_t observed){
 std::lock_guard<std::mutex> lock(mutex);
 if(!cachedForm(form)||cached.form!=form||cached.descriptor!=descriptor||!cached.producer||!cached.game){updateSource(0,x,y,0,0);return;}
 updateSource(form,x,y,slot5,observed);
}
extern "C" int nfs_menu_cached_snapshot(uint32_t form,uint32_t descriptor,uint64_t producer,uintptr_t game,uint64_t serial,uint64_t content,NfsMenuCachedSnapshot*out){
 std::lock_guard<std::mutex> lock(mutex);if(!out)return 0;*out={};
 if(pointerMetadataFailed.load(std::memory_order_acquire)||!cached.valid||cached.form!=form||cached.descriptor!=descriptor||cached.producer!=producer||cached.game!=game||sourceForm!=form||!sourceCursorValid||!serial||!content)return 0;
 out->schema=NFS_MENU_CACHED_SCHEMA;out->formTag=form;out->descriptor=descriptor;out->count=cached.count;out->game=game;
 out->producerEpoch=producer;out->sourceEpoch=sourceEpoch;out->widgetRevision=cached.revision;out->packetSerial=serial;out->contentRevision=content;out->cursorX=sourceX;out->cursorY=sourceY;
 std::copy(cached.rows.begin(),cached.rows.end(),out->widgets);return 1;
}
extern "C" int nfs_menu_cached_current(const NfsMenuCachedSnapshot*p,uint64_t*epoch,float*x,float*y){
 std::lock_guard<std::mutex> lock(mutex);
 if(!p||!epoch||!x||!y||pointerMetadataFailed.load(std::memory_order_acquire)||!cachedCurrent(*p))return 0;
 *epoch=sourceEpoch;*x=float(sourceX);*y=float(sourceY);return 1;
}
extern "C" void nfs_menu_cached_publish(const NfsMenuResponsiveLayout*value,const NfsMenuCachedSnapshot*p){
 std::lock_guard<std::mutex> lock(mutex);
 if(!value||!p||!valid(*value)||pointerMetadataFailed.load(std::memory_order_acquire)||!cachedCurrent(*p)||value->ownerEpoch!=sourceEpoch){clear();return;}
 if(shown.active&&shown.ownerEpoch==value->ownerEpoch&&value->revision<=shown.revision){clear();return;}
 const bool changed=!shown.active||!shownCached.schema||shown.ownerEpoch!=value->ownerEpoch||shown.drawableWidth!=value->drawableWidth||shown.drawableHeight!=value->drawableHeight;
 if(changed){cursorX=value->sourceCursorX+double(value->delta)*.5;cursorY=value->sourceCursorY;guestX=int32_t(value->sourceCursorX);guestY=int32_t(value->sourceCursorY);lastAppliedX=lastMenuX=guestX;lastAppliedY=lastMenuY=guestY;appliedCount=0;pointerDevice=lastSequence=0;advanceGeneration();}
 shown=*value;shownCached=*p;
}
