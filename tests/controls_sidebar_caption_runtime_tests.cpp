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
#include "hd/OriginalUiRecords.h"
static std::array<uint8_t,4194304> memory{};static unsigned leaseDepth=0,reads=0;static uint32_t deniedRead=0;static std::atomic<unsigned> allocationsInLease{0},foreignAllocations{0};static thread_local bool foreignPath=false;
void*operator new(size_t n){if(leaseDepth)++allocationsInLease;if(foreignPath)++foreignAllocations;if(void*p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}
void operator delete(void*p,size_t)noexcept{std::free(p);}
void*operator new(size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}
void operator delete(void*p,const std::nothrow_t&)noexcept{std::free(p);}
struct Game{int32_t eax=0,edx=0,ebx=0,ecx=0,ebp=0,esi=0;};
static constexpr uint32_t off_4D37C8=0x100,dword_4D40B8=0x200,dword_4DD770=0x208,dword_4DD77C=0x210,dword_4DD774=0x218,dword_4DD714=0x220,dword_4D20A0=0x228,byte_4DB25E=0x230,byte_4DB25C=0x238,dword_4DD6F4=0x240,dword_4DD6F8=0x248,dword_4E54AC=0x250,dword_4DD6EC=0x258,dword_4DB240=0x1000,dword_4D243C=0x260,byte_4D3ECC=0x268,off_4D3ED0=0x280,dword_4D3E9C=0x2c0;
static constexpr uint32_t game_id_sub_4A5830=0xface0010;
inline uint32_t guest_address(uint32_t p){return p;}
inline void*nfs_host_address(uint32_t p,size_t n){if(!leaseDepth||uint64_t(p)+n>memory.size()||(deniedRead&&uint64_t(p)<=deniedRead&&uint64_t(p)+n>deniedRead))throw 1;++reads;return memory.data()+p;}
namespace nfs{struct GuestCopyTransaction{GuestCopyTransaction(){++leaseDepth;}~GuestCopyTransaction(){--leaseDepth;}template<class T>T load(uint32_t p){T v;std::memcpy(&v,nfs_host_address(p,sizeof(T)),sizeof(T));return v;}};}
static nfs_logo_capture::Target selected{};static bool feature=true;
namespace nfs_logo_publisher{inline uintptr_t threadToken(){static thread_local unsigned char t;return reinterpret_cast<uintptr_t>(&t);}inline bool enabled(){return feature;}inline int exceptions(){return std::uncaught_exceptions();}inline void checked(uint32_t p,size_t n){(void)nfs_host_address(p,n);}inline void target(nfs::GuestCopyTransaction&,nfs_logo_capture::Target&t){++reads;t=selected;}}
#define NFS_CONTROLS_SIDEBAR_TEXT_TEST 1
#include "hd/ControlsSidebarCaptionGameHooks.inc"
namespace cap=nfs_controls_sidebar_text;
template<class T>static void put(uint32_t p,T v){assert(uint64_t(p)+sizeof v<=memory.size());std::memcpy(memory.data()+p,&v,sizeof v);}
static void prepare(){deniedRead=0;memory.fill(0);put<uint32_t>(dword_4D243C,0xdeadbeef);put<uint32_t>(byte_4D3ECC,6u<<16);put<uint32_t>(off_4D3ED0+6*4,dword_4D3E9C);put<uint32_t>(dword_4D3E9C,0x70002);selected={};auto&w=selected.w;w[1]=343;w[2]=480;w[5]=343;w[6]=480;w[7]=0x7810;w[8]=0x40000;w[10]=686;w[11]=0x2000;w[12]=0x3000;selected.owner=0x1800;selected.ownerWords=w;selected.header={{0x78,343|(480u<<16),0,0}};for(unsigned k=0;k<343;++k)selected.x[k]=2*k;for(unsigned k=0;k<480;++k)selected.y[k]=16+686*k;for(unsigned k=0;k<4;++k)put(w[8]+4*k,selected.header[k]);put<uint32_t>(dword_4D40B8,0x1234);put<uint32_t>(dword_4DD770,0x98000);put<uint32_t>(dword_4DD77C,game_id_sub_4A5830);put<uint32_t>(dword_4DD774,0x99000);put<uint32_t>(dword_4DD714,0xffff);put<uint8_t>(byte_4DB25C,16);put<int32_t>(dword_4E54AC,18);put<int32_t>(dword_4DD6EC,116);std::memcpy(memory.data()+0x90000,"options",8);for(unsigned k=0;k<16;++k)put<uint32_t>(0x99000+4*k,0xff000000u+k);}
static std::shared_ptr<const nfs_soft::FontCatalog>font(){nfs_soft::FontDescription d;for(auto&role:d.glyphs)for(auto&g:role){g.x=g.y=2;g.w=g.h=1;g.advance=65536;}std::array<uint8_t,1024*512>a{};a[2*1024+2]=255;return nfs_soft::FontCatalog::create(d,a.data(),a.size(),17);}
static void stringBody(Game&g,const char*mode){g.eax=0x90000;g.esi=!std::strcmp(mode,"measure-anchor")?169:168;g.edx=!std::strcmp(mode,"wrong-position")?121:120;g.ebx=364;if(std::strcmp(mode,"missing-measure")){if(!std::strcmp(mode,"measure-color"))put<uint32_t>(dword_4DD714,0x1234);cap::measured(g,!std::strcmp(mode,"measure-pointer")?0x90001:0x90000,!std::strcmp(mode,"measure-zero")?0:(!std::strcmp(mode,"center")?96:48),!std::strcmp(mode,"center")?cap::SourceMeasureAlignment::center:cap::SourceMeasureAlignment::right);if(!std::strcmp(mode,"measure-duplicate"))cap::measured(g,0x90000,48,cap::SourceMeasureAlignment::right);if(!std::strcmp(mode,"measure-font"))put<uint8_t>(0x98000,1);if(!std::strcmp(mode,"measure-color"))put<uint32_t>(dword_4DD714,0xffff);}cap::StringScope text(g);{cap::GlyphScope glyph(g);const uint32_t at=selected.w[8]+16+369*686+123*2;{cap::WordCapture word(g,at,0xffff);put<uint16_t>(at,!std::strcmp(mode,"bad-word")?0x1234:0xffff);}if(!std::strcmp(mode,"extra-write"))put<uint16_t>(at+2,1);}
 if(!std::strcmp(mode,"palette"))put<uint32_t>(0x99004,123);if(!std::strcmp(mode,"font"))put<uint32_t>(dword_4DD770,0x97000);if(!std::strcmp(mode,"font-header"))put<uint8_t>(0x98000,1);put<int32_t>(dword_4DD6EC,g.edx+48);}
static void descriptorRetirement(){
 for(const char*mode:{"shared-x","shared-y","shared-both","final-x","final-y","zero-x","zero-y","negative-x","negative-y","read-x","read-y","bitmap","owner","unrelated","rebind-x"}){
  prepare();Game g;g.eax=off_4D37C8;g.ebp=off_4D37C8;cap::OwnerScope owner(g);auto*s=cap::current(g);assert(s);{cap::TailScope tail(g);stringBody(g,"normal");}auto packet=cap::lastPacket(g);assert(packet);const auto lifetime=s->lifetime,serial=s->serial;const auto originalTarget=selected;const auto currentIdentity=s->current.identity;
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
#define NFS_NESTED_TEST_FORM off_4D37C8
#define NFS_NESTED_TEST_POLICY nfs_nested_sidebar_caption::Policy::Controls
#include "NestedSidebarCaptionProofCases.h"
#undef NFS_NESTED_TEST_POLICY
#undef NFS_NESTED_TEST_FORM
int main(){assert(cap::token()==nfs_logo_publisher::threadToken());prepare();Game g;g.eax=off_4D37C8;g.ebp=off_4D37C8;{cap::OwnerScope nofont(g);assert(!cap::current(g));}assert(nfs_soft::installFontCatalog(font()));feature=false;auto priorReads=reads;{cap::OwnerScope disabled(g);}assert(reads==priorReads);feature=true;g.eax=0x4d3348;{cap::OwnerScope wrong(g);assert(!cap::current(g));auto before=reads;cap::TailScope outside(g);assert(reads==before);}
 std::shared_ptr<const nfs_soft::FramePacket> retained;
 for(const char*mode:{"normal","right","center","measure-anchor","missing-measure","measure-pointer","measure-zero","measure-duplicate","measure-font","measure-color","bad-word","extra-write","palette","font","font-header","wrong-mode","wrong-callback","wrong-size","unknown-text","unrelated-global","wrong-tail","ordinal-seven","wrong-resource","aliased-resource","wrong-category","wrong-color","wrong-position","lut","y-lut","header","selection","nested","retire","unrelated"}){prepare();g.eax=off_4D37C8;g.ebp=off_4D37C8;auto original=g;cap::OwnerScope owner(g);auto*s=cap::current(g);assert(s&&s->font);assert(!std::memcmp(&g,&original,sizeof g));if(!std::strcmp(mode,"wrong-mode"))put<uint32_t>(dword_4D20A0,1);if(!std::strcmp(mode,"wrong-callback"))put<uint32_t>(dword_4DD77C,123);if(!std::strcmp(mode,"wrong-size"))put<int32_t>(dword_4E54AC,17);if(!std::strcmp(mode,"unknown-text"))memory[0x90000]=uint8_t(88);if(!std::strcmp(mode,"lut"))selected.x[100]^=2;if(!std::strcmp(mode,"y-lut"))selected.y[250]^=2;if(!std::strcmp(mode,"header"))selected.header[1]^=1;if(!std::strcmp(mode,"unrelated-global"))put<uint32_t>(dword_4D243C,123);if(!std::strcmp(mode,"wrong-tail"))g.ebp=123;if(!std::strcmp(mode,"ordinal-seven"))put<uint32_t>(byte_4D3ECC,7u<<16);if(!std::strcmp(mode,"wrong-resource"))put<uint32_t>(dword_4D3E9C,0x80002);if(!std::strcmp(mode,"aliased-resource")){put<uint32_t>(off_4D3ED0+6*4,0x2e0);put<uint32_t>(0x2e0,0x70002);}if(!std::strcmp(mode,"wrong-category"))put<uint32_t>(byte_4D3ECC,1u<<16);if(!std::strcmp(mode,"wrong-color"))put<uint32_t>(dword_4DD714,123);
  {cap::TailScope tail(g);if(!std::strcmp(mode,"nested")){cap::TailScope nested(g);}else{if(!std::strcmp(mode,"selection")){cap::SelectionScope change(g);selected.w[3]=1;}stringBody(g,mode);}}
  bool okay=!std::strcmp(mode,"right")||!std::strcmp(mode,"measure-color")||!std::strcmp(mode,"unrelated-global")||!std::strcmp(mode,"normal")||!std::strcmp(mode,"unrelated")||!std::strcmp(mode,"retire");auto packet=cap::lastPacket(g);if(!std::strcmp(mode,"normal"))assert(s->diagnosticStage==4&&s->diagnosticCenter==168&&s->diagnosticAdvance==48&&s->diagnosticFontDiff==0&&s->diagnosticTailCaptured);if(!std::strcmp(mode,"wrong-position"))assert(s->diagnosticStage==3&&s->diagnosticCenter==169&&s->failure==cap::Failure::read);if(!std::strcmp(mode,"palette"))assert(s->diagnosticFontDiff&(1u<<9));if(!std::strcmp(mode,"bad-word"))assert(s->diagnosticBuilder==uint32_t(nfs_soft::FramePacketBuilder::Failure::wordMismatch));if(bool(packet)!=okay)std::fprintf(stderr,"mode=%s failure=%s\n",mode,cap::failureName(s->failure));assert(bool(packet)==okay);
  if(packet){assert(s->measureSeen&&s->measureUsed&&s->diagnosticMeasurePointerMatch&&s->measureAnchor==168);if(!std::strcmp(mode,"right"))assert(s->measureKind==cap::SourceMeasureAlignment::right&&s->measureWidth==48&&s->diagnosticAdvance==48&&s->diagnosticCenter==168);assert(packet->pixels()==343*480&&packet->surface().geometry[10]==686&&packet->commands()==1&&packet->writes()==1);assert(packet->original()[369*343+123]==0xffff&&packet->clean()[369*343+123]==0);assert(cap::acquireCurrentPacket(g)==packet);
   if(!std::strcmp(mode,"retire")){cap::beforeOwnerRetire(g,selected.owner);assert(!cap::lastPacket(g));}else if(!std::strcmp(mode,"unrelated")){cap::beforeOwnerRetire(g,0);cap::beforeOwnerRetire(g,0x9a000);assert(cap::lastPacket(g)==packet);}else{retained=packet;put<uint16_t>(selected.w[8]+16,1);assert(!cap::acquireCurrentPacket(g));assert(packet->original()[0]==0);}
  }else assert(s->failure!=cap::Failure::none);
  if(!std::strcmp(mode,"normal")){std::thread foreign([&]{foreignPath=true;Game other;other.eax=off_4D37C8;{cap::OwnerScope foreignOwner(other);assert(!cap::current(other));}foreignPath=false;});foreign.join();assert(foreignAllocations==0);}
 }
 prepare();g.eax=off_4D37C8;{cap::OwnerScope owner(g);{cap::TailScope tail(g);g.eax=off_4D37C8;{cap::OwnerScope nested(g);assert(!cap::current(g));}}assert(!cap::lastPacket(g));}
 prepare();g.eax=off_4D37C8;{cap::OwnerScope owner(g);{cap::TailScope tail(g);stringBody(g,"normal");}auto packet=cap::lastPacket(g);assert(packet);put<uint32_t>(dword_4D243C,123);assert(cap::acquireCurrentPacket(g)==packet);put<uint32_t>(dword_4D3E9C,0x80002);assert(!cap::acquireCurrentPacket(g));assert(packet->original()[369*343+123]==0xffff);}
 descriptorRetirement();nestedProofCases();
 assert(retained&&retained->pixels()==343*480&&retained->original()[369*343+123]==0xffff);assert(!leaseDepth&&allocationsInLease==0);std::puts("PASS Controls sidebar options caption: compact343 full replay/ownership/current-plane rejection, exact owner/font gate, original words, palette/font/mode/LUT/selection/retirement/nesting fallback, shared-LUT descriptor refs/lifetime, immutable lifetime, zero allocations in guest leases");
}
