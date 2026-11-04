// Dedicated track-information type11 collector. Original calls/stores are unchanged.
#pragma once
#include "TrackInfoCore.h"
#include "GuestCopyTransaction.h"
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/SmallLogoPrepare.h"
#include <cstring>
#include <cerrno>
#include <limits>
namespace nfs_track_info_ui_live {
constexpr unsigned Pixels=640*480;
inline bool enabled()noexcept{const char*p=std::getenv("NFS_PRIVATE_TRACK_INFO_UI");return p&&p[0]=='1'&&!p[1];}
inline uintptr_t threadToken()noexcept{static thread_local char value;return reinterpret_cast<uintptr_t>(&value);}
struct Measurement {bool ready=false;uint32_t text=0;int width=0,anchor=0;unsigned alignment=0;};
struct StaticBitmapScope;
struct State {
 Game*game=nullptr;uintptr_t thread=0;uint64_t epoch=0,serial=0;unsigned owner=0,presenting=0;
 bool failed=false,capturing=false,string=false,word=false;
 uint32_t textAddress=0,metricAddress=0,fontAddress=0,form=0;uint16_t formType=0;
 struct Store{uint32_t index;uint16_t before,after;};std::vector<Store>stores;
 std::array<int16_t,256>advances{};std::array<bool,256>known{};
 std::shared_ptr<Payload>building;std::shared_ptr<const Frame>published;
 std::vector<uint16_t>replay;nfs_soft::TextCommand command{};
 std::array<uint8_t,96>textBefore{};Measurement measurement{};unsigned resourceDepth=0;
 std::shared_ptr<StaticProof>staticBuilding;StaticBitmapScope*staticScope=nullptr;bool staticFailed=false;
 void invalidate()noexcept{failed=true;capturing=string=word=false;building.reset();published.reset();replay.clear();}
};
inline State&state()noexcept{static thread_local State s;return s;}
inline State*ownerCurrent(Game&g)noexcept{auto&s=state();return s.owner==1&&s.game==&g&&s.thread==threadToken()?&s:nullptr;}
inline State*current(Game&g)noexcept{auto*s=ownerCurrent(g);return s&&!s->failed?s:nullptr;}
struct CpuErrno {
 Game&g;std::array<unsigned char,sizeof(CPU)>saved{};int error;
 explicit CpuErrno(Game&x)noexcept:g(x),error(errno){std::memcpy(saved.data(),static_cast<CPU*>(&g),sizeof(CPU));}
 ~CpuErrno()noexcept{std::memcpy(static_cast<CPU*>(&g),saved.data(),sizeof(CPU));errno=error;}
};
template<class F>inline void safe(Game&g,F&&f)noexcept{CpuErrno preserve(g);try{f();}catch(...){state().invalidate();}}
inline nfs_logo_capture::Target target(){nfs_logo_capture::Target t;nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);if(!nfs_small_logo_live::target(t,640)||t.w[3]||t.w[4]||t.w[5]!=640||t.w[6]!=480)throw 1;return t;}
inline std::vector<uint16_t>plane(const nfs_logo_capture::Target&t){std::vector<uint16_t>v(Pixels);nfs::GuestCopyTransaction tx;std::memcpy(v.data(),nfs_host_address(t.w[8]+16,Pixels*2),Pixels*2);return v;}

