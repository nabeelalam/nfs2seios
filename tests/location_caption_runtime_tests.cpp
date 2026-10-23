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
void*operator new(size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}
void operator delete(void*p,const std::nothrow_t&)noexcept{std::free(p);}
struct Game{int32_t eax=0,edx=0,ebx=0,ecx=0;};
static constexpr uint32_t off_4D2A20=0x100,dword_4D40B8=0x200,dword_4DD770=0x208,dword_4DD77C=0x210,dword_4DD774=0x218,dword_4DD714=0x220,dword_4D20A0=0x228,byte_4DB25E=0x230,byte_4DB25C=0x238,dword_4DD6F4=0x240,dword_4DD6F8=0x248,dword_4E54AC=0x250,dword_4DD6EC=0x258,dword_4DB240=0x1000;
static constexpr uint32_t game_id_sub_4A5830=0xface0010;
inline uint32_t guest_address(uint32_t p){return p;}
inline void*nfs_host_address(uint32_t p,size_t n){if(!leaseDepth||uint64_t(p)+n>memory.size()||(deniedRead&&uint64_t(p)<=deniedRead&&uint64_t(p)+n>deniedRead))throw 1;++reads;return memory.data()+p;}
namespace nfs{struct GuestCopyTransaction{GuestCopyTransaction(){++leaseDepth;}~GuestCopyTransaction(){--leaseDepth;}template<class T>T load(uint32_t p){T v;std::memcpy(&v,nfs_host_address(p,sizeof(T)),sizeof(T));return v;}};}
static nfs_logo_capture::Target selected{};static bool feature=true;
namespace nfs_logo_publisher{inline bool enabled(){return feature;}inline int exceptions(){return std::uncaught_exception()?1:0;}inline void checked(uint32_t p,size_t n){(void)nfs_host_address(p,n);}inline void target(nfs::GuestCopyTransaction&,nfs_logo_capture::Target&t){++reads;t=selected;}}
#define NFS_LOCATION_CAPTION_TEST 1
#include "hd/LocationCaptionGameHooks.inc"
namespace cap=nfs_location_caption;
template<class T>static void put(uint32_t p,T v){assert(uint64_t(p)+sizeof v<=memory.size());std::memcpy(memory.data()+p,&v,sizeof v);}
static void prepare(){deniedRead=0;memory.fill(0);selected={};auto&w=selected.w;w[1]=343;w[2]=480;w[5]=343;w[6]=480;w[7]=0x7810;w[8]=0x40000;w[10]=686;w[11]=0x2000;w[12]=0x3000;selected.owner=0x1800;selected.ownerWords=w;selected.header={{0x78,343|(480u<<16),0,0}};for(unsigned k=0;k<343;++k)selected.x[k]=2*k;for(unsigned k=0;k<480;++k)selected.y[k]=16+686*k;for(unsigned k=0;k<4;++k)put(w[8]+4*k,selected.header[k]);put<uint32_t>(dword_4D40B8,0x1234);put<uint32_t>(dword_4DD770,0x98000);put<uint32_t>(dword_4DD77C,game_id_sub_4A5830);put<uint32_t>(dword_4DD774,0x99000);put<uint32_t>(dword_4DD714,0xffff);put<uint8_t>(byte_4DB25C,16);put<int32_t>(dword_4E54AC,18);put<int32_t>(dword_4DD6EC,116);std::memcpy(memory.data()+0x90000,"location",9);std::memcpy(memory.data()+0x94000,"location",9);for(unsigned k=0;k<16;++k)put<uint32_t>(0x99000+4*k,0xff000000u+k);}
static std::shared_ptr<const nfs_soft::FontCatalog>font(){nfs_soft::FontDescription d;for(auto&role:d.glyphs)for(auto&g:role){g.x=g.y=2;g.w=g.h=1;g.advance=65536;}std::array<uint8_t,1024*512>a{};a[2*1024+2]=255;return nfs_soft::FontCatalog::create(d,a.data(),a.size(),17);}
static void stringBody(Game&g,const char*mode){g.eax=!std::strncmp(mode,"proof-",6)?0x94000:0x90000;g.edx=116;g.ebx=162;cap::StringScope text(g);{cap::GlyphScope glyph(g);const uint32_t at=selected.w[8]+16+167*686+117*2;{cap::WordCapture word(g,at,0xffff);put<uint16_t>(at,!std::strcmp(mode,"bad-word")?0x1234:0xffff);}if(!std::strcmp(mode,"extra-write"))put<uint16_t>(at+2,1);}
 if(!std::strcmp(mode,"palette"))put<uint32_t>(0x99004,123);if(!std::strcmp(mode,"font"))put<uint32_t>(dword_4DD770,0x97000);if(!std::strcmp(mode,"font-header"))put<uint8_t>(0x98000,1);if(!std::strcmp(mode,"proof-text-change"))memory[0x94000]^=1;if(!std::strcmp(mode,"proof-text-read"))deniedRead=0x94000;put<int32_t>(dword_4DD6EC,!std::strcmp(mode,"proof-advance")?167:168);}
