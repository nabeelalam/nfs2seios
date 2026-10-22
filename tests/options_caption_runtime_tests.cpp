// SPDX-License-Identifier: MIT
#include <array>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <thread>
#include <type_traits>
#include "hd/OriginalUiRecords.h"
static std::array<uint8_t,2097152> memory{};static unsigned leaseDepth=0,reads=0;static uint32_t deniedRead=0;static std::atomic<unsigned> allocationsInLease{0},foreignAllocations{0};static thread_local bool foreignPath=false;
void*operator new(size_t n){if(leaseDepth)++allocationsInLease;if(foreignPath)++foreignAllocations;if(void*p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}
void operator delete(void*p,size_t)noexcept{std::free(p);}
void*operator new(size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}
void operator delete(void*p,const std::nothrow_t&)noexcept{std::free(p);}
struct Game{int32_t eax=0,edx=0,ebx=0,ecx=0,ebp=0,esi=0;};
static constexpr uint32_t off_4D3A88=0x100,dword_4D40B8=0x200,dword_4DD770=0x208,dword_4DD77C=0x210,dword_4DD774=0x218,dword_4DD714=0x220,dword_4D20A0=0x228,byte_4DB25E=0x230,byte_4DB25C=0x238,dword_4DD6F4=0x240,dword_4DD6F8=0x248,dword_4E54AC=0x250,dword_4DD6EC=0x258,dword_4DB240=0x1000,dword_4D243C=0x260,byte_4D3ECC=0x268,off_4D3ED0=0x280,dword_4D3E9C=0x2c0;
static constexpr uint32_t off_4D2790=0x110,dword_4D3E24=0x2d0;
static constexpr uint32_t off_4D3050=0x120,off_4D3100=0x130,dword_4D3E4C=0x300,dword_4D3E60=0x320;
#if defined(NFS_TEST_PLAYER1_CAPTION)
static constexpr uint32_t testOwner=off_4D3050,testRecord=dword_4D3E4C;
static constexpr unsigned testSelector=2,testResource=2,testWidth=68,testLeft=100,testY=198;
static constexpr char testText[]="player one";
#elif defined(NFS_TEST_PLAYER2_CAPTION)
static constexpr uint32_t testOwner=off_4D3100,testRecord=dword_4D3E60;
static constexpr unsigned testSelector=3,testResource=3,testWidth=70,testLeft=98,testY=234;
static constexpr char testText[]="player two";
#elif defined(NFS_TEST_SETUP_CAPTION)
static constexpr uint32_t testOwner=off_4D2790,testRecord=dword_4D3E24;
static constexpr unsigned testSelector=0,testResource=0,testWidth=75,testLeft=93,testY=126;
static constexpr char testText[]="game setup";
#else
static constexpr uint32_t testOwner=off_4D3A88,testRecord=dword_4D3E9C;
static constexpr unsigned testSelector=6,testResource=7,testWidth=48,testLeft=120,testY=364;
static constexpr char testText[]="options";
#endif
static constexpr uint32_t game_id_sub_4A5830=0xface0010;
inline uint32_t guest_address(uint32_t p){return p;}
inline void*nfs_host_address(uint32_t p,size_t n){if(!leaseDepth||uint64_t(p)+n>memory.size()||(deniedRead&&uint64_t(p)<=deniedRead&&uint64_t(p)+n>deniedRead))throw 1;++reads;return memory.data()+p;}
namespace nfs{struct GuestCopyTransaction{GuestCopyTransaction(){++leaseDepth;}~GuestCopyTransaction(){--leaseDepth;}template<class T>T load(uint32_t p){T v;std::memcpy(&v,nfs_host_address(p,sizeof(T)),sizeof(T));return v;}};}
static nfs_logo_capture::Target selected{};static bool feature=true;
namespace nfs_logo_publisher{inline bool enabled(){return feature;}inline int exceptions(){return std::uncaught_exception()?1:0;}inline void checked(uint32_t p,size_t n){(void)nfs_host_address(p,n);}inline void target(nfs::GuestCopyTransaction&,nfs_logo_capture::Target&t){++reads;t=selected;}}
#define NFS_OPTIONS_CAPTION_TEXT_TEST 1
#define NFS_SETUP_CAPTION_TEXT_TEST 1
#define NFS_PLAYER1_CAPTION_TEXT_TEST 1
#define NFS_PLAYER2_CAPTION_TEXT_TEST 1
#include "hd/OptionsCaptionGameHooks.inc"
#include "hd/SetupCaptionGameHooks.inc"
#include "hd/Player1CaptionGameHooks.inc"
#include "hd/Player2CaptionGameHooks.inc"
#ifdef NFS_CACHED_MENU_CAPTION_NAMESPACE
#error "Caption wrapper leaked its namespace macro"
#endif
#if defined(NFS_TEST_PLAYER1_CAPTION)
namespace cap=nfs_player1_caption;
#elif defined(NFS_TEST_PLAYER2_CAPTION)
namespace cap=nfs_player2_caption;
#elif defined(NFS_TEST_SETUP_CAPTION)
namespace cap=nfs_setup_caption;
#else
namespace cap=nfs_options_caption;
#endif
template<class T>static void put(uint32_t p,T v){assert(uint64_t(p)+sizeof v<=memory.size());std::memcpy(memory.data()+p,&v,sizeof v);}
static void prepare(){deniedRead=0;memory.fill(0);put<uint32_t>(dword_4D243C,0xdeadbeef);put<uint32_t>(byte_4D3ECC,testSelector<<16);put<uint32_t>(off_4D3ED0+testSelector*4,testRecord);put<uint32_t>(testRecord,(testResource<<16)|2);selected={};auto&w=selected.w;w[1]=343;w[2]=480;w[5]=343;w[6]=480;w[7]=0x7810;w[8]=0x40000;w[10]=686;w[11]=0x2000;w[12]=0x3000;selected.owner=0x1800;selected.ownerWords=w;selected.header={{0x78,343|(480u<<16),0,0}};for(unsigned k=0;k<343;++k)selected.x[k]=2*k;for(unsigned k=0;k<480;++k)selected.y[k]=16+686*k;for(unsigned k=0;k<4;++k)put(w[8]+4*k,selected.header[k]);put<uint32_t>(dword_4D40B8,0x1234);put<uint32_t>(dword_4DD770,0x98000);put<uint32_t>(dword_4DD77C,game_id_sub_4A5830);put<uint32_t>(dword_4DD774,0x99000);put<uint32_t>(dword_4DD714,0xffff);put<uint8_t>(byte_4DB25C,16);put<int32_t>(dword_4E54AC,18);put<int32_t>(dword_4DD6EC,116);std::memcpy(memory.data()+0x94000,testText,sizeof testText);for(unsigned k=0;k<16;++k)put<uint32_t>(0x99000+4*k,0xff000000u+k);}
static std::shared_ptr<const nfs_soft::FontCatalog>font(){nfs_soft::FontDescription d;for(auto&role:d.glyphs)for(auto&g:role){g.x=g.y=2;g.w=g.h=1;g.advance=65536;}std::array<uint8_t,1024*512>a{};a[2*1024+2]=255;return nfs_soft::FontCatalog::create(d,a.data(),a.size(),17);}
static void stringBody(Game&g,const char*mode){g.eax=0x94000;g.esi=!std::strcmp(mode,"measure-anchor")?169:168;g.edx=!std::strcmp(mode,"wrong-position")?testLeft+1:testLeft;g.ebx=testY;if(std::strcmp(mode,"missing-measure")){if(!std::strcmp(mode,"measure-color"))put<uint32_t>(dword_4DD714,0x1234);cap::measured(g,!std::strcmp(mode,"measure-pointer")?0x94001:0x94000,!std::strcmp(mode,"measure-zero")?0:(!std::strcmp(mode,"measure-width")?testWidth+1:(!std::strcmp(mode,"center")?testWidth*2:testWidth)),!std::strcmp(mode,"center")?cap::SourceMeasureAlignment::center:cap::SourceMeasureAlignment::right);if(!std::strcmp(mode,"measure-duplicate"))cap::measured(g,0x94000,testWidth,cap::SourceMeasureAlignment::right);if(!std::strcmp(mode,"measure-font"))put<uint8_t>(0x98000,1);if(!std::strcmp(mode,"measure-color"))put<uint32_t>(dword_4DD714,0xffff);}cap::StringScope text(g);{cap::GlyphScope glyph(g);const uint32_t at=selected.w[8]+16+369*686+123*2;{cap::WordCapture word(g,at,0xffff);put<uint16_t>(at,!std::strcmp(mode,"bad-word")?0x1234:0xffff);}if(!std::strcmp(mode,"extra-write"))put<uint16_t>(at+2,1);}
 if(!std::strcmp(mode,"palette"))put<uint32_t>(0x99004,123);if(!std::strcmp(mode,"font"))put<uint32_t>(dword_4DD770,0x97000);if(!std::strcmp(mode,"font-header"))put<uint8_t>(0x98000,1);if(!std::strcmp(mode,"proof-text-during"))memory[0x94000]^=1;put<int32_t>(dword_4DD6EC,g.edx+testWidth);}
static void trackedString(Game&g,const char*mode){
 g.eax=!std::strcmp(mode,"proof-resource")?testResource+1:testResource;g.edx=!std::strcmp(mode,"proof-anchor")?169:168;g.ebx=testY;g.ecx=!std::strcmp(mode,"proof-category")?0:1;
 cap::ResourceScope resource(g);
 if(!std::strcmp(mode,"proof-nested")){cap::ResourceScope nested(g);}
 if(std::strcmp(mode,"proof-no-resolved")){uint32_t text=0x94000;if(!std::strcmp(mode,"proof-resolved-pointer")){std::memcpy(memory.data()+0x94100,testText,sizeof testText);text=0x94100;}g.eax=int32_t(text);cap::resolvedResource(g,text);if(!std::strcmp(mode,"proof-resolved-duplicate"))cap::resolvedResource(g,text);}
 if(!std::strcmp(mode,"proof-resolved-font"))put<uint8_t>(0x98000,1);
 stringBody(g,mode);
 if(!std::strcmp(mode,"proof-text-after"))memory[0x94000]^=1;
 if(!std::strcmp(mode,"proof-unwind"))throw 7;
}
static void sealedProofCases(){
 using Proof=nfs_compact_caption::Proof;using Evidence=nfs_compact_caption::Evidence;
 static_assert(!std::is_default_constructible<Proof>::value,"proof cannot be forged by default construction");
 static_assert(!std::is_constructible<Proof,std::shared_ptr<const nfs_soft::FramePacket>,const Evidence&>::value,"public observations cannot seal a proof");
 if(testSelector!=2&&testSelector!=3)return;
 std::shared_ptr<const Proof>retained;std::shared_ptr<const nfs_soft::FontCatalog>retainedFont;
 for(const char*mode:{"normal","measure-color","proof-resource","proof-anchor","proof-category","proof-no-resolved","proof-resolved-pointer","proof-resolved-duplicate","proof-resolved-font","proof-text-during","proof-text-after","proof-nested","proof-unwind"}){
  prepare();Game g;g.eax=testOwner;g.ebp=testOwner;
  {cap::OwnerScope owner(g);auto*state=cap::current(g);assert(state);try{cap::TailScope tail(g);trackedString(g,mode);}catch(int){assert(!std::strcmp(mode,"proof-unwind"));}
   auto proof=cap::lastProof(g);const bool okay=!std::strcmp(mode,"normal")||!std::strcmp(mode,"measure-color");assert(bool(proof)==okay);
   // Supplemental observation failures cannot fabricate metadata or alter old
   // packet admission. Unwinding remains the existing packet rejection path.
   if(std::strcmp(mode,"proof-unwind")){if(!cap::lastPacket(g))std::fprintf(stderr,"legacy packet rejected in %s: %s\n",mode,cap::failureName(state->failure));assert(cap::lastPacket(g));}
   if(proof){const auto&e=proof->evidence();assert(e.policy==(testSelector==2?nfs_compact_caption::Policy::Player1:nfs_compact_caption::Policy::Player2));assert(e.route==nfs_compact_caption::Route::OrdinaryEbp&&e.formGuest==testOwner&&e.incomingEbp==testOwner&&e.game==reinterpret_cast<uintptr_t>(&g)&&e.policyThread==cap::token());assert(e.lifetime==state->lifetime&&e.serial==state->serial&&e.resourceSerial);assert(e.requestedResource==testResource&&e.inputCategory==1&&e.inputAnchor==168&&e.inputY==testY&&e.textGuest==0x94000);assert(e.measureWidth==int(testWidth)&&e.measureAnchor==168&&e.measureKind==2);assert(e.command.length==sizeof(testText)-1&&!std::memcmp(e.command.text.data(),testText,sizeof(testText)-1));assert(e.command.x==int(testLeft)&&e.command.y==int(testY)&&e.command.advance==int(testWidth)&&e.command.role==1&&e.command.alignment==2&&e.command.sourceSize==18&&e.command.color==0xffff);assert(e.resolved.selector==testSelector&&e.resolved.entry==testRecord&&e.resolved.resource==testResource&&e.resolved.entryWord==((testResource<<16)|2));assert(e.resolved.font==0x98000&&e.measure.font==0x98000&&e.stringBefore.font==0x98000&&e.stringAfter.font==0x98000);assert(e.stringBefore.paletteWords==e.stringAfter.paletteWords&&e.stringBefore.fontHeader==e.stringAfter.fontHeader);assert(proof->font()==state->font&&proof->packet()==cap::lastPacket(g));assert(cap::acquireCurrentProof(g)==proof);
    retained=proof;retainedFont=proof->font();put<uint16_t>(selected.w[8]+16,1);assert(!cap::acquireCurrentProof(g));assert(proof->packet()->original()[0]==0);
   }
  }assert(!cap::current(g)&&!cap::lastProof(g));
 }
 assert(retained&&retainedFont&&retained->font()==retainedFont&&retained->packet()->original()[369*343+123]==0xffff);
 prepare();Game g;g.eax=testOwner;g.ebp=testOwner;{cap::OwnerScope owner(g);{cap::TailScope tail(g);stringBody(g,"normal");}assert(cap::lastPacket(g)&&!cap::lastProof(g)&&!cap::acquireCurrentProof(g));}
 prepare();g.eax=testOwner;{cap::OwnerScope owner(g);{cap::TailScope tail(g);trackedString(g,"normal");}auto proof=cap::acquireCurrentProof(g);assert(proof);const auto before=reads;std::thread other([&]{foreignPath=true;Game foreign;foreign.eax=testOwner;assert(!cap::lastProof(foreign)&&!cap::acquireCurrentProof(foreign));foreignPath=false;});other.join();assert(reads==before&&foreignAllocations==0);cap::beforeOwnerRetire(g,selected.owner);assert(!cap::lastProof(g)&&!cap::acquireCurrentProof(g));assert(proof->packet()->original()[369*343+123]==0xffff);}
}
static void descriptorRetirement(){
 for(const char*mode:{"shared-x","shared-y","shared-both","final-x","final-y","zero-x","zero-y","negative-x","negative-y","read-x","read-y","bitmap","owner","unrelated","rebind-x"}){
  prepare();Game g;g.eax=testOwner;g.ebp=testOwner;cap::OwnerScope owner(g);auto*s=cap::current(g);assert(s);{cap::TailScope tail(g);stringBody(g,"normal");}auto packet=cap::lastPacket(g);assert(packet);const auto lifetime=s->lifetime,serial=s->serial;const auto originalTarget=selected;const auto currentIdentity=s->current.identity;
  constexpr uint32_t descriptor=0xa000;const bool x=std::strstr(mode,"-x")||!std::strcmp(mode,"shared-both"),y=std::strstr(mode,"-y")||!std::strcmp(mode,"shared-both");
  put<uint32_t>(descriptor+32,!std::strcmp(mode,"bitmap")?selected.w[8]:0xb000);put<uint32_t>(descriptor+44,y?selected.w[11]:0xc000);put<uint32_t>(descriptor+48,x?selected.w[12]:0xd000);
  put<uint32_t>(selected.w[11]-4,2);put<uint32_t>(selected.w[12]-4,2);
  if(!std::strncmp(mode,"final-",6))put<uint32_t>((x?selected.w[12]:selected.w[11])-4,1);
  if(!std::strncmp(mode,"zero-",5))put<uint32_t>((x?selected.w[12]:selected.w[11])-4,0);
  if(!std::strncmp(mode,"negative-",9))put<uint32_t>((x?selected.w[12]:selected.w[11])-4,UINT32_MAX);
  if(!std::strncmp(mode,"read-",5))deniedRead=(x?selected.w[12]:selected.w[11])-4;
  if(!std::strcmp(mode,"rebind-x"))cap::beforeOwnerRetire(g,descriptor);else cap::beforeDescriptorRetire(g,!std::strcmp(mode,"owner")?selected.owner:descriptor);
  deniedRead=0;const bool survives=!std::strncmp(mode,"shared-",7)||!std::strcmp(mode,"unrelated");
  if(survives){assert(cap::lastPacket(g)==packet&&s->lifetime==lifetime&&s->serial==serial&&s->current.identity==currentIdentity);if(x)put<uint32_t>(selected.w[12]-4,1);if(y)put<uint32_t>(selected.w[11]-4,1);assert(nfs_logo_capture::same(selected,originalTarget));assert(cap::acquireCurrentPacket(g)==packet);cap::beforeDescriptorRetire(g,0);assert(cap::lastPacket(g)==packet&&s->lifetime==lifetime);}
  else assert(!cap::lastPacket(g)&&s->lifetime>lifetime);
  assert(packet->original()[369*343+123]==0xffff&&packet->clean()[369*343+123]==0&&packet->pixels()==343*480);assert(!leaseDepth&&allocationsInLease==0);
 }
}
static void policyIsolation(){
 void*tls[]={&nfs_options_caption::ownerThread(),&nfs_setup_caption::ownerThread(),&nfs_player1_caption::ownerThread(),&nfs_player2_caption::ownerThread()};
 for(unsigned i=0;i<4;++i)for(unsigned j=i+1;j<4;++j)assert(tls[i]!=tls[j]);
 Game options,setup,player1,player2;options.eax=off_4D3A88;setup.eax=off_4D2790;player1.eax=off_4D3050;player2.eax=off_4D3100;
 nfs_options_caption::OwnerScope first(options);auto*old=nfs_options_caption::current(options);assert(old);
 {nfs_setup_caption::OwnerScope second(setup);auto*other=nfs_setup_caption::current(setup);assert(other&&static_cast<void*>(other)!=static_cast<void*>(old));
  {nfs_player1_caption::OwnerScope third(player1);auto*one=nfs_player1_caption::current(player1);assert(one&&static_cast<void*>(one)!=static_cast<void*>(old)&&static_cast<void*>(one)!=static_cast<void*>(other));
   {nfs_player2_caption::OwnerScope fourth(player2);auto*two=nfs_player2_caption::current(player2);assert(two&&static_cast<void*>(two)!=static_cast<void*>(one)&&static_cast<void*>(two)!=static_cast<void*>(old)&&static_cast<void*>(two)!=static_cast<void*>(other));
    assert(nfs_options_caption::current(options)==old&&nfs_setup_caption::current(setup)==other&&nfs_player1_caption::current(player1)==one);
    assert(!nfs_options_caption::current(player1)&&!nfs_options_caption::current(player2)&&!nfs_setup_caption::current(player1)&&!nfs_setup_caption::current(player2));
    assert(!nfs_player1_caption::current(options)&&!nfs_player1_caption::current(setup)&&!nfs_player1_caption::current(player2));
    assert(!nfs_player2_caption::current(options)&&!nfs_player2_caption::current(setup)&&!nfs_player2_caption::current(player1));
   }assert(!nfs_player2_caption::current(player2)&&nfs_player1_caption::current(player1)==one);
  }assert(!nfs_player1_caption::current(player1)&&nfs_setup_caption::current(setup)==other);
 }assert(nfs_options_caption::current(options)==old&&!nfs_setup_caption::current(setup));
}
int main(){prepare();Game g;g.eax=testOwner;g.ebp=testOwner;{cap::OwnerScope nofont(g);assert(!cap::current(g));}assert(nfs_soft::installFontCatalog(font()));policyIsolation();feature=false;auto priorReads=reads;{cap::OwnerScope disabled(g);}assert(reads==priorReads);feature=true;g.eax=0x4d3348;{cap::OwnerScope wrong(g);assert(!cap::current(g));auto before=reads;cap::TailScope outside(g);assert(reads==before);}
 std::shared_ptr<const nfs_soft::FramePacket> retained;
 for(const char*mode:{"normal","right","center","measure-anchor","missing-measure","measure-pointer","measure-zero","measure-width","measure-duplicate","measure-font","measure-color","bad-word","extra-write","palette","font","font-header","wrong-mode","wrong-callback","wrong-size","unknown-text","unrelated-global","wrong-tail","ordinal-seven","wrong-resource","aliased-resource","wrong-category","wrong-color","wrong-position","lut","y-lut","header","selection","nested","retire","unrelated"}){prepare();g.eax=testOwner;g.ebp=testOwner;auto original=g;cap::OwnerScope owner(g);auto*s=cap::current(g);assert(s&&s->font);assert(!std::memcmp(&g,&original,sizeof g));if(!std::strcmp(mode,"wrong-mode"))put<uint32_t>(dword_4D20A0,1);if(!std::strcmp(mode,"wrong-callback"))put<uint32_t>(dword_4DD77C,123);if(!std::strcmp(mode,"wrong-size"))put<int32_t>(dword_4E54AC,17);if(!std::strcmp(mode,"unknown-text"))memory[0x94000]=uint8_t(88);if(!std::strcmp(mode,"lut"))selected.x[100]^=2;if(!std::strcmp(mode,"y-lut"))selected.y[250]^=2;if(!std::strcmp(mode,"header"))selected.header[1]^=1;if(!std::strcmp(mode,"unrelated-global"))put<uint32_t>(dword_4D243C,123);if(!std::strcmp(mode,"wrong-tail"))g.ebp=123;if(!std::strcmp(mode,"ordinal-seven"))put<uint32_t>(byte_4D3ECC,((testSelector+1)%8)<<16);if(!std::strcmp(mode,"wrong-resource"))put<uint32_t>(testRecord,((testResource+1)<<16)|2);if(!std::strcmp(mode,"aliased-resource")){put<uint32_t>(off_4D3ED0+testSelector*4,0x2e0);put<uint32_t>(0x2e0,(testResource<<16)|2);}if(!std::strcmp(mode,"wrong-category"))put<uint32_t>(byte_4D3ECC,1u<<16);if(!std::strcmp(mode,"wrong-color"))put<uint32_t>(dword_4DD714,123);
  {cap::TailScope tail(g);if(!std::strcmp(mode,"nested")){cap::TailScope nested(g);}else{if(!std::strcmp(mode,"selection")){cap::SelectionScope change(g);selected.w[3]=1;}stringBody(g,mode);}}
  bool okay=!std::strcmp(mode,"right")||!std::strcmp(mode,"measure-color")||!std::strcmp(mode,"unrelated-global")||!std::strcmp(mode,"normal")||!std::strcmp(mode,"unrelated")||!std::strcmp(mode,"retire");auto packet=cap::lastPacket(g);if(!std::strcmp(mode,"normal"))assert(s->diagnosticStage==4&&s->diagnosticCenter==168&&s->diagnosticAdvance==testWidth&&s->diagnosticFontDiff==0&&s->diagnosticTailCaptured);if(!std::strcmp(mode,"wrong-position"))assert(s->diagnosticStage==1&&s->failure==cap::Failure::unsupportedText);if(!std::strcmp(mode,"palette"))assert(s->diagnosticFontDiff&(1u<<9));if(!std::strcmp(mode,"bad-word"))assert(s->diagnosticBuilder==uint32_t(nfs_soft::FramePacketBuilder::Failure::wordMismatch));if(bool(packet)!=okay)std::fprintf(stderr,"mode=%s failure=%s\n",mode,cap::failureName(s->failure));assert(bool(packet)==okay);
  if(packet){assert(!cap::lastProof(g));assert(s->measureSeen&&s->measureUsed&&s->diagnosticMeasurePointerMatch&&s->measureAnchor==168);if(!std::strcmp(mode,"right"))assert(s->measureKind==cap::SourceMeasureAlignment::right&&s->measureWidth==testWidth&&s->diagnosticAdvance==testWidth&&s->diagnosticCenter==168);assert(packet->pixels()==343*480&&packet->surface().geometry[10]==686&&packet->commands()==1&&packet->writes()==1);assert(packet->original()[369*343+123]==0xffff&&packet->clean()[369*343+123]==0);assert(cap::acquireCurrentPacket(g)==packet);
   if(!std::strcmp(mode,"retire")){cap::beforeOwnerRetire(g,selected.owner);assert(!cap::lastPacket(g));}else if(!std::strcmp(mode,"unrelated")){cap::beforeOwnerRetire(g,0);cap::beforeOwnerRetire(g,0x9a000);assert(cap::lastPacket(g)==packet);}else{retained=packet;put<uint16_t>(selected.w[8]+16,1);assert(!cap::acquireCurrentPacket(g));assert(packet->original()[0]==0);}
  }else assert(s->failure!=cap::Failure::none);
  if(!std::strcmp(mode,"normal")){std::thread foreign([&]{foreignPath=true;Game other;other.eax=testOwner;{cap::OwnerScope foreignOwner(other);assert(!cap::current(other));}foreignPath=false;});foreign.join();assert(foreignAllocations==0);}
 }
 prepare();g.eax=testOwner;{cap::OwnerScope owner(g);{cap::TailScope tail(g);g.eax=testOwner;{cap::OwnerScope nested(g);assert(!cap::current(g));}}assert(!cap::lastPacket(g));}
 prepare();g.eax=testOwner;{cap::OwnerScope owner(g);{cap::TailScope tail(g);stringBody(g,"normal");}auto packet=cap::lastPacket(g);assert(packet);put<uint32_t>(dword_4D243C,123);assert(cap::acquireCurrentPacket(g)==packet);put<uint32_t>(testRecord,((testResource+1)<<16)|2);assert(!cap::acquireCurrentPacket(g));assert(packet->original()[369*343+123]==0xffff);}
 descriptorRetirement();sealedProofCases();
 assert(retained&&retained->pixels()==343*480&&retained->original()[369*343+123]==0xffff);assert(!leaseDepth&&allocationsInLease==0);std::puts("PASS cached menu caption (Options/Setup/Player1/Player2): compact343 full replay/ownership/current-plane rejection, exact owner/font gate, original words, palette/font/mode/LUT/selection/retirement/nesting fallback, shared-LUT descriptor refs/lifetime, immutable lifetime, zero allocations in guest leases");
}
