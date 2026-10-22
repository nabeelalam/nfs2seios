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
#include "Main343OriginFixture.h"
#include "hd/PlayerInitialSidebarProof.h"
#include "PlayerSceneFixture.h"
#include "hd/PlayerNativeUploadWitness.h"
static std::array<uint8_t,2097152> memory{};static unsigned leaseDepth=0,reads=0;static uint32_t deniedRead=0;static std::atomic<unsigned> allocationsInLease{0},foreignAllocations{0};static thread_local bool foreignPath=false;
void*operator new(size_t n){if(leaseDepth)++allocationsInLease;if(foreignPath)++foreignAllocations;if(void*p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}
void operator delete(void*p,size_t)noexcept{std::free(p);}
void*operator new(size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}
void operator delete(void*p,const std::nothrow_t&)noexcept{std::free(p);}
struct Game{int32_t eax=0,edx=0,ebx=0,ecx=0,ebp=0,esi=0;};
static constexpr uint32_t off_4D3A88=0x100,dword_4D40B8=0x200,dword_4DD770=0x208,dword_4DD77C=0x210,dword_4DD774=0x218,dword_4DD714=0x220,dword_4D20A0=0x228,byte_4DB25E=0x230,byte_4DB25C=0x238,dword_4DD6F4=0x240,dword_4DD6F8=0x248,dword_4E54AC=0x250,dword_4DD6EC=0x258,dword_4DB240=0x1000,dword_4D243C=0x260,byte_4D3ECC=0x268,off_4D3ED0=0x280,dword_4D3E9C=0x2c0;
static constexpr uint32_t off_4D2790=0x110,dword_4D3E24=0x2d0;
static constexpr uint32_t off_4D3EC4=0x140,dword_4D2314=0x340,dword_4D231C=0x350,dword_4D2320=0x354;
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
namespace nfs_logo_publisher{inline uintptr_t threadToken(){return 29;}inline bool enabled(){return feature;}inline int exceptions(){return std::uncaught_exception()?1:0;}inline void checked(uint32_t p,size_t n){(void)nfs_host_address(p,n);}inline void target(nfs::GuestCopyTransaction&,nfs_logo_capture::Target&t){++reads;t=selected;}}
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
static void stringBody(Game&g,const char*mode){g.eax=0x94000;g.esi=!std::strcmp(mode,"measure-anchor")?169:168;g.edx=!std::strcmp(mode,"wrong-position")?testLeft+1:testLeft;g.ebx=testY;if(std::strcmp(mode,"missing-measure")){if(!std::strcmp(mode,"measure-color"))put<uint32_t>(dword_4DD714,0x1234);cap::measured(g,!std::strcmp(mode,"measure-pointer")?0x94001:0x94000,!std::strcmp(mode,"measure-zero")?0:(!std::strcmp(mode,"measure-width")?testWidth+1:(!std::strcmp(mode,"center")?testWidth*2:testWidth)),!std::strcmp(mode,"center")?cap::SourceMeasureAlignment::center:cap::SourceMeasureAlignment::right);if(!std::strcmp(mode,"measure-duplicate"))cap::measured(g,0x94000,testWidth,cap::SourceMeasureAlignment::right);if(!std::strcmp(mode,"measure-font"))put<uint8_t>(0x98000,1);if(!std::strcmp(mode,"measure-color"))put<uint32_t>(dword_4DD714,0xffff);}cap::StringScope text(g);{cap::GlyphScope glyph(g);const uint32_t at=selected.w[8]+16+(!std::strcmp(mode,"initial-logo-overlap")?0:369*686+123*2);{cap::WordCapture word(g,at,0xffff);put<uint16_t>(at,!std::strcmp(mode,"bad-word")?0x1234:0xffff);}if(!std::strcmp(mode,"extra-write"))put<uint16_t>(at+2,1);}
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
static std::unique_ptr<bg::Plane>actualCaptionPlane(){auto p=std::unique_ptr<bg::Plane>(new bg::Plane);p->target=selected;p->valid=1;p->width=343;p->height=480;std::memcpy(p->pixels.data(),memory.data()+selected.w[8]+16,343*480*2);return p;}
static nfs_player_initial_sidebar::Binding initialBinding(Game&g){nfs_player_initial_sidebar::Binding b;b.form=testSelector==2?0x4d3050:0x4d3100;b.formGuest=testOwner;b.archive=0x1234;b.game=reinterpret_cast<uintptr_t>(&g);b.thread=29;b.epoch=9;b.captionEvent={9,1,0};return b;}
static void initialSidebarCases(const std::shared_ptr<const nfs_main_sidebar_origin::Proof>&origin,const std::shared_ptr<const nfs_compact_caption::Proof>&caption,Game&g,bg::Workspace&workspace){
 namespace initial=nfs_player_initial_sidebar;using Proof=initial::Proof;
 static_assert(!std::is_default_constructible<Proof>::value,"initial sidebar requires genuine source proofs");
 static_assert(!std::is_copy_constructible<Proof>::value,"initial sidebar seal cannot be copied into a new authority");
 auto actual=actualCaptionPlane();const auto binding=initialBinding(g);auto catalog=origin->catalog();initial::Store store;
 auto good=store.accept(origin,caption,binding,*actual,workspace,catalog);assert(good&&good->current()&&store.current()==good&&store.acquire(binding,*actual,catalog)==good);
 assert(good->origin()==origin&&good->captionProof()==caption&&good->caption()==caption->packet()&&good->captionSerial()==caption->evidence().serial&&good->captionLifetime()==caption->evidence().lifetime);assert(good->binding().epoch==9&&good->origin()->epoch()==1&&good->origin()->groupEvent().ownerEpoch==1);assert(good->binding().captionEvent.ownerEpoch==9&&good->catalogMatches(*catalog));assert(good->post().pixels[369*343+123]==caption->packet()->original()[369*343+123]);
 auto badPlane=std::unique_ptr<bg::Plane>(new bg::Plane);
 for(const char*mode:{"form","form-guest","archive","game","thread","epoch","event-epoch","event-sequence","event-iteration","null-origin","null-caption","null-catalog","catalog","full-word","owner","header","x-lut","y-lut","handles","geometry","invalid-plane"}){
  auto old=store.accept(origin,caption,binding,*actual,workspace,catalog);assert(old&&old->current());auto b=binding;*badPlane=*actual;auto o=origin;auto c=caption;auto cat=catalog;
  if(!std::strcmp(mode,"form"))b.form=b.form==0x4d3050?0x4d3100:0x4d3050;if(!std::strcmp(mode,"form-guest"))b.formGuest^=4;if(!std::strcmp(mode,"archive"))++b.archive;if(!std::strcmp(mode,"game"))++b.game;if(!std::strcmp(mode,"thread"))++b.thread;if(!std::strcmp(mode,"epoch"))b.epoch=0;if(!std::strcmp(mode,"event-epoch"))b.captionEvent.ownerEpoch=1;if(!std::strcmp(mode,"event-sequence"))b.captionEvent.sequence=0;if(!std::strcmp(mode,"event-iteration"))b.captionEvent.iteration=1;
  if(!std::strcmp(mode,"null-origin"))o.reset();if(!std::strcmp(mode,"null-caption"))c.reset();if(!std::strcmp(mode,"null-catalog"))cat.reset();if(!std::strcmp(mode,"catalog")){auto changed=std::make_shared<bg::SourceCatalog>(*cat);changed->identities[4].sha[0]^=1;cat=changed;}
  if(!std::strcmp(mode,"full-word"))badPlane->pixels[100000]^=1;if(!std::strcmp(mode,"owner"))badPlane->target.owner^=4;if(!std::strcmp(mode,"header"))badPlane->target.header[3]^=1;if(!std::strcmp(mode,"x-lut"))badPlane->target.x[241]^=2;if(!std::strcmp(mode,"y-lut"))badPlane->target.y[398]^=2;if(!std::strcmp(mode,"handles"))badPlane->target.ownerState[2]^=4;if(!std::strcmp(mode,"geometry"))badPlane->target.w[3]=1;if(!std::strcmp(mode,"invalid-plane"))badPlane->valid=0;
  assert(!store.accept(o,c,b,*badPlane,workspace,cat));assert(!old->current()&&!store.current());
 }
 good=store.accept(origin,caption,binding,*actual,workspace,catalog);assert(good);auto replacement=store.accept(origin,caption,binding,*actual,workspace,catalog);assert(replacement&&replacement!=good&&!good->current());store.reset();assert(!replacement->current()&&!store.current());
 for(const char*mode:{"binding","pixels","catalog","handles"}){auto proof=store.accept(origin,caption,binding,*actual,workspace,catalog);assert(proof);auto b=binding;*badPlane=*actual;auto cat=catalog;if(!std::strcmp(mode,"binding"))++b.epoch;if(!std::strcmp(mode,"pixels"))badPlane->pixels[160000]^=1;if(!std::strcmp(mode,"handles"))badPlane->target.ownerState[1]^=4;if(!std::strcmp(mode,"catalog")){auto changed=std::make_shared<bg::SourceCatalog>(*cat);changed->mapSha[1][4]^=1;cat=changed;}assert(!store.acquire(b,*badPlane,cat)&&!proof->current());}
}
// Compose a real initial source from the genuine caption/Main proof above.
// The old Main catalog/map/group remain immutable. Only the NEW Player base,
// preview observations and original copy are constructed below.
static void initialSourceWitnessCases(const std::shared_ptr<const nfs_player_initial_sidebar::Proof>&initial,nfs_player_initial_sidebar::Store&pendingStore,bg::Workspace&workspace){
 namespace scene=nfs_player_scene_cpu;namespace pv=nfs_player_preview;namespace native=nfs_player_native;
 assert(initial&&initial->current());const auto&ib=initial->binding();const auto origin=initial->origin();
 auto base=std::unique_ptr<bg::Base>(new bg::Base(origin->base()));base->form=ib.form;base->event={9,10,0};
 auto previewFixture=std::unique_ptr<player_scene_fixture::pf::Fixture>(new player_scene_fixture::pf::Fixture(false,ib.form));auto&pi=previewFixture->in;
 pi.pre=base->post;pi.post=pi.pre;pi.lifetime=9;pi.serial=7;pi.archive=ib.archive;
 // Rebase the independently computed row oracle, not the sealed catalog or
 // old Main plane. Fill is black; draw arithmetic uses the fixture reference.
 pi.fill.before=pi.fill.after=pi.pre.target;
 for(auto&row:pi.fill.rows){const unsigned at=(row.args[1]-pi.pre.target.w[8]-16)/2;assert(at+row.args[2]<=bg::Full);for(unsigned k=0;k<row.args[2];++k){row.before[k]=pi.post.pixels[at+k];row.after[k]=0;pi.post.pixels[at+k]=0;}}
 for(auto&draw:pi.draws){draw.before=draw.after=pi.pre.target;for(auto&row:draw.rows){const unsigned at=(row.args[1]-pi.pre.target.w[8]-16)/2;assert(at+row.args[2]<=bg::Full);for(unsigned k=0;k<row.args[2];++k){row.before[k]=pi.post.pixels[at+k];const uint32_t color=draw.paletteBefore[row.source[k]];row.after[k]=player_scene_fixture::pf::blend(row.before[k],color);pi.post.pixels[at+k]=row.after[k];}}}
 auto preview=pv::createPacket(pi,base->post);assert(preview&&preview->form()==ib.form&&preview->baseMatches(base->post));
 auto copy=std::unique_ptr<rec::Copy>(new rec::Copy(origin->copy()));copy->serial=41;copy->thread=ib.thread;copy->pre=pi.post;copy->post=copy->pre;copy->sourceOwner=initial->post().target;copy->args[0]=copy->sourceOwner.w[8];std::memcpy(copy->sourceBefore.data(),copy->sourceOwner.header.data(),16);
 for(unsigned k=0;k<bg::Side;++k)put16(copy->sourceBefore.data()+16+2*k,initial->post().pixels[k]);copy->sourceAfter=copy->sourceBefore;assert(nfs_texture_sha256(copy->sourceBefore.data(),copy->span,copy->shaBefore.data()));copy->shaAfter=copy->shaBefore;
 // Reference original copy: keyed words leave PRE untouched, opaque words
 // replace it exactly. Expanded payload is written bytewise for every row.
 for(unsigned y=0;y<480;++y){auto&row=copy->row[y];for(unsigned x=0;x<343;++x){const unsigned sourceAt=y*343+x,at=y*640+x;const uint16_t word=initial->post().pixels[sourceAt];const uint32_t raw=word==row.colorKey?0:0xff000000u|uint32_t((word>>11)&31)*0x80000u|uint32_t((word>>5)&63)*0x400u|uint32_t(word&31)*8u;put32(row.source.data()+4*x,raw);if(word!=row.colorKey)copy->post.pixels[at]=word;}}
 std::atomic<uint64_t>globalEvents{23};std::vector<std::shared_ptr<const scene::VerifiedSource>>sources;std::vector<std::shared_ptr<const native::Proof>>proofs;std::vector<native::CurrentBinding>currents;std::vector<std::unique_ptr<native::Witness>>witnesses;auto lastActual=std::unique_ptr<bg::Plane>(new bg::Plane);
 for(bool refresh:{false,true}){
  copy->serial=refresh?42:41;scene::JoinedInput input(*base,*copy);auto&binding=input.binding;binding.form=ib.form;binding.formGuest=binding.sourceOwner=ib.formGuest;binding.archive=ib.archive;binding.gameOwner=ib.game;binding.thread=ib.thread;binding.epoch=binding.lifetime=9;binding.generation=refresh?24:23;binding.contentSerial=refresh?25:24;binding.copySerial=copy->serial;binding.kind=refresh?scene::SourceKind::Refresh:scene::SourceKind::Initial;
  input.initialSidebar=initial;input.actualSidebarPost=&initial->post();input.sidebarValidationEvent={9,12,0};input.preview=preview;input.acceptedPreviewSerial=7;input.previewLifetime=9;input.previewArchive=ib.archive;input.previewBaseEvent=base->event;input.previewEvent={9,11,0};input.actualPreviewPost=&pi.post;input.copyEvent={9,13,0};input.uploadEvent={9,refresh?21u:14u,0};
  std::unique_ptr<player_scene_fixture::sf::Fixture>swatchFixture;const bg::Plane*actualUpload=&copy->post;
  if(refresh){
   // Actual40CAB0 Refresh writes AFTER this exact original copy. Rebase both
   // recorded fill rows onto Copy.POST, leaving the original Copy unchanged.
   namespace swatch=nfs_player_swatch;swatchFixture.reset(new player_scene_fixture::sf::Fixture(ib.form==scene::Player1Form?1:2,swatch::Origin::Refresh));auto&a=swatchFixture->authority;auto&sb=a.binding;sb.owner=ib.formGuest;sb.epoch=sb.lifetime=9;sb.serial=61;sb.archive=ib.archive;sb.car=preview->selection().selectedCar;sb.colorAddress=(sb.player==1?0x512dc1:0x512ddc)+sb.car;a.events={14,15,16,17,18,19,20};swatchFixture->in.binding=sb;swatchFixture->in.events=a.events;swatchFixture->base=copy->post;swatchFixture->in.pre=copy->post;swatchFixture->in.post=copy->post;
   for(unsigned k=0;k<2;++k){auto&fill=swatchFixture->in.fills[k];fill.before=fill.after=copy->post.target;const unsigned x=k?366:367,y=k?141:142;const uint16_t value=k?0xf800:0;for(unsigned yy=0;yy<21;++yy){auto&row=fill.rows[yy];row.args[1]=copy->post.target.w[8]+16+((y+yy)*640+x)*2;for(unsigned xx=0;xx<84;++xx){auto&word=swatchFixture->in.post.pixels[(y+yy)*640+x+xx];row.before[xx]=word;row.after[xx]=value;word=value;}}}
   swatchFixture->current=swatchFixture->in.post;input.swatch=swatch::createPacket(swatchFixture->in,a,swatchFixture->base,swatchFixture->current);assert(input.swatch&&input.swatch->preMatches(copy->post));input.swatchAuthority=a;input.acceptedSwatchCopySerial=copy->serial;input.swatchCopyEvent=input.copyEvent;input.swatchEvent={9,20,0};input.swatchCopyPost=&copy->post;input.actualSwatchPost=&swatchFixture->current;actualUpload=&swatchFixture->current;
  }
  input.actualUploadPost=actualUpload;
  scene::Failure failure=scene::Failure::None;auto source=scene::verifySource(input,workspace,*origin->catalog(),&failure);if(!source)std::fprintf(stderr,"genuine inherited Initial sidebar source rejected form=%x refresh=%u failure=%u\n",ib.form,unsigned(refresh),unsigned(failure));assert(source&&source->current()&&source->original()==actualUpload->pixels);assert(source->binding().epoch==9&&origin->epoch()==1&&source->form()==ib.form&&source->binding().kind==(refresh?scene::SourceKind::Refresh:scene::SourceKind::Initial));
  // A genuine initial-side proof cannot authorize a Lazy redraw, even when
  // its surrounding copy events are made internally coherent for Lazy.
  const auto kind=binding.kind;binding.kind=scene::SourceKind::Lazy;input.sidebarValidationEvent.iteration=input.copyEvent.iteration=input.uploadEvent.iteration=1;assert(!scene::verifySource(input,workspace,*origin->catalog()));binding.kind=kind;input.sidebarValidationEvent.iteration=input.copyEvent.iteration=input.uploadEvent.iteration=0;
  if(refresh){auto swatch=input.swatch;input.swatch.reset();assert(!scene::verifySource(input,workspace,*origin->catalog()));input.swatch=swatch;}else{binding.kind=scene::SourceKind::Refresh;assert(!scene::verifySource(input,workspace,*origin->catalog()));binding.kind=kind;}
  const uint32_t sideOwner=copy->sourceOwner.owner;copy->sourceOwner.owner=copy->pre.target.owner;assert(!scene::verifySource(input,workspace,*origin->catalog()));copy->sourceOwner.owner=sideOwner;
  auto changedSide=std::unique_ptr<bg::Plane>(new bg::Plane(initial->post()));changedSide->pixels[100000]^=1;input.actualSidebarPost=changedSide.get();assert(!scene::verifySource(input,workspace,*origin->catalog()));input.actualSidebarPost=&initial->post();
  auto changedUpload=std::unique_ptr<bg::Plane>(new bg::Plane(*actualUpload));changedUpload->pixels[250000]^=1;input.actualUploadPost=changedUpload.get();assert(!scene::verifySource(input,workspace,*origin->catalog()));input.actualUploadPost=actualUpload;
  native::CurrentBinding current;current.source=binding;current.sourceOwner=ib.formGuest;current.fullTargetOwner=actualUpload->target.owner;current.sourceBitmap=actualUpload->target.w[8];globalEvents.store(binding.generation);if(refresh)assert(sources.front()->current()&&!proofs.front()->current(currents.front()));auto witness=std::unique_ptr<native::Witness>(new native::Witness);assert(witness->begin(source,current,*actualUpload,globalEvents.load(),globalEvents));
  auto tile=[&](unsigned n){native::TileObservation t;t.descriptorGuest=t.expectedSlotDescriptor=0x20000000u+64*n;t.payloadGuest=0xc0000000u;t.before={{3,3,3,11,0,0x01000000u+2048*n,3}};t.after=t.before;t.after[4]=t.payloadGuest;t.originalUploadReturned=true;const unsigned tx=n/15,ty=n%15;for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){const uint16_t word=actualUpload->pixels[(ty*32+y)*640+tx*32+x];const uint16_t rgb=uint16_t(((word>>11)&31)*1024+((word>>6)&31)*32+(word&31));const uint16_t payload=rgb?uint16_t(0x8000|rgb):0;const unsigned at=2*(y*32+x);t.pixels[at]=uint8_t(payload);t.pixels[at+1]=uint8_t(payload>>8);}return t;};
  auto wrong=tile(0);wrong.pixels[2047]^=1;assert(!witness->tile(wrong,current));assert(witness->begin(source,current,*actualUpload,globalEvents.load(),globalEvents));for(unsigned k=0;k<300;++k)assert(witness->tile(tile(k),current));assert(witness->uploaded()==300&&witness->finish(*actualUpload,true,current));auto nativeProof=witness->proofForPresentation(current,*actualUpload);assert(nativeProof&&nativeProof->current(current)&&nativeProof->source(current)==source);
  auto prepared=std::unique_ptr<scene::Prepared>(new scene::Prepared);assert(nativeProof->preparedForPresentation(current,*actualUpload,*prepared));assert(prepared->valid&&prepared->form==ib.form&&prepared->original==actualUpload->pixels&&prepared->caption==initial->caption()&&prepared->preview==preview);for(unsigned k=0;k<300;++k)assert(prepared->keys[k]==0x01000000u+2048*k);const auto view=prepared->view();assert(view.schema==NFS_PLAYER_SCENE_RUNTIME_SCHEMA&&view.scene.tailKind==(ib.form==scene::Player1Form?NFS_BACKGROUND_TAIL_PLAYER1_CAPTION:NFS_BACKGROUND_TAIL_PLAYER2_CAPTION));
  // Keep Witness owners and the same process-counter fixture alive through
  // common Store reset. The Refresh upload also overwrites the prior upload
  // event, independently revoking that earlier native proof.
  sources.push_back(source);proofs.push_back(nativeProof);currents.push_back(current);witnesses.push_back(std::move(witness));*lastActual=*actualUpload;
 }
 pendingStore.clear();assert(!initial->current());for(unsigned k=0;k<sources.size();++k)assert(!sources[k]->current()&&!proofs[k]->current(currents[k]));auto invalidPrepared=std::unique_ptr<scene::Prepared>(new scene::Prepared);assert(!proofs.back()->preparedForPresentation(currents.back(),*lastActual,*invalidPrepared)&&!invalidPrepared->valid);
}
// The source fixture publishes through the actual Main verifier and Store.
// Only guest reads/register execution are simulated here; no public metadata
// or private constructor is used to manufacture Main origin authority.
static void initialCaptionCases(){
 if(testSelector!=2&&testSelector!=3)return;
 using Origin=nfs_main_sidebar_origin::Proof;
 auto fixture=std::unique_ptr<main343_origin_fixture::Fixture>(new main343_origin_fixture::Fixture);
 const auto originalFixtureSide=fixture->scene.tailPost.target;
 auto slots=[&](unsigned slot){
  auto target=[&](rec::Target&t){t.ownerState[0]=slot<<16;t.ownerState[1]=slot?0x1c00:0x1800;t.ownerState[2]=slot?0x1800:0x1c00;};
  target(fixture->base.post.target);for(auto*p:{&fixture->scene.preSide,&fixture->scene.postSide,&fixture->scene.postWords,&fixture->scene.tailPre,&fixture->scene.tailPost,&fixture->group.pre,&fixture->group.post,&fixture->copy.pre,&fixture->copy.post})target(p->target);target(fixture->copy.sourceOwner);
  for(auto&d:fixture->scene.draws){target(d.before);target(d.after);}for(auto&d:fixture->group.draw){target(d.before);target(d.after);}
 };
 auto materialize=[&](const Origin&origin,unsigned slot){selected=origin.post().target;selected.ownerState[0]=slot<<16;std::memcpy(memory.data()+selected.w[8],selected.header.data(),16);std::memcpy(memory.data()+selected.w[8]+16,origin.post().pixels.data(),343*480*2);std::memcpy(memory.data()+selected.owner,selected.ownerWords.data(),56);put<uint32_t>(0x1c00+32,0xa0000);put<uint32_t>(dword_4D2314+2,slot<<16);put<uint32_t>(dword_4D231C,selected.ownerState[1]);put<uint32_t>(dword_4D2320,selected.ownerState[2]);put<uint32_t>(dword_4D243C,off_4D3EC4);};
 for(const char*mode:{"normal","initial-logo-overlap","sentinel","sentinel-change","sentinel-two","sentinel-two-change","unknown-negative","unknown-previous","slot-one","ambient-before","null","wrong-game","wrong-thread","wrong-main","archive","revoked","wrong-esi","wrong-ebp","wrong-global","wrong-slot","chosen-handle","bitmap-alias","same-handle","pre-pixel","pre-header","pre-lut","pre-owner","missing-resource","resource-ebp","resource-global","nested-caller","tail-without-caller","during-origin-retire","post-global","post-pixel","post-owner","post-esi","caller-unwind"}){
  prepare();Game g;g.eax=testOwner;g.esi=testOwner;g.ebp=off_4D3EC4;const unsigned slot=!std::strcmp(mode,"slot-one")?1:0;slots(slot);fixture->binding.game=!std::strcmp(mode,"wrong-game")?reinterpret_cast<uintptr_t>(&g)+1:reinterpret_cast<uintptr_t>(&g);fixture->binding.publisherThread=!std::strcmp(mode,"wrong-thread")?30:29;fixture->group.thread=fixture->copy.thread=fixture->binding.publisherThread;fixture->binding.formGuest=!std::strcmp(mode,"wrong-main")?off_4D3EC4+4:off_4D3EC4;fixture->binding.archive=!std::strcmp(mode,"archive")?0x1235:0x1234;
  nfs_main_sidebar_origin::Store store;auto origin=fixture->publish(store);assert(origin&&origin->current());materialize(*origin,slot);const uint32_t previous=(!std::strcmp(mode,"sentinel")||!std::strcmp(mode,"sentinel-change"))?UINT32_MAX:((!std::strcmp(mode,"sentinel-two")||!std::strcmp(mode,"sentinel-two-change"))?UINT32_MAX-1:(!std::strcmp(mode,"unknown-negative")?UINT32_MAX-2:(!std::strcmp(mode,"unknown-previous")?0x12345678:off_4D3EC4)));g.ebp=int32_t(previous);put<uint32_t>(dword_4D243C,previous);
  if(!std::strcmp(mode,"ambient-before")){selected.ownerState[0]=1;put<uint32_t>(dword_4D2314+2,1);} // low ambient bits, selected slot unchanged
  if(!std::strcmp(mode,"revoked"))store.clear();if(!std::strcmp(mode,"wrong-esi"))g.esi=123;if(!std::strcmp(mode,"wrong-ebp"))g.ebp=testOwner;if(!std::strcmp(mode,"wrong-global"))put<uint32_t>(dword_4D243C,testOwner);
  if(!std::strcmp(mode,"wrong-slot")){selected.ownerState[0]=1u<<16;put<uint32_t>(dword_4D2314+2,1u<<16);}if(!std::strcmp(mode,"chosen-handle"))put<uint32_t>(dword_4D231C,0x1c00);if(!std::strcmp(mode,"bitmap-alias"))put<uint32_t>(0x1c00+32,selected.w[8]);if(!std::strcmp(mode,"same-handle")){selected.ownerState[2]=selected.owner;put<uint32_t>(dword_4D2320,selected.owner);}
  if(!std::strcmp(mode,"pre-pixel"))put<uint16_t>(selected.w[8]+16,1);if(!std::strcmp(mode,"pre-header"))selected.header[2]^=1;if(!std::strcmp(mode,"pre-lut"))selected.x[125]^=2;if(!std::strcmp(mode,"pre-owner"))selected.owner^=4;
  std::shared_ptr<const nfs_compact_caption::Proof>escaped; nfs_player_initial_sidebar::Store pendingStore;std::shared_ptr<const nfs_player_initial_sidebar::Proof>pending;
  {cap::OwnerScope owner(g);
   auto draw=[&]{if(!std::strcmp(mode,"sentinel-two-change")){g.ebp=-1;put<uint32_t>(dword_4D243C,UINT32_MAX);}if(!std::strcmp(mode,"sentinel-change")){g.ebp=off_4D3EC4;put<uint32_t>(dword_4D243C,off_4D3EC4);}if(!std::strcmp(mode,"resource-ebp"))g.ebp=testOwner;if(!std::strcmp(mode,"resource-global"))put<uint32_t>(dword_4D243C,testOwner);{cap::TailScope tail(g);if(!std::strcmp(mode,"missing-resource"))stringBody(g,"normal");else trackedString(g,!std::strcmp(mode,"initial-logo-overlap")?mode:"normal");}escaped=cap::lastProof(g);g.esi=testOwner;};
   if(!std::strcmp(mode,"tail-without-caller")){draw();assert(!escaped);}
   else try{cap::InitialCallerScope caller(g,!std::strcmp(mode,"null")?std::shared_ptr<const Origin>{}:origin);if(!std::strcmp(mode,"nested-caller")){cap::InitialCallerScope nested(g,origin);}if(!std::strcmp(mode,"during-origin-retire"))store.clear();draw();if(!std::strcmp(mode,"caller-unwind")){auto actual=actualCaptionPlane();pending=pendingStore.accept(origin,escaped,initialBinding(g),*actual,fixture->workspace,origin->catalog());assert(pending&&pending->current());}if(!std::strcmp(mode,"post-global"))put<uint32_t>(dword_4D243C,testOwner);if(!std::strcmp(mode,"post-pixel"))put<uint16_t>(selected.w[8]+16,1);if(!std::strcmp(mode,"post-owner"))selected.owner^=4;if(!std::strcmp(mode,"post-esi"))g.esi=123;if(!std::strcmp(mode,"caller-unwind"))throw 8;}catch(int){assert(!std::strcmp(mode,"caller-unwind"));}
   const bool okay=!std::strcmp(mode,"normal")||!std::strcmp(mode,"initial-logo-overlap")||!std::strcmp(mode,"sentinel")||!std::strcmp(mode,"sentinel-two")||!std::strcmp(mode,"slot-one")||!std::strcmp(mode,"ambient-before");auto proof=cap::acquireCurrentProof(g);if(bool(proof)!=okay)std::fprintf(stderr,"initial route mode=%s failure=%s\n",mode,cap::failureName(cap::current(g)->failure));assert(bool(proof)==okay);
   if(okay){assert(proof->origin()==origin&&proof->current());const auto&e=proof->evidence();assert(e.route==nfs_compact_caption::Route::InitialMain&&e.incomingEbp==previous&&e.formGuest==testOwner);assert(origin->epoch()==1&&origin->scene().form==bg::MainForm);assert(proof->packet()->clean()[0]==origin->post().pixels[0]);assert(cap::lastPacket(g)==proof->packet());const auto before=reads;std::thread foreign([&]{foreignPath=true;assert(!cap::acquireCurrentProof(g));foreignPath=false;});foreign.join();assert(before==reads&&foreignAllocations==0);auto actual=actualCaptionPlane();pending=pendingStore.accept(origin,proof,initialBinding(g),*actual,fixture->workspace,origin->catalog());if(!std::strcmp(mode,"initial-logo-overlap"))assert(!pending);else{assert(pending&&pending->current());if(!std::strcmp(mode,"normal")){initialSidebarCases(origin,proof,g,fixture->workspace);initialSourceWitnessCases(pending,pendingStore,fixture->workspace);}if(!std::strcmp(mode,"sentinel")){nfs_main_sidebar_origin::retireGame(reinterpret_cast<uintptr_t>(&g));assert(!pending->current()&&!pendingStore.current());}}store.clear();assert(!proof->current()&&!cap::acquireCurrentProof(g));if(pending)assert(!pending->current()&&!pendingStore.current());}
   else if(escaped)assert(!escaped->current());if(!std::strcmp(mode,"caller-unwind"))assert(pending&&!pending->current()&&!pendingStore.current());
  }
  assert(!leaseDepth&&allocationsInLease==0);
 }
 assert(originalFixtureSide.owner==0x1800);
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
 descriptorRetirement();sealedProofCases();initialCaptionCases();
 assert(retained&&retained->pixels()==343*480&&retained->original()[369*343+123]==0xffff);assert(!leaseDepth&&allocationsInLease==0);std::puts("PASS cached menu caption (Options/Setup/Player1/Player2): compact343 full replay/ownership/current-plane rejection, exact owner/font gate, original words, palette/font/mode/LUT/selection/retirement/nesting fallback, shared-LUT descriptor refs/lifetime, immutable lifetime, zero allocations in guest leases");
}