inline void metricTable(State&s,uint32_t font,uint32_t table,unsigned count){
 if(s.metricAddress==table&&s.fontAddress==font)return;if(!font||!table||!count||count>4096)throw 1;
 s.known.fill(false);s.advances.fill(0);nfs::GuestCopyTransaction tx;
 for(unsigned k=0;k<count;++k){uint32_t at=table+11*k;unsigned code=tx.load<uint16_t>(at);if(code<256){if(s.known[code])throw 1;s.known[code]=true;s.advances[code]=int8_t(tx.load<uint8_t>(at+8));}}
 s.metricAddress=table;s.fontAddress=font;
}
struct OwnerScope {
 Game&g;bool entered=false;
 explicit OwnerScope(Game&x)noexcept:g(x){const uint32_t expected=uint32_t(guest_address(dword_4D2870));const uint16_t typeExpected=11;if(!enabled()||uint32_t(g.eax)!=expected)return;safe(g,[&]{
  auto&s=state();if(s.owner){s.invalidate();return;}uint16_t type;{nfs::GuestCopyTransaction tx;type=tx.load<uint16_t>(uint32_t(g.eax)+8);}if(type!=typeExpected)return;
  static std::atomic<uint64_t>epochs{0};uint64_t epoch=epochs.fetch_add(1)+1;if(!epoch)throw 1;s=State{};s.game=&g;s.thread=threadToken();s.epoch=epoch;s.owner=1;s.form=expected;s.formType=type;entered=true;
 });}
 ~OwnerScope()noexcept{if(entered){CpuErrno preserve(g);state()=State{};}}
};
// A complete source-derived static scene is optional. Refusal preserves the
// old static layer while the separately proved dynamic text remains available.
template<class F>inline void staticSafe(Game&g,F&&f)noexcept{CpuErrno preserve(g);try{f();}catch(...){auto&s=state();s.staticFailed=true;s.staticBuilding.reset();}}
inline uint32_t staticArchiveIndex(uint32_t archive){nfs::GuestCopyTransaction tx;if(!archive||tx.load<uint32_t>(archive)!=0x49504853u)throw 1;const auto bytes=tx.load<uint32_t>(archive+4),entries=tx.load<uint32_t>(archive+8);for(unsigned k=0;k<8;++k)if(bytes==nfs_track_info_archives[k].bytes&&entries==nfs_track_info_archives[k].entries&&uint64_t(archive)+bytes<=UINT64_C(0x100000000))return k;throw 1;}
inline void staticSourceCurrent(const StaticProof&p,unsigned slot){if(slot>=3)throw 1;const auto&d=p.sources[slot];if(d.index>=NFS_TRACK_INFO_SOURCE_COUNT)throw 1;const auto&r=nfs_track_info_sources[d.index];if(r.archive!=p.archiveIndex||r.role!=slot||d.address!=p.archive+r.offset||uint64_t(d.address)+r.span>uint64_t(p.archive)+nfs_track_info_archives[p.archiveIndex].bytes)throw 1;
 if(staticArchiveIndex(p.archive)!=p.archiveIndex)throw 1;nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=p.archive||tx.load<uint8_t>(byte_512DFB)!=p.locale)throw 1;unsigned found=0;for(unsigned k=0;k<nfs_track_info_archives[p.archiveIndex].entries;++k){const auto at=p.archive+16+8*k;bool same=true;for(unsigned j=0;j<4;++j)if(tx.load<uint8_t>(at+j)!=uint8_t(r.name[j]))same=false;if(same){if(tx.load<uint32_t>(at+4)!=r.offset)throw 1;++found;}}if(found!=1)throw 1;unsigned char sha[32];if(!nfs_texture_sha256(static_cast<const unsigned char*>(nfs_host_address(d.address,r.span)),r.span,sha)||std::memcmp(sha,r.sha,32))throw 1;
}
inline void beginStatics(Game&g)noexcept{auto*s=ownerCurrent(g);if(!s)return;staticSafe(g,[&]{
 if(s->failed){const auto epoch=s->epoch,serial=s->serial;const auto form=s->form;const auto type=s->formType;*s=State{};s->game=&g;s->thread=threadToken();s->epoch=epoch;s->serial=serial;s->owner=1;s->form=form;s->formType=type;}
 if(s->capturing||s->string||s->word||s->staticScope||s->serial==UINT64_MAX)throw 1;s->staticFailed=false;s->published.reset();auto p=std::make_shared<StaticProof>();{nfs::GuestCopyTransaction tx;p->archive=tx.load<uint32_t>(dword_4D40B8);p->locale=tx.load<uint8_t>(byte_512DFB);}if(p->locale>=6)throw 1;p->archiveIndex=staticArchiveIndex(p->archive);p->target=target();p->plane=plane(p->target);p->epoch=s->epoch;p->serial=s->serial+1;s->staticBuilding=std::move(p);
 });}
