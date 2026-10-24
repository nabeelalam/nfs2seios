// SPDX-License-Identifier: MIT
// Maintained chart GameHooks against synthetic guest memory. The SHA boundary
// below is explicitly mocked: this test proves collector controlflow/ownership,
// not real original-source hashing, artwork identity, execution or native output.
#include "controls_setup_runtime_fixture.h"
// Load all map/catalog/hash-bearing neutral helpers before the chart-only mock.
#include "hd/GraphicsFramePacket.h"
#include "hd/BackgroundSceneCatalog.h"
#include "hd/SteeringFramePacket.h"
// Preload every standard header used by the namespaced fixture globally.
#include <cstdio>
#include <memory>
#include <stdexcept>
#undef CHECK
namespace map_fixture {
#include "HdSceneFixture.h"
}
#undef CHECK
#define CHECK(v) do{++checks;if(!(v)){std::fprintf(stderr,"FAIL steering live line%d: %s\n",__LINE__,#v);std::abort();}}while(0)
#define main steeringTextFixtureUnusedMain
#include "controls_steering_text_runtime_tests.cpp"
#undef main
static constexpr uint32_t dword_5636EC=0x338,dword_4DADA0=0x340,dword_4DADA8=0x348,game_id_sub_483288=0xbeef0010;
static constexpr uint32_t Archive=0x200000,ChartSource=Archive+48+614416,ChartPalette=0x160000;
static std::array<uint8_t,18500>syntheticSource{};static unsigned hashCalls=0,hashLevel=0;static bool hashEnabled=true,wrongDigest=false;
static const std::array<std::array<uint8_t,32>,4>mockDigests{{
 {{0xf3,0x3b,0xc7,0x83,0x5f,0x16,0x74,0x04,0x35,0xc3,0x96,0x1a,0x47,0x12,0x1b,0x83,0xc3,0xb1,0xef,0xaf,0x70,0x1b,0x02,0xa7,0xef,0x3f,0xbf,0xbc,0x77,0x7b,0x7b,0xd2}},
 {{0xda,0x54,0xa4,0x7c,0x71,0xd8,0x73,0x22,0x4b,0xfd,0x09,0x45,0x96,0xe0,0x59,0xad,0x31,0xf2,0xbe,0xa4,0xb2,0x60,0x26,0xc3,0x0e,0xd2,0x8e,0x90,0xcb,0x36,0x8b,0xdb}},
 {{0xd6,0xed,0x54,0xf0,0x1c,0xf9,0x3a,0x96,0xf7,0x6e,0xb1,0x5f,0x38,0x29,0x13,0xcc,0x84,0x0e,0x6d,0xd8,0x55,0x99,0x6b,0xdd,0x8d,0x4a,0xc2,0xf4,0x44,0xce,0xa7,0xaa}},
 {{0x85,0x12,0xaa,0x7f,0x49,0x1e,0xcc,0x6f,0xdf,0x46,0xcb,0xd8,0x99,0x60,0xd0,0x2d,0xd4,0xb3,0xb1,0x0d,0xe7,0x21,0x4f,0xaa,0x06,0xf8,0x68,0x5d,0xad,0xaf,0x49,0xc9}}
}};
static bool syntheticSteeringSha(const unsigned char*data,size_t count,unsigned char*out){
 CHECK(!leaseDepth);++hashCalls;if(!hashEnabled||!data||!out||count!=syntheticSource.size()||std::memcmp(data,syntheticSource.data(),count))return false;
 std::memcpy(out,mockDigests[wrongDigest?(hashLevel+1)%4:hashLevel].data(),32);return true;
}
#define nfs_texture_sha256 syntheticSteeringSha
#include "hd/ControlsSteeringChartGameHooks.inc"
#undef nfs_texture_sha256
namespace chartLive=nfs_controls_steering_chart_live;
static uint32_t load32(uint32_t at){uint32_t v;std::memcpy(&v,memory.data()+at,4);return v;}
static uint16_t load16(uint32_t at){uint16_t v;std::memcpy(&v,memory.data()+at,2);return v;}
static uint16_t indexedReference(uint32_t ebx,uint16_t before){if(ebx<0x10000000u)return before;if(ebx>=0xfc000000u)return uint16_t(ebx);uint32_t eax=before,edx=eax,ecx=ebx;ecx^=0xffffffffu;eax<<=16;eax=(eax&0xffffff00u)|(edx&255u);edx&=0x7e0;edx<<=8;eax&=0xf800001fu;ecx>>=24;eax+=edx;const uint64_t product=uint64_t(eax)*ecx;eax=uint32_t(product);edx=uint32_t(product>>32);edx<<=8;ecx=eax;eax>>=16;edx&=0xf800;ecx>>=8;eax&=0x7e0;ebx+=edx;ecx&=31;ebx+=eax;ebx+=ecx;return uint16_t(ebx);}
static void prepareChart(unsigned level=0){
 prepareSteering(level);hashLevel=level;hashCalls=0;hashEnabled=true;wrongDigest=false;syntheticSource.fill(0);
 const uint8_t header[]={0x7b,0x34,0x44,0,0x64,1,0x31,0,0xec,0xfe,0xbc,0xff,0x14,1,0x44,0},attachment[]={0x2a,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0};std::memcpy(syntheticSource.data(),header,16);std::memcpy(syntheticSource.data()+17460,attachment,16);
 for(unsigned i=0;i<356*49;++i)syntheticSource[16+i]=uint8_t(i);
 const unsigned alphas[]={0,15,16,128,251,252,255};for(unsigned i=0;i<256;++i){const unsigned a=alphas[i%7],r=(i*11)&255,green=(i*13)&255,blue=(i*17)&255;const uint32_t raw=(a<<24)|(r<<16)|(green<<8)|blue;for(unsigned j=0;j<4;++j)syntheticSource[17476+4*i+j]=uint8_t(raw>>(8*j));put<uint32_t>(ChartPalette+4*i,(a<<24)|((r>>3)<<11)|((green>>2)<<5)|(blue>>3));}
 put<uint32_t>(dword_4D40B8,Archive);put<uint32_t>(Archive,0x49504853);put<uint32_t>(Archive+4,18532);put<uint32_t>(Archive+8,1);put<uint32_t>(Archive+12,0);const uint32_t names[]={0x6c756673,0x72687473,0x666c6873,0x74727173};put<uint32_t>(Archive+16,names[level]);put<uint32_t>(Archive+20,32);std::memcpy(memory.data()+ChartSource,syntheticSource.data(),syntheticSource.size());
 put<uint32_t>(dword_5636EC,ChartPalette);put<uint32_t>(dword_4DADA0,game_id_sub_483288);put<uint32_t>(dword_4DADA8,0x7b);
}
static void joinedTextTail(Game&g);
static void chartFrame(Game&g,const char*mode="normal"){
 joinedTextTail(g);CHECK(steering::acquireCurrentPacket(g));
 {chartLive::ChartScope chart(g);
  if(!std::strcmp(mode,"nested-chart")){chartLive::ChartScope nested(g);}
  if(!std::strcmp(mode,"hash-failure"))hashEnabled=false;if(!std::strcmp(mode,"wrong-digest"))wrongDigest=true;
  if(!std::strcmp(mode,"archive-signature"))put<uint32_t>(Archive,0);if(!std::strcmp(mode,"archive-size"))put<uint32_t>(Archive+4,18531);if(!std::strcmp(mode,"archive-name"))put<uint32_t>(Archive+16,0);if(!std::strcmp(mode,"archive-offset"))put<uint32_t>(Archive+20,33);
  g.eax=ChartSource;g.edx=0x3000;g.ebx=280;g.ecx=280;if(!std::strcmp(mode,"mode"))g.edx=0;if(!std::strcmp(mode,"origin"))g.ecx=281;
  {chartLive::ResolvedScope resolved(g);
   if(!std::strcmp(mode,"nested-resolved")){chartLive::ResolvedScope nested(g);}
   if(!std::strcmp(mode,"unsupported")){chartLive::UnsupportedRowScope bad(g);}
   for(unsigned row=0;row<49;++row){const uint32_t destination=Plane+16+(280+row)*1280+560;g.eax=ChartSource+16+row*356;g.edx=destination;g.ebx=356;
    if(row==0&&!std::strcmp(mode,"count"))g.ebx=355;if(row==0&&!std::strcmp(mode,"row-source"))g.eax+=1;if(row==0&&!std::strcmp(mode,"row-destination"))g.edx+=2;
    {chartLive::RowScope scope(g,game_id_sub_483288);
     if(row==0&&!std::strcmp(mode,"nested-row")){chartLive::RowScope nested(g,game_id_sub_483288);}
     for(unsigned x=0;x<356;++x){const uint32_t palette=load32(ChartPalette+4*memory[ChartSource+16+row*356+x]);const uint32_t at=destination+2*x;const uint16_t before=load16(at),after=indexedReference(palette,before);
      if(palette<0x10000000u){if(row==0&&x==0&&!std::strcmp(mode,"transparent-store")){chartLive::WordCapture unexpected(g,at,before);put<uint16_t>(at,before);}continue;}
      if(row==0&&x==2&&!std::strcmp(mode,"missing-store")){put<uint16_t>(at,after);continue;}
      if(row==0&&x==2&&!std::strcmp(mode,"wrong-game")){Game other=g;chartLive::WordCapture wrong(other,at,after);put<uint16_t>(at,after);continue;}
      {chartLive::WordCapture word(g,at,after);put<uint16_t>(at,row==0&&x==2&&!std::strcmp(mode,"wrong-word")?uint16_t(after^1):after);}
     }
     if(row==0){if(!std::strcmp(mode,"palette-after"))memory[ChartPalette+4]^=1;if(!std::strcmp(mode,"palette-pointer-after"))put<uint32_t>(dword_5636EC,ChartPalette+4);if(!std::strcmp(mode,"callback-after"))put<uint32_t>(dword_4DADA0,0);if(!std::strcmp(mode,"override-after"))put<uint32_t>(dword_4DADA8,0x7d);if(!std::strcmp(mode,"source-after"))memory[ChartSource+16]^=1;if(!std::strcmp(mode,"target-after"))selected.x[300]^=2;if(!std::strcmp(mode,"retire"))steering::beforeOwnerRetire(g,selected.owner);if(!std::strcmp(mode,"descriptor-retire"))steering::beforeDescriptorRetire(g,selected.owner);if(!std::strcmp(mode,"authority-after"))put<uint8_t>(byte_512E00,1);if(!std::strcmp(mode,"archive-after"))memory[Archive+16]^=1;if(!std::strcmp(mode,"plane-after"))put<uint16_t>(Plane+16,1);}
    }
   }
   if(!std::strcmp(mode,"source-palette-after"))memory[ChartSource+17476]^=1;
  }
  if(!std::strcmp(mode,"unobserved-final"))put<uint16_t>(Plane+18,1);
 }
}

