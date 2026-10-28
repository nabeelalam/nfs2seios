// PRIVATE Records source collector. Original engine calls/stores continue unchanged.
#pragma once
#include "RecordsCore.h"
#include "GuestCopyTransaction.h"
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/SmallLogoPrepare.h"
#include <cstring>
#include <cerrno>
#include <limits>
namespace nfs_records_ui_live {
constexpr unsigned Pixels=640*480;
inline bool enabled()noexcept{const char*p=std::getenv("NFS_PRIVATE_RECORDS_HD");return p&&p[0]=='1'&&!p[1];}
inline uintptr_t threadToken()noexcept{static thread_local char value;return reinterpret_cast<uintptr_t>(&value);}
struct Measurement {bool ready=false;uint32_t text=0;int width=0,anchor=0;unsigned alignment=0;};
struct State {
 Game*game=nullptr;uintptr_t thread=0;uint64_t epoch=0,serial=0;unsigned owner=0,presenting=0;
 bool failed=false,capturing=false,string=false,word=false;
 uint32_t textAddress=0,metricAddress=0,fontAddress=0;
 std::array<int16_t,256>advances{};std::array<bool,256>known{};
 std::shared_ptr<Payload>building;std::shared_ptr<const Frame>published;
 std::vector<uint16_t>replay;nfs_soft::TextCommand command{};
 std::array<uint8_t,96>textBefore{};Measurement measurement{};unsigned resourceDepth=0;
 void invalidate()noexcept{failed=true;capturing=string=word=false;building.reset();published.reset();replay.clear();}
};
inline State&state()noexcept{static thread_local State s;return s;}
inline State*current(Game&g)noexcept{auto&s=state();return s.owner==1&&s.game==&g&&s.thread==threadToken()&&!s.failed?&s:nullptr;}
struct CpuErrno {
 Game&g;std::array<unsigned char,sizeof(CPU)>saved{};int error;
 explicit CpuErrno(Game&x)noexcept:g(x),error(errno){std::memcpy(saved.data(),static_cast<CPU*>(&g),sizeof(CPU));}
 ~CpuErrno()noexcept{std::memcpy(static_cast<CPU*>(&g),saved.data(),sizeof(CPU));errno=error;}
};
template<class F>inline void safe(Game&g,F&&f)noexcept{CpuErrno preserve(g);try{f();}catch(...){state().invalidate();}}
inline nfs_logo_capture::Target target(){nfs_logo_capture::Target t;nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);if(!nfs_small_logo_live::target(t,640)||t.w[3]||t.w[4]||t.w[5]!=640||t.w[6]!=480)throw 1;return t;}
inline std::vector<uint16_t>plane(const nfs_logo_capture::Target&t){std::vector<uint16_t>v(Pixels);nfs::GuestCopyTransaction tx;std::memcpy(v.data(),nfs_host_address(t.w[8]+16,Pixels*2),Pixels*2);return v;}
inline std::array<uint8_t,620>records(uint32_t at){std::array<uint8_t,620>v{};nfs::GuestCopyTransaction tx;std::memcpy(v.data(),nfs_host_address(at,v.size()),v.size());return v;}
inline void metricTable(State&s,uint32_t font,uint32_t table,unsigned count){
 if(s.metricAddress==table&&s.fontAddress==font)return;if(!font||!table||!count||count>4096)throw 1;
 s.known.fill(false);s.advances.fill(0);nfs::GuestCopyTransaction tx;
 for(unsigned k=0;k<count;++k){uint32_t at=table+11*k;unsigned code=tx.load<uint16_t>(at);if(code<256){if(s.known[code])throw 1;s.known[code]=true;s.advances[code]=int8_t(tx.load<uint8_t>(at+8));}}
 s.metricAddress=table;s.fontAddress=font;
}
struct OwnerScope {
 Game&g;bool entered=false;
 explicit OwnerScope(Game&x)noexcept:g(x){if(!enabled()||uint32_t(g.eax)!=uint32_t(guest_address(dword_4D2944)))return;safe(g,[&]{
  auto&s=state();if(s.owner){s.invalidate();return;}uint16_t type;{nfs::GuestCopyTransaction tx;type=tx.load<uint16_t>(uint32_t(g.eax)+8);}if(type!=10)return;
  static std::atomic<uint64_t>epochs{0};uint64_t epoch=epochs.fetch_add(1)+1;if(!epoch)throw 1;s=State{};s.game=&g;s.thread=threadToken();s.epoch=epoch;s.owner=1;entered=true;
 });}
 ~OwnerScope()noexcept{if(entered){CpuErrno preserve(g);state()=State{};}}
};
inline NfsRecordsBackdropView readBackdrop(uint32_t stack,uint64_t epoch,uint64_t serial){NfsRecordsBackdropView v{};nfs::GuestCopyTransaction tx;v.schema=1;v.trackIndex=tx.load<uint16_t>(stack+0xf0);v.language=tx.load<uint8_t>(byte_512DFB);v.frameEpoch=epoch;v.content=serial;v.sourceArchive=tx.load<uint32_t>(dword_4D40B8);if(v.trackIndex>7||v.language>5||!v.sourceArchive)throw 1;return v;}
inline bool backdropCurrent(const Payload&p){return nfs_records_backdrop_same(p.backdrop,readBackdrop(p.handlerStack,p.epoch,p.serial));}
struct Adapter {
 static void begin(Game&g)noexcept{auto*s=current(g);if(!s)return;safe(g,[&]{
  if(s->capturing||s->string||s->word||s->serial==UINT64_MAX)throw 1;
  auto p=std::make_shared<Payload>();p->target=target();p->clean=plane(p->target);p->recordsAddress=uint32_t(g.ebp);p->records=records(p->recordsAddress);p->epoch=s->epoch;p->serial=++s->serial;p->handlerStack=uint32_t(g.esp);p->backdrop=readBackdrop(p->handlerStack,p->epoch,p->serial);
  s->replay=p->clean;s->building=std::move(p);s->published.reset();s->metricAddress=s->fontAddress=0;s->capturing=true;
 });}
 static void end(Game&g)noexcept{auto*s=current(g);if(!s||!s->capturing)return;safe(g,[&]{
  if(s->string||s->word||!s->building||s->building->commands.empty())throw 1;
  auto p=s->building;if(!backdropCurrent(*p)||!nfs_logo_capture::same(p->target,target())||p->records!=records(p->recordsAddress))throw 1;p->original=plane(p->target);if(p->original!=s->replay)throw 1;
  auto display=nfs_classic_ui::PreparedText::createCommands(p->commands,nfs_classic_ui::recordsFont(),p);if(!display)throw 1;
  auto f=std::shared_ptr<Frame>(new Frame);f->data_=p;f->display_=std::move(display);s->published=std::move(f);s->building.reset();s->replay.clear();s->capturing=false;
 });}
 static std::shared_ptr<const Frame>acquire(Game&g)noexcept{auto*s=current(g);if(!s||s->presenting!=1||s->capturing||s->string||s->word||!s->published)return {};std::shared_ptr<const Frame>out;safe(g,[&]{
  auto f=s->published;const auto&d=f->data();if(!backdropCurrent(d)||d.epoch!=s->epoch||!nfs_logo_capture::same(d.target,target())||d.records!=records(d.recordsAddress)||d.original!=plane(d.target))throw 1;out=std::move(f);
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
   if(!c.length||c.length>95||s->textBefore[c.length])throw 1;
   font=tx.load<uint32_t>(dword_4DD770);table=tx.load<uint32_t>(dword_4DD76C);count=tx.load<uint32_t>(dword_4DD75C);size=tx.load<uint32_t>(dword_4E54AC);callback=tx.load<uint32_t>(dword_4DD77C);
   c.color=uint16_t(tx.load<uint32_t>(dword_4DD714));c.originX=tx.load<int32_t>(dword_4DD6F4);c.originY=tx.load<int32_t>(dword_4DD6F8);
  }
  if(callback!=uint32_t(game_id_sub_4A5830)||size!=18)throw 1;metricTable(*s,font,table,count);
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
  {nfs::GuestCopyTransaction tx;before=tx.load<uint16_t>(at);}if(s->replay[index]!=before)throw 1;s->word=true;entered=true;
 });}
 ~Word()noexcept{if(!entered||!s||s!=current(g))return;safe(g,[&]{uint16_t after;{nfs::GuestCopyTransaction tx;after=tx.load<uint16_t>(address);}if(after!=value)throw 1;s->replay[index]=after;s->word=false;});}
};
struct Presentation {
 Game&g;State*s=nullptr;bool entered=false;
 explicit Presentation(Game&x)noexcept:g(x){s=current(g);if(!s||!s->published)return;safe(g,[&]{if(s->presenting||s->capturing)throw 1;s->presenting=1;entered=true;});}
 ~Presentation()noexcept{if(entered&&s&&s==current(g))s->presenting=0;}
};
inline void retire(Game&g,uint32_t owner)noexcept{auto*s=current(g);if(!s||!s->published)return;CpuErrno preserve(g);const auto&t=s->published->data().target;if(owner==s->published->data().recordsAddress||owner==t.owner||owner==t.w[8]||owner==t.w[11]||owner==t.w[12])s->invalidate();}
} // namespace nfs_records_ui_live
