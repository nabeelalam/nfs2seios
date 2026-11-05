// PRIVATE Results source collector. Original engine calls/stores continue unchanged.
#pragma once
#include "ResultsCore.h"
#include "GuestCopyTransaction.h"
#include "TextureDigest.h"
#include "hd/BackgroundSceneArt.h"
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/SmallLogoPrepare.h"
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <limits>
namespace nfs_results_ui_live {
constexpr unsigned Pixels=640*480;
inline bool enabled()noexcept{const char*p=std::getenv("NFS_PRIVATE_RESULTS_HD");return p&&p[0]=='1'&&!p[1];}
inline unsigned headingForType(unsigned t)noexcept{switch(t){case 3:return 1;case 4:return 2;case 5:return 3;case 6:return 4;case 16:return 5;case 15:return 6;default:return 0;}}
inline void trace(const char* stage,uint32_t a=0,uint32_t b=0)noexcept{
 const char*flag=std::getenv("NFS_PRIVATE_RESULTS_TRACE");
 if(!flag||flag[0]!='1'||flag[1])return;
 static thread_local unsigned frameLines=0,recordLines=0;
 const bool record=std::strncmp(stage,"record-",7)==0;
 unsigned&lines=record?recordLines:frameLines;
 if(lines>=(record?12u:48u))return;
 ++lines;
 const int saved=errno;
 std::fprintf(stderr,"RESULTS SOURCE %s a=%u b=%u\n",stage,a,b);
 errno=saved;
}
inline uintptr_t threadToken()noexcept{static thread_local char value;return reinterpret_cast<uintptr_t>(&value);}
struct Measurement {bool ready=false;uint32_t text=0;int width=0,anchor=0;unsigned alignment=0;};
struct State {
 Game*game=nullptr;uintptr_t thread=0;uint64_t epoch=0,serial=0;unsigned owner=0,presenting=0;
 bool failed=false,capturing=false,string=false,word=false;
 uint32_t textAddress=0,metricAddress=0,fontAddress=0,form=0;uint16_t formType=0;
 struct Store{uint32_t index;uint16_t before,after;};std::vector<Store>stores;
 std::array<int16_t,256>advances{};std::array<bool,256>known{};
 std::shared_ptr<Payload>building;std::shared_ptr<const Frame>published;
 nfs_logo_capture::Target backdropTarget{};
 bool resolvedBackdrop=false;
 std::vector<uint16_t>backdrop;
 std::vector<uint16_t>replay;nfs_soft::TextCommand command{};
 std::array<uint8_t,96>textBefore{};Measurement measurement{};unsigned resourceDepth=0,nameEntryDepth=0,recordQualificationDepth=0;
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
template<class F>inline void safe(Game&g,F&&f,const char* stage="collector-refused")noexcept{CpuErrno preserve(g);try{f();}catch(...){trace(stage);state().invalidate();}}
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
 explicit OwnerScope(Game&x,uint32_t expected,uint16_t typeExpected)noexcept:g(x){if(!enabled())return;if(uint32_t(g.eax)!=expected){trace("owner-address-refused",uint32_t(g.eax),expected);return;}safe(g,[&]{
  auto&s=state();if(s.owner){s.invalidate();return;}uint16_t type;{nfs::GuestCopyTransaction tx;type=tx.load<uint16_t>(uint32_t(g.eax)+8);}if(type!=typeExpected){trace("owner-type-refused",type,typeExpected);return;}
  static std::atomic<uint64_t>epochs{0};uint64_t epoch=epochs.fetch_add(1)+1;if(!epoch)throw 1;s=State{};s.game=&g;s.thread=threadToken();s.epoch=epoch;s.owner=1;s.form=expected;s.formType=type;entered=true;trace("owner-entered",expected,type);
 });}
 ~OwnerScope()noexcept{if(entered){CpuErrno preserve(g);state()=State{};}}
};
struct Adapter {
 static void resolveBackdrop(Game&g)noexcept{
  auto*s=current(g);if(!s||s->capturing||!s->backdrop.empty())return;
  CpuErrno preserve(g);
  try{
   NfsResultsBackdropImage artwork{};
   if(!nfs_results_backdrop_image(0,&artwork))return;
   const uint32_t source=uint32_t(g.eax);
   if(uint64_t(source)+16+Pixels*2>UINT64_C(0x100000000))return;
   nfs::GuestCopyTransaction tx;
   if(tx.load<uint32_t>(source)!=0x78||tx.load<uint32_t>(source+4)!=0x01e00280||
      tx.load<uint32_t>(source+8)||tx.load<uint32_t>(source+12))return;
   static constexpr unsigned char expected[32]={
    0xc7,0x4c,0x24,0x60,0x1c,0x9a,0xa0,0x33,0xd4,0x37,0x93,0x9f,0xde,0x97,0xeb,0xdb,
    0x2a,0x73,0xda,0xdd,0x27,0xab,0xa5,0x9d,0x1f,0xda,0x08,0x5d,0x58,0x4e,0xf9,0x14};
   unsigned char digest[32]{};
   if(nfs_texture_sha256(nfs_host_address(source+16,Pixels*2),Pixels*2,digest)&&
      !std::memcmp(digest,expected,sizeof digest)){
    s->resolvedBackdrop=true;
    trace("backdrop-source-authenticated");
   }
  }catch(...){}
 }
 static void observeBackdrop(Game&g)noexcept{
  auto*s=current(g);if(!s||s->capturing||!s->resolvedBackdrop)return;
  // The authenticated bitmap may be converted by the original blitter.
  // Use its rendered pixels as the baseline for the later overlay mask.
  CpuErrno preserve(g);
  s->resolvedBackdrop=false;
  try{
   const auto backdropTarget=target();
   auto candidate=plane(backdropTarget);
   s->backdropTarget=backdropTarget;
   s->backdrop=std::move(candidate);
   trace("backdrop-authenticated");
  }catch(...){}
 }
 static void begin(Game&g)noexcept{auto*s=ownerCurrent(g);if(!s)return;safe(g,[&]{
  // A refused frame must not poison this handler's later complete frames.
  // Retry only at its original full-frame boundary; every proof is recaptured.
  if(s->failed){const auto epoch=s->epoch,serial=s->serial;const auto form=s->form;const auto formType=s->formType;*s=State{};s->game=&g;s->thread=threadToken();s->epoch=epoch;s->serial=serial;s->owner=1;s->form=form;s->formType=formType;}
  if(s->capturing||s->string||s->word||s->serial==UINT64_MAX)throw 1;
  auto p=std::make_shared<Payload>();p->target=target();p->sourceGame=reinterpret_cast<uintptr_t>(&g);p->form=s->form;p->formType=s->formType;p->headingIndex=headingForType(p->formType);{nfs::GuestCopyTransaction tx;if(!p->headingIndex||tx.load<uint8_t>(byte_512DFB)!=0)throw 1;}p->epoch=s->epoch;p->serial=++s->serial;
  s->stores.clear();s->replay.clear();s->building=std::move(p);s->published.reset();s->metricAddress=s->fontAddress=0;s->capturing=true;
  trace("capture-began",s->formType);
 },"capture-begin-refused");}
 static void end(Game&g)noexcept{auto*s=current(g);if(!s||!s->capturing)return;safe(g,[&]{
  if(s->string||s->word||!s->building||s->building->commands.empty())throw 1;
  auto p=s->building;if(!nfs_logo_capture::same(p->target,target()))throw 1;p->original=plane(p->target);p->clean=p->original;
  for(auto i=s->stores.rbegin();i!=s->stores.rend();++i){if(p->clean[i->index]!=i->after)throw 1;p->clean[i->index]=i->before;}
  if(s->backdrop.size()==Pixels&&nfs_logo_capture::same(p->target,s->backdropTarget)){
   p->backdropMask.resize(Pixels);
   for(unsigned i=0;i<Pixels;++i)p->backdropMask[i]=p->clean[i]==s->backdrop[i]?255:0;
  }
  s->backdrop.clear();
  // Reverse only actual observed text stores. Retain late original nontext sprites; reject any conflicting overlap.
  for(size_t n=0;n<p->commands.size();++n){auto&c=p->commands[n];bool numeric=true;for(unsigned k=0;k<c.length;++k){unsigned v=c.text[k];if(v==9)continue;if(!((v>=48&&v<=57)||v==32||v==58||v==46||v==44||v==45||v==43||v==47))numeric=false;}
   unsigned red=(c.color>>11)&31,green=(c.color>>5)&63,blue=c.color&31;if(numeric&&red>12&&green>20&&blue<12)c.role=1;
   if(n&&c.role==1){auto&shadow=p->commands[n-1];if(shadow.color==0&&shadow.text==c.text&&shadow.x==c.x+1&&shadow.y==c.y+1)shadow.role=1;}
  }
  auto display=nfs_results_text::PreparedText::createCommands(p->commands,nfs_results_text::resultsFont(),p);if(!display)throw 1;
  auto f=std::shared_ptr<Frame>(new Frame);f->data_=p;f->display_=std::move(display);s->published=std::move(f);s->building.reset();s->replay.clear();s->capturing=false;
  trace("frame-published",static_cast<uint32_t>(p->commands.size()),!p->backdropMask.empty());
 },"frame-build-refused");}
 static std::shared_ptr<const Frame>acquire(Game&g)noexcept{auto*s=current(g);if(!s||s->presenting!=1||s->capturing||s->string||s->word||!s->published)return {};std::shared_ptr<const Frame>out;safe(g,[&]{
  auto f=s->published;const auto&d=f->data();if(d.epoch!=s->epoch||!nfs_logo_capture::same(d.target,target())||d.original!=plane(d.target))throw 1;out=std::move(f);
 },"frame-reacquire-refused");return out;}
 static bool reacquire(Game&g,const std::shared_ptr<const Frame>&f)noexcept{auto now=acquire(g);return now&&now.get()==f.get()&&!now.owner_before(f)&&!f.owner_before(now);}
 // 413914 -> 401F40 is the original record-name prompt, after a complete
 // Results frame. Reuse only that exact source frame, never current GPU pixels.
 static std::shared_ptr<const Frame>nameEntryParent(Game&g)noexcept{
  auto*s=current(g);if(!s||s->formType!=3||s->form!=uint32_t(guest_address(off_4D3D90))||s->nameEntryDepth!=1||s->recordQualificationDepth!=1||s->capturing||s->presenting||s->string||s->word||!s->published)return {};
  std::shared_ptr<const Frame>out;safe(g,[&]{auto f=s->published;const auto&d=f->data();{nfs::GuestCopyTransaction tx;if(tx.load<uint16_t>(d.form+8)!=3)throw 1;}
   if(d.epoch!=s->epoch||d.form!=s->form||d.formType!=3||d.sourceGame!=reinterpret_cast<uintptr_t>(&g)||!nfs_logo_capture::same(d.target,target())||d.original!=plane(d.target))throw 1;
   out=std::move(f);
  });return out;
 }

};
struct RecordQualificationScope {
 Game&g;State*s=nullptr;uint64_t epoch=0;bool entered=false;
 explicit RecordQualificationScope(Game&x)noexcept:g(x){s=current(g);if(!s||s->formType!=3||s->capturing||s->presenting||s->string||s->word||!s->published||s->recordQualificationDepth)return;epoch=s->epoch;++s->recordQualificationDepth;entered=true;trace("record-qualification-entered");if(std::getenv("NFS_PRIVATE_RESULTS_TRACE")){
   try{
    nfs::GuestCopyTransaction memory;
    trace("record-qualification-mode",memory.load<uint32_t>(dword_4E5508));
    const uint32_t rules=uint32_t(memory.load<uint8_t>(byte_512DAA))
     |(uint32_t(memory.load<uint8_t>(byte_512DD0))<<8)
     |(uint32_t(memory.load<uint8_t>(byte_512DEB))<<16);
    trace("record-qualification-rules",rules);
   }catch(...){trace("record-qualification-state-unavailable");}
  }}
 ~RecordQualificationScope()noexcept{if(entered&&s==ownerCurrent(g)&&s->epoch==epoch&&s->recordQualificationDepth){trace("record-qualification-returned",uint16_t(g.eax));--s->recordQualificationDepth;}}
};
struct NameEntryCallerScope {
 Game&g;State*s=nullptr;uint64_t epoch=0;bool entered=false;
 explicit NameEntryCallerScope(Game&x)noexcept:g(x){s=current(g);if(!s||s->formType!=3||s->capturing||s->presenting||s->string||s->word||s->recordQualificationDepth!=1||s->nameEntryDepth)return;epoch=s->epoch;++s->nameEntryDepth;entered=true;trace("record-name-caller-entered");}
 ~NameEntryCallerScope()noexcept{if(entered&&s==ownerCurrent(g)&&s->epoch==epoch&&s->nameEntryDepth)--s->nameEntryDepth;}
};
struct ResourceScope {
 Game&g;State*s=nullptr;Measurement old{};bool entered=false;
 explicit ResourceScope(Game&x)noexcept:g(x){s=current(g);if(!s||!s->capturing)return;old=s->measurement;s->measurement={};++s->resourceDepth;entered=true;}
 ~ResourceScope()noexcept{if(entered&&s&&s==current(g)){s->measurement=old;if(s->resourceDepth)--s->resourceDepth;}}
};
inline void measured(Game&g,uint32_t text,int width,unsigned alignment)noexcept{
 auto*s=current(g);if(!s||!s->capturing||!s->resourceDepth||s->string)return;
 safe(g,[&]{if(width<=0||width>4096||(alignment!=1&&alignment!=2)){trace("measurement-invalid",uint32_t(width),alignment);throw 1;}s->measurement={true,text,width,int(g.esi),alignment};});
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
  size&=255;if(callback!=uint32_t(game_id_sub_4A5830)||(size!=13&&size!=14&&size!=16&&size!=18&&size!=24&&size!=36)){trace("string-font-or-callback",size,callback);throw 1;}metricTable(*s,font,table,count);
  // The software font interpreter uses TAB to advance to a configured column.
  // Preserve the entire original draw until the native layout models those stops.
  if(std::find(s->textBefore.begin(),s->textBefore.begin()+c.length,9)!=s->textBefore.begin()+c.length)return;
  int width=0;for(unsigned k=0;k<c.length;++k){unsigned code=s->textBefore[k];c.text[k]=uint8_t(code);if(!s->known[code]||nfs_soft::index(uint8_t(code))<0){trace("string-glyph-refused",0,!s->known[code]);throw 1;}width+=s->advances[code];}
  if(width<0||width>4096)throw 1;c.sourceSize=size;c.x=g.edx;c.y=g.ebx;c.advance=width;c.clip={0,0,640,480};c.alignment=0;c.role=0;
  const auto&m=s->measurement;if(m.ready&&m.text==s->textAddress){int expected=m.anchor-(m.alignment==2?m.width:m.width/2);if(c.x==expected||c.x==expected+1){c.alignment=m.alignment;c.advance=m.width;}}
  s->string=true;entered=true;
 },"string-begin-refused");}
 ~StringScope()noexcept{if(!entered||!s||s!=current(g))return;safe(g,[&]{
  if(!s->string||s->word||!s->building)throw 1;
  {nfs::GuestCopyTransaction tx;for(unsigned k=0;k<=s->command.length;++k)if(tx.load<uint8_t>(s->textAddress+k)!=s->textBefore[k])throw 1;}
  s->building->commands.push_back(s->command);s->string=false;
 },"string-end-refused");}
};
struct Word {
 Game&g;State*s=nullptr;uint32_t address=0,index=0;uint16_t before=0,value=0;bool entered=false;
 Word(Game&x,uint32_t at,uint16_t computed)noexcept:g(x){s=current(g);if(!s||!s->capturing||!s->string)return;safe(g,[&]{
  if(s->word||!s->building)throw 1;uint64_t first=uint64_t(s->building->target.w[8])+16;if(at<first||uint64_t(at)+2>first+Pixels*2||(at-first)%2)throw 1;address=at;index=unsigned((at-first)/2);value=computed;
  {nfs::GuestCopyTransaction tx;before=tx.load<uint16_t>(at);}if(s->stores.size()>=1000000)throw 1;s->word=true;entered=true;
 },"pixel-store-begin-refused");}
 ~Word()noexcept{if(!entered||!s||s!=current(g))return;safe(g,[&]{uint16_t after;{nfs::GuestCopyTransaction tx;after=tx.load<uint16_t>(address);}if(after!=value)throw 1;s->stores.push_back({index,before,after});s->word=false;},"pixel-store-end-refused");}
};
struct Presentation {
 Game&g;State*s=nullptr;bool entered=false;
 explicit Presentation(Game&x)noexcept:g(x){s=current(g);if(!s||!s->published)return;safe(g,[&]{if(s->presenting||s->capturing)throw 1;s->presenting=1;entered=true;trace("presentation-entered",s->formType);});}
 ~Presentation()noexcept{if(entered&&s&&s==current(g))s->presenting=0;}
};
inline void retire(Game&g,uint32_t owner)noexcept{auto*s=current(g);if(!s||!s->published)return;CpuErrno preserve(g);const auto&t=s->published->data().target;if(owner==t.owner||owner==t.w[8]||owner==t.w[11]||owner==t.w[12])s->invalidate();}
} // namespace nfs_results_ui_live
