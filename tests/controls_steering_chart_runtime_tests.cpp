// SPDX-License-Identifier: MIT
// Maintained chart GameHooks against synthetic guest memory. The SHA boundary
// below is explicitly mocked: this test proves collector controlflow/ownership,
// not real original-source hashing, artwork identity, execution or native output.
#include "controls_setup_runtime_fixture.h"
#define main steeringTextFixtureUnusedMain
#include "controls_steering_text_runtime_tests.cpp"
#undef main
static constexpr uint32_t dword_5636EC=0x338,dword_4DADA0=0x340,dword_4DADA8=0x348,game_id_sub_483288=0xbeef0010;
static constexpr uint32_t Archive=0x100000,ChartSource=Archive+32,ChartPalette=0x160000;
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
static void chartFrame(Game&g,const char*mode="normal"){
 steeringFrame(g,"normal",true);CHECK(steering::acquireCurrentPacket(g));
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
static void positives(){
 for(unsigned level=0;level<4;++level){prepareChart(level);Game g;g.eax=off_4D36A8;std::shared_ptr<const nfs_steering_chart::Packet>outside;{steering::OwnerScope owner(g,0x4101c0);CHECK(steering::current(g));chartFrame(g);auto p=chartLive::acquireCurrentPacket(g);CHECK(p&&p->rows()==49&&p->stores()>0&&p->binding().sensitivity==level&&p->sourceAddress()==ChartSource);CHECK(hashCalls==1&&!leaseDepth&&allocationsInLease==0);const auto retained=p;outside=p;
  // The converter's scratch palette has expired after the resolved draw. Its
  // global pointer and bytes grant no retained authority and are not reread.
  const auto savedScratch=load32(ChartPalette+4);put<uint32_t>(dword_5636EC,0);put<uint32_t>(ChartPalette+4,savedScratch^1);CHECK(chartLive::acquireCurrentPacket(g)==p);put<uint32_t>(ChartPalette+4,savedScratch);put<uint32_t>(dword_5636EC,ChartPalette);
  auto*s=steering::current(g);CHECK(s);const auto savedTarget=selected;
  memory[Plane+16]^=1;CHECK(!chartLive::acquireCurrentPacket(g));memory[Plane+16]^=1;CHECK(chartLive::acquireCurrentPacket(g)==p);
  memory[ChartSource+16]^=1;CHECK(!chartLive::acquireCurrentPacket(g));memory[ChartSource+16]^=1;CHECK(chartLive::acquireCurrentPacket(g)==p);
  memory[Archive+16]^=1;CHECK(!chartLive::acquireCurrentPacket(g));memory[Archive+16]^=1;CHECK(chartLive::acquireCurrentPacket(g)==p);
  selected.x[20]^=2;CHECK(!chartLive::acquireCurrentPacket(g));selected=savedTarget;CHECK(chartLive::acquireCurrentPacket(g)==p);
  ++s->serial;CHECK(!chartLive::acquireCurrentPacket(g));--s->serial;CHECK(chartLive::acquireCurrentPacket(g)==p);++s->lifetime;CHECK(!chartLive::acquireCurrentPacket(g));--s->lifetime;CHECK(chartLive::acquireCurrentPacket(g)==p);++s->ownerEpoch;CHECK(!chartLive::acquireCurrentPacket(g));--s->ownerEpoch;CHECK(chartLive::acquireCurrentPacket(g)==p);
  Game other=g;CHECK(!chartLive::acquireCurrentPacket(other));std::thread foreign([&]{Game another;CHECK(!chartLive::current(another)&&!chartLive::acquireCurrentPacket(another));});foreign.join();
  steering::beforeOwnerRetire(g,selected.owner);CHECK(!chartLive::acquireCurrentPacket(g));CHECK(retained->rows()==49&&retained->source().size()==18500&&retained->original().size()==307200);
  }CHECK(!chartLive::acquireCurrentPacket(g));CHECK(outside&&outside->rows()==49&&outside->source().size()==18500&&outside->original().size()==307200);
 }
}
int main(){CHECK(nfs_soft::installFontCatalog(font()));positives();for(const char*mode:{"nested-chart","hash-failure","wrong-digest","archive-signature","archive-size","archive-name","archive-offset","mode","origin","nested-resolved","unsupported","count","row-source","row-destination","nested-row","transparent-store","missing-store","wrong-game","wrong-word","palette-after","palette-pointer-after","callback-after","override-after","source-after","target-after","retire","descriptor-retire","authority-after","archive-after","plane-after","source-palette-after","unobserved-final"}){prepareChart();Game g;g.eax=off_4D36A8;{steering::OwnerScope owner(g,0x4101c0);CHECK(steering::current(g));chartFrame(g,mode);CHECK(!chartLive::acquireCurrentPacket(g));}CHECK(!leaseDepth&&allocationsInLease==0);}std::printf("PASS steering chart GameHooks checks=%u synthetic-only=1 SHA-boundary=mocked real-source-authentication=unproven native-renderer=none\n",checks);}
