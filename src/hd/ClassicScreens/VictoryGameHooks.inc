// PRIVATE Victory source collector. Original engine calls/stores continue unchanged.
#pragma once
#include "VictoryCore.h"
#include "VictoryWordData.h"
#include "GuestCopyTransaction.h"
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/SmallLogoPrepare.h"
#include <cstring>
#include <cerrno>
#include <limits>
namespace nfs_victory_ui_live {
constexpr unsigned Pixels=640*480;
inline bool enabled()noexcept{const char*p=std::getenv("NFS_PRIVATE_VICTORY_HD");return p&&p[0]=='1'&&!p[1];}
inline uintptr_t threadToken()noexcept{static thread_local char value;return reinterpret_cast<uintptr_t>(&value);}
struct Measurement {bool ready=false;uint32_t text=0;int width=0,anchor=0;unsigned alignment=0;};
struct BitmapCapture {unsigned index=12;uint32_t source=0,span=0,mode=0;NfsVictoryWordView view{};nfs_logo_capture::Target target{};std::vector<uint16_t>before,after;bool resolved=false;};
struct State {
 Game*game=nullptr;uintptr_t thread=0;uint64_t epoch=0,serial=0;unsigned owner=0,presenting=0;bool ready=false;
 bool failed=false,capturing=false,string=false,word=false;
 uint32_t textAddress=0,metricAddress=0,fontAddress=0,form=0;uint16_t formType=0;
 struct Store{uint32_t index;uint16_t before,after;};std::vector<Store>stores;
 std::array<int16_t,256>advances{};std::array<bool,256>known{};
 std::shared_ptr<Payload>building;std::shared_ptr<const Frame>published;
 std::vector<std::shared_ptr<BitmapCapture>>bitmapWords;std::shared_ptr<BitmapCapture>activeBitmap;
 std::vector<uint16_t>replay;nfs_soft::TextCommand command{};
 std::array<uint8_t,96>textBefore{};Measurement measurement{};unsigned resourceDepth=0;
 void invalidate()noexcept{failed=true;capturing=string=word=false;building.reset();published.reset();replay.clear();bitmapWords.clear();activeBitmap.reset();}
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

inline int finiteWord(const unsigned char*name){for(unsigned k=0;k<12;++k)if(!std::memcmp(name,nfs_victory_words[k].name,4))return int(k);return -1;}
inline bool bitmapSha(const BitmapCapture&b){if(b.index>=12||!b.source||!b.span)return false;unsigned char sha[32];return nfs_texture_sha256(static_cast<const unsigned char*>(nfs_host_address(b.source,b.span)),b.span,sha)&&!std::memcmp(sha,nfs_victory_words[b.index].source,32);}
struct BitmapScope {
 Game&g;State*s=nullptr;std::shared_ptr<BitmapCapture>b;int unwind=0;
 explicit BitmapScope(Game&x)noexcept:g(x){s=current(g);if(!s||!s->ready||s->capturing)return;safe(g,[&]{unsigned char name[4];{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<4;++k)name[k]=tx.load<uint8_t>(uint32_t(g.eax)+k);}if(!std::memcmp(name,"back",4)){s->bitmapWords.clear();s->activeBitmap.reset();return;}int index=finiteWord(name);if(index<0)return;if(s->activeBitmap||s->bitmapWords.size()>=2)throw 1;b=std::make_shared<BitmapCapture>();b->index=unsigned(index);b->mode=uint32_t(g.edx);if(b->mode!=0x2800&&b->mode!=0x400){b.reset();return;}b->target=target();b->before=plane(b->target);s->activeBitmap=b;unwind=nfs_logo_publisher::exceptions();});}
 ~BitmapScope()noexcept{if(!b||!s||s!=current(g))return;safe(g,[&]{if(s->activeBitmap!=b||nfs_logo_publisher::exceptions()>unwind||!b->resolved||!bitmapSha(*b)||!nfs_logo_capture::same(b->target,target()))throw 1;b->after=plane(b->target);s->bitmapWords.push_back(b);s->activeBitmap.reset();});}
};
inline void bitmapResolved(Game&g)noexcept{auto*s=current(g);if(!s||!s->activeBitmap)return;safe(g,[&]{auto&b=*s->activeBitmap;if(b.resolved)throw 1;const auto&asset=nfs_victory_words[b.index];nfs::GuestCopyTransaction tx;uint32_t source=uint32_t(g.eax);if(uint64_t(source)+asset.span>UINT64_C(0x100000000))throw 1;uint32_t head=tx.load<uint32_t>(source),wh=tx.load<uint32_t>(source+4);if((head&255)!=0x7b||(wh&65535)!=asset.width||(wh>>16)!=asset.height)throw 1;uint32_t pal=source+(head>>8),count=tx.load<uint16_t>(pal+4);if((tx.load<uint32_t>(pal)&255)!=0x2a||!count||count>256||(head>>8)+16+4*count!=asset.span)throw 1;b.source=source;b.span=asset.span;if(!bitmapSha(b))throw 1;b.view.index=b.index;b.view.width=asset.width;b.view.height=asset.height;uint32_t pos=tx.load<uint32_t>(source+12);b.view.left=int32_t(pos<<20)>>20;b.view.top=int32_t(pos<<4)>>20;if(b.view.left<-640||b.view.top<-480||b.view.left>640||b.view.top>480)throw 1;b.resolved=true;});}

inline void metricTable(State&s,uint32_t font,uint32_t table,unsigned count){
 if(s.metricAddress==table&&s.fontAddress==font)return;if(!font||!table||!count||count>4096)throw 1;
 s.known.fill(false);s.advances.fill(0);nfs::GuestCopyTransaction tx;
 for(unsigned k=0;k<count;++k){uint32_t at=table+11*k;unsigned code=tx.load<uint16_t>(at);if(code<256){if(s.known[code])throw 1;s.known[code]=true;s.advances[code]=int8_t(tx.load<uint8_t>(at+8));}}
 s.metricAddress=table;s.fontAddress=font;
}
struct OwnerScope {
 Game&g;bool entered=false;
 explicit OwnerScope(Game&x)noexcept:g(x){if(!enabled())return;safe(g,[&]{
  auto&s=state();if(s.owner){s.invalidate();return;}static std::atomic<uint64_t>epochs{0};uint64_t epoch=epochs.fetch_add(1)+1;if(!epoch)throw 1;
  s=State{};s.game=&g;s.thread=threadToken();s.epoch=epoch;s.owner=1;entered=true;
 });}
 ~OwnerScope()noexcept{if(entered){CpuErrno preserve(g);state()=State{};}}
};
inline void archiveReady(Game&g)noexcept{auto*s=current(g);if(!s)return;safe(g,[&]{uint32_t form=uint32_t(guest_address(off_4D3DB8));nfs::GuestCopyTransaction tx;if(tx.load<uint16_t>(form+8)!=0||!tx.load<uint32_t>(form))throw 1;s->form=form;s->formType=0;s->ready=true;});}
struct Adapter {
 static void begin(Game&g)noexcept{auto*s=current(g);if(!s||!s->ready)return;safe(g,[&]{
  if(s->capturing||s->string||s->word||s->serial==UINT64_MAX)throw 1;
  auto p=std::make_shared<Payload>();p->target=target();p->form=s->form;p->formType=s->formType;p->epoch=s->epoch;p->serial=++s->serial;
  s->stores.clear();s->replay.clear();s->building=std::move(p);s->published.reset();s->metricAddress=s->fontAddress=0;s->capturing=true;
 });}
 static void end(Game&g)noexcept{auto*s=current(g);if(!s||!s->capturing)return;safe(g,[&]{
  if(s->string||s->word||!s->building||s->building->commands.empty())throw 1;
  auto p=s->building;if(!nfs_logo_capture::same(p->target,target()))throw 1;p->original=plane(p->target);p->clean=p->original;
  for(auto i=s->stores.rbegin();i!=s->stores.rend();++i){if(p->clean[i->index]!=i->after)throw 1;p->clean[i->index]=i->before;}
  // Reverse only authenticated finite-word effects after reversing live text.
  if(!s->activeBitmap&&!s->bitmapWords.empty()){auto restored=p->clean;bool valid=true;for(auto it=s->bitmapWords.rbegin();it!=s->bitmapWords.rend();++it){const auto&b=**it;if(!bitmapSha(b)||!nfs_logo_capture::same(b.target,p->target)||restored!=b.after){valid=false;break;}restored=b.before;}if(valid){p->clean=std::move(restored);for(const auto&b:s->bitmapWords)p->words.push_back(b->view);}s->bitmapWords.clear();}
  // Reverse only actual observed text stores. Retain late original nontext sprites; reject any conflicting overlap.
  for(size_t n=0;n<p->commands.size();++n){auto&c=p->commands[n];bool numeric=true;for(unsigned k=0;k<c.length;++k){unsigned v=c.text[k];if(v==9)continue;if(!((v>=48&&v<=57)||v==32||v==58||v==46||v==44||v==45||v==43||v==47))numeric=false;}
   unsigned red=(c.color>>11)&31,green=(c.color>>5)&63,blue=c.color&31;if(numeric&&red>12&&green>20&&blue<12)c.role=1;
   if(n&&c.role==1){auto&shadow=p->commands[n-1];if(shadow.color==0&&shadow.text==c.text&&shadow.x==c.x+1&&shadow.y==c.y+1)shadow.role=1;}
  }
  auto display=nfs_victory_text::PreparedText::createCommands(p->commands,nfs_victory_text::victoryFont(),p);if(!display)throw 1;
  auto f=std::shared_ptr<Frame>(new Frame);f->data_=p;f->display_=std::move(display);s->published=std::move(f);s->building.reset();s->replay.clear();s->capturing=false;
 });}
 static std::shared_ptr<const Frame>acquire(Game&g)noexcept{auto*s=current(g);if(!s||s->presenting!=1||s->capturing||s->string||s->word||!s->published)return {};std::shared_ptr<const Frame>out;safe(g,[&]{
  auto f=s->published;const auto&d=f->data();if(d.epoch!=s->epoch||!nfs_logo_capture::same(d.target,target())||d.original!=plane(d.target))throw 1;out=std::move(f);
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
inline void retire(Game&g,uint32_t owner)noexcept{auto*s=current(g);if(!s||!s->published)return;CpuErrno preserve(g);const auto&t=s->published->data().target;if(owner==t.owner||owner==t.w[8]||owner==t.w[11]||owner==t.w[12])s->invalidate();}
} // namespace nfs_victory_ui_live