// Only the indexed chart's pinned proprietary digest is mocked above. Map SHA,
// map catalogue validation, full-plane replay and immutable joins remain real.
static constexpr uint32_t dword_4D2314=0x380,dword_4D231C=0x390,SideBitmap=0x300000;
static nfs_logo_capture::Target retainedSide{};
namespace nfs_logo_publisher {
inline void target(nfs::GuestCopyTransaction&,nfs_logo_capture::Target&t,uint32_t owner){++reads;if(owner!=retainedSide.owner)throw 1;t=retainedSide;}
}
#include "hd/ControlsSteeringLiveGameHooks.inc"
namespace live=nfs_controls_steering_live;
static const map_fixture::Fixture&mapFixture(){static const auto f=std::unique_ptr<map_fixture::Fixture>(new map_fixture::Fixture);return *f;}
static void prepareFull(unsigned level=0,unsigned slot=0){
 prepareChart(level);const auto&f=mapFixture();
 const uint32_t mapOffset=48,chartOffset=48+614416;put<uint32_t>(Archive,0x49504853);put<uint32_t>(Archive+4,chartOffset+18500);put<uint32_t>(Archive+8,2);put<uint32_t>(Archive+12,0);
 std::memcpy(memory.data()+aBlk4_0,"test",4);std::memcpy(memory.data()+Archive+16,"test",4);put<uint32_t>(Archive+20,mapOffset);
 const uint32_t names[]={0x6c756673,0x72687473,0x666c6873,0x74727173};put<uint32_t>(Archive+24,names[level]);put<uint32_t>(Archive+28,chartOffset);
 std::memcpy(memory.data()+Archive+mapOffset,f.base.sourceBefore.data(),614416);std::memcpy(memory.data()+ChartSource,syntheticSource.data(),18500);
 retainedSide={};auto&w=retainedSide.w;w[1]=343;w[2]=480;w[5]=343;w[6]=480;w[7]=0x7810;w[8]=SideBitmap;w[10]=686;w[11]=0x15000;w[12]=0x16000;retainedSide.owner=0x1e000;retainedSide.ownerWords=w;retainedSide.ownerState[0]=slot<<16;retainedSide.ownerState[1+slot]=retainedSide.owner;retainedSide.ownerState[2-slot]=0x17800;retainedSide.ownerState[3]=selected.owner;
 retainedSide.header={{0x78,343u|(480u<<16),0,0}};for(unsigned x=0;x<343;++x)retainedSide.x[x]=2*x;for(unsigned y=0;y<480;++y)retainedSide.y[y]=16+686*y;
 for(unsigned k=0;k<4;++k)put<uint32_t>(SideBitmap+4*k,retainedSide.header[k]);for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x)put<uint16_t>(SideBitmap+16+2*(y*343+x),(x+y)%5?uint16_t(0x6000|((x*3+y*7)&0xfff)):0);
 put<uint32_t>(dword_4D2314+2,slot<<16);put<uint32_t>(dword_4D231C+4*slot,retainedSide.owner);put<uint32_t>(dword_4D231C+4*(slot^1u),0x17800);selected.ownerState=retainedSide.ownerState;CHECK(retainedSide.owner!=selected.owner);
}
static void originalBackground(Game&g,const char*mode){
 auto*s=live::current(g);CHECK(s);g.eax=aBlk4_0;g.edx=0x400;g.ebx=g.ecx=0;
 {live::ResetScope reset(g);CHECK(reset.active);g.eax=s->mapSource;
  {live::ResolvedScope resolved(g);std::memcpy(memory.data()+Plane+16,memory.data()+s->mapSource+16,614400);
   if(!std::strcmp(mode,"map-source"))memory[s->mapSource+17]^=1;if(!std::strcmp(mode,"reset-plane"))memory[Plane+17]^=1;
  }
 }
 if(s->invalid)return;
 g.eax=SideBitmap;g.edx=0x1000;g.ebx=g.ecx=0;
 {live::SidebarScope sidebar(g);if(!std::strcmp(mode,"nested-sidebar")){live::SidebarScope nested(g);}if(s->invalid)return;CHECK(sidebar.active);
  {live::ResolvedScope resolved(g);for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){const uint16_t value=load16(SideBitmap+16+2*(y*343+x));if(value)put<uint16_t>(Plane+16+2*(y*640+x),value);}
   if(!std::strcmp(mode,"side-source"))memory[SideBitmap+17]^=1;if(!std::strcmp(mode,"side-plane"))memory[Plane+17]^=1;
  }
 }
}
static void joinedTextTail(Game&g){
 g.eax=off_4D36A8;live::beginTail(g);steering::beginTail(g);
 std::memcpy(memory.data()+Text,"sensitivity",12);g.eax=0x153;{steering::PolicyScope policy(g,false);g.eax=0;steering::resourceAlignment(g);put<uint32_t>(dword_4DD714,65535);g.eax=Text;g.edx=369;g.ebx=144;
 {steering::StringScope string(g);live::bindText(g);{steering::GlyphScope glyph(g);const uint32_t at=Plane+16+146*1280+742;{steering::WordCapture word(g,at,65535);put<uint16_t>(at,65535);}}put<int32_t>(dword_4DD6EC,435);}
 } // End source policy before completing the original text tail.
 steering::finishTail(g);live::finishTail(g);
}
static void positivesFull(){
 for(unsigned level=0;level<4;++level)for(unsigned slot=0;slot<2;++slot){prepareFull(level,slot);Game g;g.eax=off_4D36A8;std::shared_ptr<const nfs_steering_frame::SteeringFramePacket>retained;
  {steering::OwnerScope textOwner(g,0x4101c0);live::OwnerScope owner(g);auto*s=live::current(g);CHECK(s&&!s->invalid);originalBackground(g,"normal");CHECK(!s->invalid&&s->verifiedBackground);chartFrame(g);live::finishChart(g);auto p=live::acquireCurrentPacket(g);CHECK(p&&p->chart()->rows()==49&&p->chart()->binding().sensitivity==level&&p->original()==p->chart()->original());retained=p;
   CHECK(!steering::acquireCurrentPacket(g));CHECK(p->text()->original()!=p->original());nfs_graphics_frame::SidebarBinding sidebar;CHECK(live::acquireCurrentSidebarBinding(g,sidebar)&&sidebar.handle==retainedSide.owner&&sidebar.word==(slot<<16));
   memory[Plane+17]^=1;CHECK(!live::acquireCurrentPacket(g));memory[Plane+17]^=1;CHECK(live::acquireCurrentPacket(g)==p);
   memory[SideBitmap+17]^=1;CHECK(!live::acquireCurrentPacket(g)&&!live::acquireCurrentSidebarBinding(g,sidebar));memory[SideBitmap+17]^=1;CHECK(live::acquireCurrentPacket(g)==p);
   memory[Archive+16]^=1;CHECK(!live::acquireCurrentPacket(g));memory[Archive+16]^=1;CHECK(live::acquireCurrentPacket(g)==p);
   put<uint8_t>(byte_512E00,uint8_t((level+1)%4));CHECK(!live::acquireCurrentPacket(g));put<uint8_t>(byte_512E00,uint8_t(level));CHECK(live::acquireCurrentPacket(g)==p);
   Game other=g;CHECK(!live::acquireCurrentPacket(other));const auto before=reads;std::thread foreign([&]{Game wrong;CHECK(!live::current(wrong)&&!live::acquireCurrentPacket(wrong));});foreign.join();CHECK(reads==before);
   steering::beforeOwnerRetire(g,selected.owner);live::afterRetire(g);CHECK(!live::acquireCurrentPacket(g)&&s->invalid);CHECK(p->original().size()==307200);
  }CHECK(!live::acquireCurrentPacket(g));CHECK(retained&&retained->background()&&retained->text()&&retained->chart());CHECK(!leaseDepth&&allocationsInLease==0);
 }
}
static void failuresFull(){
 for(const char*mode:{"map-source","reset-plane","side-source","side-plane","nested-sidebar"}){prepareFull();Game g;g.eax=off_4D36A8;steering::OwnerScope textOwner(g,0x4101c0);live::OwnerScope owner(g);CHECK(live::current(g));originalBackground(g,mode);CHECK(!live::acquireCurrentPacket(g)&&live::current(g)->invalid);CHECK(!leaseDepth&&allocationsInLease==0);}
 for(const char*mode:{"missing-store","wrong-word","source-after","palette-after","unobserved-final"}){prepareFull();Game g;g.eax=off_4D36A8;steering::OwnerScope textOwner(g,0x4101c0);live::OwnerScope owner(g);originalBackground(g,"normal");chartFrame(g,mode);live::finishChart(g);CHECK(!live::acquireCurrentPacket(g)&&live::current(g)->invalid);CHECK(!leaseDepth&&allocationsInLease==0);}
 prepareFull();Game g;g.eax=off_4D36A8;steering::OwnerScope textOwner(g,0x4101c0);live::OwnerScope owner(g);CHECK(live::current(g));live::finishChart(g);CHECK(live::current(g)->invalid&&!live::acquireCurrentPacket(g));
}
int main(){CHECK(nfs_soft::installFontCatalog(font()));auto catalogue=std::make_shared<const nfs_background_scene_cpu::SourceCatalog>(mapFixture().catalog);CHECK(nfs_background_scene_runtime::installCatalog(catalogue));positivesFull();failuresFull();CHECK(!leaseDepth&&allocationsInLease==0);std::printf("PASS steering full640 live checks=%u synthetic-only=1 map-sha=real chart-sha=mocked native-authority=none\n",checks);}