struct StaticBitmapScope {
 Game&g;State*s=nullptr;unsigned slot=0;uint32_t nameAddress=0,flags=0;std::array<uint8_t,5>name{};std::vector<uint16_t>expected;bool entered=false,resolved=false;int unwind=0;
 StaticBitmapScope(Game&x,unsigned ordinal)noexcept:g(x),slot(ordinal){s=current(g);if(!s||s->staticFailed||!s->staticBuilding)return;staticSafe(g,[&]{auto&p=*s->staticBuilding;if(slot>=3||p.completed!=slot||s->staticScope||s->capturing||!nfs_logo_capture::same(p.target,target())||plane(p.target)!=p.plane)throw 1;nameAddress=uint32_t(g.eax);flags=uint32_t(g.edx);if(flags!=(slot==2?0x1000u:0x400u)||uint32_t(g.ebx)||uint32_t(g.ecx))throw 1;{nfs::GuestCopyTransaction tx;for(unsigned j=0;j<5;++j)name[j]=tx.load<uint8_t>(nameAddress+j);}if(name[4]||!name[0])throw 1;s->staticScope=this;entered=true;unwind=nfs_logo_publisher::exceptions();});}
 void resolve()noexcept{if(!entered||!s||s!=current(g)||s->staticFailed||!s->staticBuilding)return;staticSafe(g,[&]{if(resolved||uint32_t(g.edx)!=flags)throw 1;auto&p=*s->staticBuilding;const uint32_t source=uint32_t(g.eax);unsigned found=NFS_TRACK_INFO_SOURCE_COUNT;for(unsigned k=0;k<NFS_TRACK_INFO_SOURCE_COUNT;++k){const auto&r=nfs_track_info_sources[k];if(r.archive==p.archiveIndex&&r.role==slot&&source==p.archive+r.offset&&!std::memcmp(name.data(),r.name,4)){if(found!=NFS_TRACK_INFO_SOURCE_COUNT)throw 1;found=k;}}if(found==NFS_TRACK_INFO_SOURCE_COUNT)throw 1;
 const auto&r=nfs_track_info_sources[found];auto&d=p.sources[slot];d.index=found;d.address=source;d.clip={int32_t(p.target.w[3]),int32_t(p.target.w[4]),int32_t(p.target.w[5]),int32_t(p.target.w[6])};staticSourceCurrent(p,slot);std::vector<uint16_t>pixels(size_t(r.width)*r.height);{nfs::GuestCopyTransaction tx;const uint32_t wh=tx.load<uint32_t>(source+4),pos=tx.load<uint32_t>(source+12);if((tx.load<uint32_t>(source)&255)!=120||wh!=(r.width|(r.height<<16))||(int32_t(pos<<20)>>20)!=r.left||(int32_t(pos<<4)>>20)!=r.top||tx.load<uint8_t>(byte_4DB25C)!=16||tx.load<uint8_t>(byte_4DB25D)!=120||tx.load<uint8_t>(byte_4DB25E))throw 1;const uint32_t table=slot==2?uint32_t(guest_address(off_4DB54C)):uint32_t(guest_address(off_4DADB4));if(tx.load<uint32_t>(table+12)!=(slot==2?uint32_t(game_id_sub_486BC8):uint32_t(game_id_sub_483438)))throw 1;std::memcpy(pixels.data(),nfs_host_address(source+16,pixels.size()*2),pixels.size()*2);}
 expected=p.plane;for(unsigned y=0;y<r.height;++y)for(unsigned x=0;x<r.width;++x){const int64_t dx=int64_t(r.left)+x,dy=int64_t(r.top)+y;if(dx<d.clip[0]||dx>=d.clip[2]||dy<d.clip[1]||dy>=d.clip[3])continue;const auto value=pixels[size_t(y)*r.width+x];if(slot!=2||value)expected[size_t(dy)*640+size_t(dx)]=value;}resolved=true;
 });}
 ~StaticBitmapScope()noexcept{if(!entered||!s)return;if(s==ownerCurrent(g)&&s->staticScope==this)s->staticScope=nullptr;if(s!=current(g)||s->staticFailed||!s->staticBuilding)return;staticSafe(g,[&]{auto&p=*s->staticBuilding;if(!resolved||p.completed!=slot||nfs_logo_publisher::exceptions()>unwind||!nfs_logo_capture::same(p.target,target())||plane(p.target)!=expected)throw 1;{nfs::GuestCopyTransaction tx;for(unsigned j=0;j<5;++j)if(tx.load<uint8_t>(nameAddress+j)!=name[j])throw 1;}staticSourceCurrent(p,slot);p.plane.swap(expected);++p.completed;});}
};
inline void staticResolved(Game&g)noexcept{auto*s=current(g);if(s&&s->staticScope)s->staticScope->resolve();}
inline bool staticsCurrent(const Payload&p){if(!p.statics)return !p.art;const auto&s=*p.statics;if(s.completed!=3||s.epoch!=p.epoch||s.serial!=p.serial||!nfs_logo_capture::same(s.target,p.target))return false;for(unsigned k=0;k<3;++k)staticSourceCurrent(s,k);return !p.art||(p.art->view.generation&&p.art->view.generation==nfs_track_info_art_generation());}
struct Adapter {
 static void begin(Game&g)noexcept{auto*s=ownerCurrent(g);if(!s)return;safe(g,[&]{
  // A refused frame must not poison this handler's later complete frames.
  // Retry only at its original full-frame boundary; every proof is recaptured.
  if(s->failed){const auto epoch=s->epoch,serial=s->serial;const auto form=s->form;const auto formType=s->formType;*s=State{};s->game=&g;s->thread=threadToken();s->epoch=epoch;s->serial=serial;s->owner=1;s->form=form;s->formType=formType;}
  if(s->capturing||s->string||s->word||s->serial==UINT64_MAX)throw 1;
  auto p=std::make_shared<Payload>();p->target=target();p->form=s->form;p->formType=s->formType;{nfs::GuestCopyTransaction tx;if(p->formType!=11||tx.load<uint8_t>(byte_512DFB)>5)throw 1;}p->epoch=s->epoch;p->serial=++s->serial;
  s->stores.clear();s->replay.clear();s->building=std::move(p);s->published.reset();s->metricAddress=s->fontAddress=0;s->capturing=true;
 });}
 static void end(Game&g)noexcept{auto*s=current(g);if(!s||!s->capturing)return;safe(g,[&]{
  if(s->string||s->word||!s->building||s->building->commands.empty())throw 1;
  auto p=s->building;if(!nfs_logo_capture::same(p->target,target()))throw 1;p->original=plane(p->target);p->clean=p->original;
  for(auto i=s->stores.rbegin();i!=s->stores.rend();++i){if(p->clean[i->index]!=i->after)throw 1;p->clean[i->index]=i->before;}
  // Reverse only actual observed text stores. Retain late original nontext sprites; reject any conflicting overlap.
  for(size_t n=0;n<p->commands.size();++n){auto&c=p->commands[n];bool numeric=true;for(unsigned k=0;k<c.length;++k){unsigned v=c.text[k];if(v==9)continue;if(!((v>=48&&v<=57)||v==32||v==58||v==46||v==44||v==45||v==43||v==47))numeric=false;}
   unsigned red=(c.color>>11)&31,green=(c.color>>5)&63,blue=c.color&31;if(numeric&&red>12&&green>20&&blue<12)c.role=1;
   if(n&&c.role==1){auto&shadow=p->commands[n-1];if(shadow.color==0&&shadow.text==c.text&&shadow.x==c.x+1&&shadow.y==c.y+1)shadow.role=1;}
  }
  // Only a complete three-draw replay may supply replacement statics. The
  // post-text inverse must join its complete output exactly; otherwise retain
  // the original clean statics and the independently proved dynamic text.
  if(!s->staticFailed&&s->staticBuilding&&s->staticBuilding->completed==3&&s->staticBuilding->epoch==p->epoch&&s->staticBuilding->serial==p->serial&&nfs_logo_capture::same(s->staticBuilding->target,p->target)&&s->staticBuilding->plane==p->clean){p->statics=s->staticBuilding;p->art=StaticArt::acquire(*p->statics);}
  auto display=nfs_results_text::PreparedText::createCommands(p->commands,nfs_results_text::resultsFont(),p);if(!display)throw 1;
  auto f=std::shared_ptr<Frame>(new Frame);f->data_=p;f->display_=std::move(display);s->published=std::move(f);s->building.reset();s->replay.clear();s->capturing=false;
 });}
 static std::shared_ptr<const Frame>acquire(Game&g)noexcept{auto*s=current(g);if(!s||s->presenting!=1||s->capturing||s->string||s->word||!s->published)return {};std::shared_ptr<const Frame>out;safe(g,[&]{
  auto f=s->published;const auto&d=f->data();if(d.epoch!=s->epoch||!nfs_logo_capture::same(d.target,target())||d.original!=plane(d.target)||!staticsCurrent(d))throw 1;out=std::move(f);
 });return out;}
 static bool reacquire(Game&g,const std::shared_ptr<const Frame>&f)noexcept{auto now=acquire(g);return now&&now.get()==f.get()&&!now.owner_before(f)&&!f.owner_before(now);}
};
struct ResourceScope {
 Game&g;State*s=nullptr;Measurement old{};bool entered=false;
 explicit ResourceScope(Game&x)noexcept:g(x){s=current(g);if(!s||!s->capturing)return;old=s->measurement;s->measurement={};++s->resourceDepth;entered=true;}
 ~ResourceScope()noexcept{if(entered&&s&&s==current(g)){s->measurement=old;if(s->resourceDepth)--s->resourceDepth;}}
};
inline void measured(Game&g,uint32_t text,int width,unsigned alignment)noexcept{
 auto*s=current(g);if(!s||!s->capturing||!s->resourceDepth||s->string)return;
 safe(g,[&]{if(width<=0||width>4096||(alignment!=1&&alignment!=2))throw 1;s->measurement={true,text,width,int(g.esi),alignment};});
}
struct StringScope {
 Game&g;State*s=nullptr;bool entered=false;
 explicit StringScope(Game&x)noexcept:g(x){s=current(g);if(!s||!s->capturing)return;safe(g,[&]{
  if(s->string||s->word||s->building->commands.size()>=256)throw 1;
  auto&c=s->command;c={};s->textBefore.fill(0);s->textAddress=uint32_t(g.eax);
  uint32_t font=0,table=0,count=0,size=0,callback=0;
  {nfs::GuestCopyTransaction tx;for(unsigned k=0;k<96;++k){uint8_t value=tx.load<uint8_t>(s->textAddress+k);s->textBefore[k]=value;if(!value){c.length=k;break;}}
   // Original 405CF0 draws no glyphs for an empty string (e.g. a blank name).
   // Skip that command; any unexpected original stores still fail final replay.
   if(!c.length){if(!s->textBefore[0])return;throw 1;}
   if(c.length>95||s->textBefore[c.length])throw 1;
   font=tx.load<uint32_t>(dword_4DD770);table=tx.load<uint32_t>(dword_4DD76C);count=tx.load<uint32_t>(dword_4DD75C);size=tx.load<uint32_t>(dword_4E54AC);callback=tx.load<uint32_t>(dword_4DD77C);
   c.color=uint16_t(tx.load<uint32_t>(dword_4DD714));c.originX=tx.load<int32_t>(dword_4DD6F4);c.originY=tx.load<int32_t>(dword_4DD6F8);
  }
  size&=255;if(callback!=uint32_t(game_id_sub_4A5830)||(size!=13&&size!=14&&size!=16&&size!=18&&size!=24&&size!=36))throw 1;metricTable(*s,font,table,count);
  bool terminal=false;int width=0;for(unsigned k=0;k<c.length;++k){unsigned code=s->textBefore[k];c.text[k]=uint8_t(code);if(code==9){terminal=true;continue;}if(terminal||!s->known[code]||nfs_soft::index(uint8_t(code))<0)throw 1;width+=s->advances[code];}
  if(width<0||width>4096)throw 1;c.sourceSize=size;c.x=g.edx;c.y=g.ebx;c.advance=width;c.clip={0,0,640,480};c.alignment=0;c.role=0;
  const auto&m=s->measurement;if(m.ready&&m.text==s->textAddress){int expected=m.anchor-(m.alignment==2?m.width:m.width/2);if(c.x==expected||c.x==expected+1){c.alignment=m.alignment;c.advance=m.width;}}
  s->string=true;entered=true;
 });}
 ~StringScope()noexcept{if(!entered||!s||s!=current(g))return;safe(g,[&]{
  if(!s->string||s->word||!s->building)throw 1;
  {nfs::GuestCopyTransaction tx;for(unsigned k=0;k<=s->command.length;++k)if(tx.load<uint8_t>(s->textAddress+k)!=s->textBefore[k])throw 1;}
  s->building->commands.push_back(s->command);s->string=false;
 });}
};
struct Word {
 Game&g;State*s=nullptr;uint32_t address=0,index=0;uint16_t before=0,value=0;bool entered=false;
 Word(Game&x,uint32_t at,uint16_t computed)noexcept:g(x){s=current(g);if(!s||!s->capturing||!s->string)return;safe(g,[&]{
  if(s->word||!s->building)throw 1;uint64_t first=uint64_t(s->building->target.w[8])+16;if(at<first||uint64_t(at)+2>first+Pixels*2||(at-first)%2)throw 1;address=at;index=unsigned((at-first)/2);value=computed;
  {nfs::GuestCopyTransaction tx;before=tx.load<uint16_t>(at);}if(s->stores.size()>=1000000)throw 1;s->word=true;entered=true;
 });}
 ~Word()noexcept{if(!entered||!s||s!=current(g))return;safe(g,[&]{uint16_t after;{nfs::GuestCopyTransaction tx;after=tx.load<uint16_t>(address);}if(after!=value)throw 1;s->stores.push_back({index,before,after});s->word=false;});}
};
struct Presentation {
 Game&g;State*s=nullptr;bool entered=false;
 explicit Presentation(Game&x)noexcept:g(x){s=current(g);if(!s||!s->published)return;safe(g,[&]{if(s->presenting||s->capturing)throw 1;s->presenting=1;entered=true;});}
 ~Presentation()noexcept{if(entered&&s&&s==current(g))s->presenting=0;}
};
inline void retire(Game&g,uint32_t owner)noexcept{auto*s=current(g);if(!s)return;if(s->staticBuilding&&owner==s->staticBuilding->archive){CpuErrno preserve(g);s->staticFailed=true;s->staticBuilding.reset();s->published.reset();}if(!s->published)return;CpuErrno preserve(g);const auto&t=s->published->data().target;if(owner==t.owner||owner==t.w[8]||owner==t.w[11]||owner==t.w[12]||(s->published->data().statics&&owner==s->published->data().statics->archive))s->invalidate();}
} // namespace nfs_track_info_ui_live