static void descriptorRetirement(){
 for(const char*mode:{"shared-x","shared-y","shared-both","final-x","final-y","zero-x","zero-y","negative-x","negative-y","read-x","read-y","bitmap","owner","unrelated","rebind-x"}){
  prepare();Game g;g.eax=off_4D2A20;cap::OwnerScope owner(g);auto*s=cap::current(g);assert(s);{cap::TailScope tail(g);stringBody(g,"normal");}auto packet=cap::lastPacket(g);assert(packet);const auto lifetime=s->lifetime,serial=s->serial;const auto originalTarget=selected;const auto currentIdentity=s->current.identity;
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
  assert(packet->original()[167*343+117]==0xffff&&packet->clean()[167*343+117]==0&&packet->pixels()==343*480);assert(!leaseDepth&&allocationsInLease==0);
 }
}
static void proofCases(){
 using Proof=nfs_location_caption_source::Proof;using Evidence=nfs_location_caption_source::Evidence;
 static_assert(!std::is_default_constructible<Proof>::value,"source proof cannot be publicly fabricated");
 static_assert(!std::is_constructible<Proof,std::shared_ptr<const nfs_soft::FramePacket>,const Evidence&>::value,"public observations do not seal source evidence");
 std::shared_ptr<const Proof>retained;
 for(const char*mode:{"proof-normal","proof-nonwhite","proof-origin","proof-advance","proof-text-change","proof-text-read","proof-post-text","proof-clip"}){
  prepare();Game g;g.eax=off_4D2A20;
  {cap::OwnerScope owner(g);auto*s=cap::current(g);assert(s);
   if(!std::strcmp(mode,"proof-nonwhite"))put<uint32_t>(dword_4DD714,0x7bef);
   if(!std::strcmp(mode,"proof-origin"))put<int32_t>(dword_4DD6F4,1);
   if(!std::strcmp(mode,"proof-clip")){selected.w[5]=342;selected.ownerWords=selected.w;}
   {cap::TailScope tail(g);assert(!cap::lastProof(g)&&!cap::acquireCurrentProof(g));stringBody(g,mode);if(!std::strcmp(mode,"proof-post-text"))memory[0x94000]^=1;}
   deniedRead=0;auto packet=cap::lastPacket(g);assert(packet); // Supplemental proof failures preserve legacy replay.
   auto proof=cap::lastProof(g);const bool accepted=!std::strcmp(mode,"proof-normal");assert(bool(proof)==accepted);
   if(!proof){assert(!cap::acquireCurrentProof(g));continue;}
   assert(cap::acquireCurrentProof(g)==proof&&proof->current()&&proof->packet()==packet&&proof->font()==packet->font());
   const auto&e=proof->evidence();assert(e.logicalForm==0x404abd&&e.formGuest==off_4D2A20&&e.logicalForm!=e.formGuest);
   assert(e.game==reinterpret_cast<uintptr_t>(&g)&&e.policyThread==cap::token()&&e.lifetime==s->lifetime&&e.serial==s->serial&&e.textGuest==0x94000);
   assert(e.command.length==8&&!std::memcmp(e.command.text.data(),"location",8)&&e.command.x==116&&e.command.y==162&&e.command.advance==52&&e.command.sourceSize==18&&e.command.color==0xffff);
   assert(e.command.role==1&&!e.command.alignment&&!e.command.originX&&!e.command.originY&&e.command.clip.right==343&&e.command.clip.bottom==480);
   for(const auto*p:{&e.pre,&e.stringBefore,&e.stringAfter,&e.post})assert(p->archive==0x1234&&p->target.owner==selected.owner&&p->identity.lifetime==s->lifetime);
   assert(e.stringBefore.font==0x98000&&e.stringAfter.font==0x98000&&e.stringBefore.fontHeader==e.stringAfter.fontHeader&&e.stringBefore.paletteWords==e.stringAfter.paletteWords);
   assert(packet->records().size()==1&&packet->records()[0].command.text==e.command.text);
   const auto actual=selected;const auto captured=packet->original();
   put<uint32_t>(dword_4D40B8,0x1235);assert(!cap::acquireCurrentProof(g)&&cap::acquireCurrentPacket(g)==packet);put<uint32_t>(dword_4D40B8,0x1234);assert(cap::acquireCurrentProof(g)==proof);
   selected.owner+=4;assert(!cap::acquireCurrentProof(g)&&cap::acquireCurrentPacket(g)==packet);selected=actual;assert(cap::acquireCurrentProof(g)==proof);
   selected.ownerWords[3]^=1;assert(!cap::acquireCurrentProof(g));selected=actual;assert(cap::acquireCurrentProof(g)==proof);
   selected.ownerState[1]^=4;assert(!cap::acquireCurrentProof(g));selected=actual;assert(cap::acquireCurrentProof(g)==proof);
   memory[e.textGuest]^=1;assert(!cap::acquireCurrentProof(g)&&cap::acquireCurrentPacket(g)==packet);memory[e.textGuest]^=1;assert(cap::acquireCurrentProof(g)==proof);
   feature=false;assert(!cap::acquireCurrentProof(g));feature=true;assert(cap::acquireCurrentProof(g)==proof);
   put<uint16_t>(selected.w[8]+16,1);assert(!cap::acquireCurrentProof(g));put<uint16_t>(selected.w[8]+16,captured[0]);assert(cap::acquireCurrentProof(g)==proof);
   const auto beforeReads=reads;std::thread foreign([&]{foreignPath=true;assert(!cap::lastProof(g)&&!cap::acquireCurrentProof(g));foreignPath=false;});foreign.join();assert(reads==beforeReads&&foreignAllocations==0);
   retained=proof;cap::beforeOwnerRetire(g,selected.owner);assert(!cap::lastProof(g)&&!cap::acquireCurrentProof(g));assert(proof->current()&&proof->packet()->original()==captured);
  }
  assert(!cap::lastProof(g)&&!cap::acquireCurrentProof(g));
 }
 assert(retained&&retained->current()&&retained->packet()->original()[167*343+117]==0xffff);
 // Logical scene handler identity must never be admitted as the lexical descriptor.
 prepare();Game logical;logical.eax=0x404abd;{cap::OwnerScope wrong(logical);assert(!cap::current(logical)&&!cap::lastProof(logical));}
}
int main(){prepare();Game g;g.eax=off_4D2A20;{cap::OwnerScope nofont(g);assert(!cap::current(g));}assert(nfs_soft::installFontCatalog(font()));feature=false;auto priorReads=reads;{cap::OwnerScope disabled(g);}assert(reads==priorReads);feature=true;g.eax=0x999;{cap::OwnerScope wrong(g);assert(!cap::current(g));}
 std::shared_ptr<const nfs_soft::FramePacket> retained;
 for(const char*mode:{"normal","bad-word","extra-write","palette","font","font-header","wrong-mode","wrong-callback","wrong-size","unknown-text","lut","selection","nested","retire","unrelated"}){prepare();g.eax=off_4D2A20;auto original=g;cap::OwnerScope owner(g);auto*s=cap::current(g);assert(s&&s->font);assert(!std::memcmp(&g,&original,sizeof g));if(!std::strcmp(mode,"wrong-mode"))put<uint32_t>(dword_4D20A0,1);if(!std::strcmp(mode,"wrong-callback"))put<uint32_t>(dword_4DD77C,123);if(!std::strcmp(mode,"wrong-size"))put<int32_t>(dword_4E54AC,17);if(!std::strcmp(mode,"unknown-text"))memory[0x90000]=uint8_t(88);if(!std::strcmp(mode,"lut"))selected.x[100]^=2;
  {cap::TailScope tail(g);if(!std::strcmp(mode,"nested")){cap::TailScope nested(g);}else{if(!std::strcmp(mode,"selection")){cap::SelectionScope change(g);selected.w[3]=1;}stringBody(g,mode);}}
  bool okay=!std::strcmp(mode,"normal")||!std::strcmp(mode,"unrelated")||!std::strcmp(mode,"retire");auto packet=cap::lastPacket(g);assert(bool(packet)==okay);
  if(packet){assert(packet->pixels()==343*480&&packet->surface().geometry[10]==686&&packet->commands()==1&&packet->writes()==1);assert(packet->original()[167*343+117]==0xffff&&packet->clean()[167*343+117]==0);assert(cap::acquireCurrentPacket(g)==packet);
   if(!std::strcmp(mode,"retire")){cap::beforeOwnerRetire(g,selected.owner);assert(!cap::lastPacket(g));}else if(!std::strcmp(mode,"unrelated")){cap::beforeOwnerRetire(g,0);cap::beforeOwnerRetire(g,0x9a000);assert(cap::lastPacket(g)==packet);}else{retained=packet;put<uint16_t>(selected.w[8]+16,1);assert(!cap::acquireCurrentPacket(g));assert(packet->original()[0]==0);}
  }else assert(s->failure!=cap::Failure::none);
  if(!std::strcmp(mode,"normal")){std::thread foreign([&]{foreignPath=true;Game other;other.eax=off_4D2A20;{cap::OwnerScope foreignOwner(other);assert(!cap::current(other));}foreignPath=false;});foreign.join();assert(foreignAllocations==0);}
 }
 descriptorRetirement();proofCases();
 assert(retained&&retained->pixels()==343*480&&retained->original()[167*343+117]==0xffff);assert(!leaseDepth&&allocationsInLease==0);std::puts("PASS Location caption: compact343 full replay/ownership/current-plane rejection, exact owner/font gate, original words, palette/font/mode/LUT/selection/retirement/nesting fallback, shared-LUT descriptor refs/lifetime, immutable lifetime, zero allocations in guest leases");
}
