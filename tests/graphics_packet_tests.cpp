// SPDX-License-Identifier: MIT
#include "hd/GraphicsFramePacket.h"
#include "hd/SoftwareFontBuiltin.h"
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <functional>
namespace gf=nfs_graphics_frame;
namespace af=nfs_audio_frame;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x))throw std::runtime_error(#x);}while(0)
static void put16(uint8_t*p,uint16_t v){p[0]=uint8_t(v);p[1]=uint8_t(v>>8);}
static void put32(uint8_t*p,uint32_t v){for(unsigned k=0;k<4;++k)p[k]=uint8_t(v>>(8*k));}
static af::Target target(unsigned width,uint32_t bitmap,uint32_t owner){
 af::Target t{};t.w[1]=width;t.w[2]=480;t.w[5]=width;t.w[6]=480;t.w[7]=0x7810;
 t.w[8]=bitmap;t.w[10]=width*2;t.w[11]=owner+0x10000;t.w[12]=owner+0x20000;
 t.header={{0x78,width|(480u<<16),0,0}};t.owner=owner;t.ownerWords=t.w;t.ownerState[3]=owner;
 for(unsigned x=0;x<width;++x)t.x[x]=x*2;
 for(unsigned y=0;y<480;++y)t.y[y]=16+y*width*2;
 return t;
}
struct Fixture {
 af::Binding current{};
 gf::SidebarBinding side{};
 gf::BackgroundRecord record{};
 std::vector<uint8_t>map;
 std::array<unsigned char,32>sha{};
 std::shared_ptr<const af::BackgroundCatalog>catalog;
 std::shared_ptr<const nfs_soft::FontCatalog>font;
 Fixture(){
  const auto full=target(640,0x100000,0x40000);
  current.form=0x8000;current.owner=full.owner;current.archive=0x700000;
  current.ownerEpoch=2;current.pass=3;current.lifetime=current.surface.lifetime=4;
  std::copy(full.w.begin(),full.w.end(),current.surface.geometry.begin());
  auto&g=current.surface.geometry;g[15]=2;g[16]=1278;g[17]=16;g[18]=1296;g[19]=613136;
  side.target=target(343,0x300000,0x50000);side.target.ownerState[1]=side.target.owner;side.target.ownerState[2]=0x60000;side.target.ownerState[3]=current.owner;side.handle=side.tableEntry=side.target.owner;side.lifetime=7;side.word=0;
  side.source.resize(gf::SidebarBytes);put32(side.source.data(),0x78);put32(side.source.data()+4,343u|(480u<<16));
  map.resize(614416);put32(map.data(),0x78);put32(map.data()+4,640u|(480u<<16));
  record.preReset.resize(gf::Pixels);record.postReset.resize(gf::Pixels);record.postSidebar.resize(gf::Pixels);
  // Spatially varying independent oracle catches shifts/stride errors. Every
  // fifth sidebar word is transparent, including the first word of the image.
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){
   size_t at=y*640+x;uint16_t m=uint16_t(1+(x*13+y*31)%65535);
   put16(map.data()+16+at*2,m);record.preReset[at]=uint16_t(m^0xffff);
   record.postReset[at]=record.postSidebar[at]=m;
   if(x<343){uint16_t s=(x+y)%5?uint16_t(0x8000|((x*3+y*7)&0x7fff)):0;
    put16(side.source.data()+16+(y*343+x)*2,s);if(s)record.postSidebar[at]=s;}
  }
  nfs_texture_sha256(map.data(),map.size(),sha.data());catalog=af::BackgroundCatalog::create(sha,map);
  record.binding=current;record.complete=1;record.begin=1;record.end=6;
  record.sidebarBefore=record.sidebarAfter=side;
  auto fill=[&](gf::ScopedDraw&d,uint64_t begin,uint32_t source,uint32_t mode,const std::vector<uint8_t>&bytes){
   d.entry=d.exit=current;d.before=d.after=full;d.begin=begin;d.end=begin+1;d.complete=1;
   d.args={{source,mode,0,0}};d.sourceBefore=d.sourceAfter=bytes;
  };
  fill(record.reset,2,0x800000,0x400,map);fill(record.sidebar,4,side.target.w[8],0x1000,side.source);
  font=nfs_soft::createBuiltinFontCatalog();
 }
 std::shared_ptr<const gf::VerifiedBackground>verify()const{return gf::BackgroundVerifier::verify(record,current,side,catalog);}
 std::shared_ptr<const nfs_soft::FramePacket>text(std::vector<uint16_t>&post,const std::vector<uint16_t>&clean)const{
  nfs_soft::FramePacketBuilder builder(font);if(!builder.begin(current.surface,19,clean.data(),clean.size()))throw std::runtime_error("text begin");
  nfs_soft::TextCommand c{};c.length=8;std::memcpy(c.text.data(),"GRAPHICS",8);c.sourceSize=18;c.x=350;c.y=100;c.advance=144;c.color=0xffff;
  post=clean;const size_t at=120*640+350;post[at]^=0xffff;
  if(!builder.beginString(c)||!builder.word(at,clean[at],post[at],post[at])||!builder.endString(c))throw std::runtime_error("text command");
  return builder.finish(current.surface,post.data(),post.size());
 }
};
int main(){
 static_assert(!std::is_default_constructible<gf::VerifiedBackground>::value,"verified issuer only");
 static_assert(!std::is_default_constructible<gf::GraphicsFramePacket>::value,"verified join only");
 try{
  auto f=std::unique_ptr<Fixture>(new Fixture);CHECK(f->current.valid()&&f->side.valid()&&f->catalog&&f->font);
  // Exercise the binding itself: equality against a recorded witness cannot
  // substitute for proving membership in the CURRENT selected retained slot.
  for(unsigned index=0;index<2;++index){
   auto side=f->side;side.word=index<<16;side.target.ownerState[0]=side.word;
   side.target.ownerState[1]=0x60000;side.target.ownerState[2]=0x70000;
   side.target.ownerState[1+index]=side.handle;
   CHECK(side.target.ownerState[3]!=side.handle&&side.valid());
   {auto wrong=side;wrong.target.ownerState[1+index]=0x80000;
    wrong.target.ownerState[2-index]=wrong.handle;
    // Owner still occurs in a sidebar slot, but not the selected one.
    CHECK(!wrong.valid());}
   {auto wrong=side;wrong.target.ownerState[1]=wrong.target.ownerState[2]=wrong.handle;
    wrong.target.ownerState[0]=(index^1u)<<16;
    // Both table entries match, isolating the selector/state0 mismatch.
    CHECK(!wrong.valid());}
   {auto wrong=side;wrong.target.ownerState[1]=0x60000;wrong.target.ownerState[2]=0x70000;
    wrong.target.ownerState[3]=wrong.handle;
    // A full640 slot cannot authorize a retained343 sidebar descriptor.
    CHECK(!wrong.valid());}
  }
  auto background=f->verify();CHECK(background);
  CHECK(background->pixels()==f->record.postSidebar&&background->sidebar()==f->side);
  CHECK(background->pixels()[0]==f->record.postReset[0]);
  CHECK(background->pixels()[343]==f->record.postReset[343]);
  std::vector<uint16_t>post;auto text=f->text(post,background->pixels());CHECK(text);
  auto packet=gf::GraphicsFramePacket::create(background,text,f->current,19,post.data(),post.size());CHECK(packet);
  CHECK(packet->matches(f->current,19,post.data(),post.size()));
  // Mutate one boundary fact per case while retaining independent current data.
  auto reject=[&](const std::function<void(gf::BackgroundRecord&)>&mutate){auto r=f->record;mutate(r);CHECK(!gf::BackgroundVerifier::verify(r,f->current,f->side,f->catalog));};
  reject([](gf::BackgroundRecord&r){r.complete=0;});
  reject([](gf::BackgroundRecord&r){r.error=1;});
  reject([](gf::BackgroundRecord&r){r.reset.complete=0;});
  reject([](gf::BackgroundRecord&r){r.sidebar.error=1;});
  reject([](gf::BackgroundRecord&r){r.begin=0;});
  reject([](gf::BackgroundRecord&r){r.reset.begin=r.begin;});
  reject([](gf::BackgroundRecord&r){r.reset.end=r.sidebar.begin;});
  reject([](gf::BackgroundRecord&r){r.sidebar.end=r.end;});
  reject([](gf::BackgroundRecord&r){r.reset.end=r.reset.begin;});
  reject([](gf::BackgroundRecord&r){r.reset.entry.ownerEpoch++;});
  reject([](gf::BackgroundRecord&r){r.sidebar.exit.pass++;});
  reject([](gf::BackgroundRecord&r){r.binding.form++;});
  reject([](gf::BackgroundRecord&r){r.reset.before.x[319]++;});
  reject([](gf::BackgroundRecord&r){r.reset.before.y[241]++;});
  reject([](gf::BackgroundRecord&r){r.reset.before.w[3]=1;});
  reject([](gf::BackgroundRecord&r){r.reset.before.ownerWords[8]++;});
  reject([](gf::BackgroundRecord&r){r.reset.before.ownerState[3]++;});
  reject([](gf::BackgroundRecord&r){r.sidebar.after.owner++;});
  reject([](gf::BackgroundRecord&r){r.sidebar.before.header[3]=1;});
  reject([](gf::BackgroundRecord&r){r.reset.sourceBefore[16]^=1;});
  reject([](gf::BackgroundRecord&r){r.reset.sourceBefore[16]^=1;r.reset.sourceAfter=r.reset.sourceBefore;});
  reject([](gf::BackgroundRecord&r){r.sidebar.sourceAfter[16]^=1;});
  reject([](gf::BackgroundRecord&r){r.sidebar.sourceBefore[18]^=1;r.sidebar.sourceAfter=r.sidebar.sourceBefore;});
  reject([](gf::BackgroundRecord&r){r.reset.args[0]=r.reset.before.w[8]+100;});
  reject([](gf::BackgroundRecord&r){r.reset.args[0]=r.sidebar.args[0];});
  reject([](gf::BackgroundRecord&r){r.reset.args[0]=0xffff0000;});
  reject([](gf::BackgroundRecord&r){r.reset.args[1]=0x1000;});
  reject([](gf::BackgroundRecord&r){r.sidebar.args[1]=0x400;});
  reject([](gf::BackgroundRecord&r){r.sidebar.args[2]=1;});
  reject([](gf::BackgroundRecord&r){r.reset.args[3]=1;});
  reject([](gf::BackgroundRecord&r){r.sidebarBefore.lifetime++;});
  reject([](gf::BackgroundRecord&r){r.sidebarAfter.handle++;});
  reject([](gf::BackgroundRecord&r){r.sidebarAfter.source[16]^=1;});
  reject([](gf::BackgroundRecord&r){r.preReset.pop_back();});
  reject([](gf::BackgroundRecord&r){r.postReset[21345]^=1;});
  reject([](gf::BackgroundRecord&r){r.postSidebar[0]=0;}); // transparent source must retain map
  reject([](gf::BackgroundRecord&r){r.postSidebar[343]^=1;}); // outside narrow source
  reject([](gf::BackgroundRecord&r){r.postSidebar[479*640+639]^=1;});
  reject([](gf::BackgroundRecord&r){r.postSidebar[1]=r.postReset[1];}); // nonzero source must replace
  // Coherent aliases must fail even when recorded and current data agree.
  {auto r=f->record;auto side=f->side;side.target.w[8]=f->current.surface.geometry[8];side.target.ownerWords[8]=side.target.w[8];
   CHECK(side.valid());r.sidebarBefore=r.sidebarAfter=side;r.sidebar.args[0]=side.target.w[8];
   CHECK(!gf::BackgroundVerifier::verify(r,f->current,side,f->catalog));}
  {auto r=f->record;auto side=f->side;side.target.owner=f->current.owner;side.target.ownerState[1]=side.target.owner;
   side.handle=side.tableEntry=side.target.owner;CHECK(side.valid());r.sidebarBefore=r.sidebarAfter=side;
   CHECK(!gf::BackgroundVerifier::verify(r,f->current,side,f->catalog));}
  reject([](gf::BackgroundRecord&r){r.sidebar.end=r.sidebar.begin;});
  reject([](gf::BackgroundRecord&r){r.sidebar.begin=r.sidebar.end+1;});
  CHECK(!gf::BackgroundVerifier::verify(f->record,f->current,f->side,{}));
  CHECK(!gf::GraphicsFramePacket::create({},text,f->current,19,post.data(),post.size()));
  CHECK(!gf::GraphicsFramePacket::create(background,{},f->current,19,post.data(),post.size()));
  // Independently supplied current source binding must be complete and current.
  for(unsigned k=0;k<9;++k){auto side=f->side;
   if(k==0)side.lifetime=0;if(k==1)side.lifetime++;
   if(k==2)side.target.x[200]++;if(k==3)side.target.y[400]++;
   if(k==4)side.target.w[5]=342;if(k==5)side.tableEntry++;
   if(k==6)side.word=2u<<16;if(k==7)side.source[16]^=1;
   if(k==8)side.target.header[2]=1;
   CHECK(!gf::BackgroundVerifier::verify(f->record,f->current,side,f->catalog));
  }
  for(unsigned k=0;k<6;++k){auto b=f->current;
   if(k==0)b.ownerEpoch++;if(k==1)b.pass++;if(k==2)b.archive++;
   if(k==3)b.lifetime++;if(k==4)b.surface.lifetime++;if(k==5)b.surface.geometry[10]++;
   CHECK(!gf::BackgroundVerifier::verify(f->record,b,f->side,f->catalog));
   CHECK(!gf::GraphicsFramePacket::create(background,text,b,19,post.data(),post.size()));
  }
  auto badSha=f->sha;badSha[0]^=1;CHECK(!af::BackgroundCatalog::create(badSha,f->map));
  for(unsigned header=0;header<4;++header){auto bytes=f->map;bytes[header*4]^=1;std::array<unsigned char,32>sha{};nfs_texture_sha256(bytes.data(),bytes.size(),sha.data());CHECK(!af::BackgroundCatalog::create(sha,bytes));}
  CHECK(!gf::GraphicsFramePacket::create(background,text,f->current,20,post.data(),post.size()));
  CHECK(!gf::GraphicsFramePacket::create(background,text,f->current,19,post.data(),post.size()-1));
  CHECK(!gf::GraphicsFramePacket::create(background,text,f->current,19,nullptr,post.size()));
  auto changed=post;changed[55]^=1;CHECK(!gf::GraphicsFramePacket::create(background,text,f->current,19,changed.data(),changed.size()));
  auto wrongClean=background->pixels();wrongClean[33]^=1;std::vector<uint16_t>wrongPost;
  auto wrongText=f->text(wrongPost,wrongClean);CHECK(wrongText);
  CHECK(!gf::GraphicsFramePacket::create(background,wrongText,f->current,19,wrongPost.data(),wrongPost.size()));
  // Retained publication owns every source and text plane after all mutable
  // captures, caller arrays and catalog references have been destroyed.
  const auto savedMap=packet->background()->catalog()->source();const auto savedSide=packet->background()->sidebar().source;
  std::weak_ptr<const af::BackgroundCatalog>weak=f->catalog;
  f->map.assign(f->map.size(),0);f->side.source.assign(f->side.source.size(),0);f.reset();
  background.reset();text.reset();post.clear();
  CHECK(!weak.expired());CHECK(packet->background()->catalog()->source()==savedMap);
  CHECK(packet->background()->sidebar().source==savedSide);CHECK(packet->text()->original().size()==gf::Pixels);
  CHECK(packet->background()->pixels()==packet->text()->clean());
  packet.reset();CHECK(weak.expired());
  std::cout<<"PASS Graphics complete-frame packet "<<checks<<" checks\n";return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}
}
