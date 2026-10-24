// SPDX-License-Identifier: MIT
#include "hd/SteeringFramePacket.h"
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

namespace sf=nfs_steering_frame;
namespace sc=nfs_steering_chart;
struct ChartFixture {
 std::shared_ptr<const nfs_soft::FramePacket>text;
 sc::Binding binding{};std::vector<uint8_t>source;std::vector<uint16_t>post;
 std::shared_ptr<const sc::Packet>packet;
 ChartFixture(const Fixture&f,const std::vector<uint16_t>&clean,unsigned level=0,unsigned pattern=0){
  text=f.text(post,clean);CHECK(text);
  binding={f.current.lifetime,f.current.ownerEpoch,19,f.current.form,0x4101c0,f.current.archive,level,f.current.surface.geometry[8]};
  source.assign(18500,0);const uint8_t h[]={0x7b,0x34,0x44,0,0x64,1,0x31,0,0xec,0xfe,0xbc,0xff,0x14,1,0x44,0};
  const uint8_t a[]={0x2a,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0};
  std::memcpy(source.data(),h,16);std::memcpy(source.data()+17460,a,16);
  // Three independently encoded entries: skipped alpha15, opaque red, opaque
  // black. This fixture uses opaque stores so its final-plane oracle needs no
  // production blend/conversion helper.
  put32(source.data()+17476,0x0f123456);put32(source.data()+17480,0xffff0000);put32(source.data()+17484,0xff000000);
  sc::Builder::Palette palette{};palette[0]=0x0f0011aa;palette[1]=0xff00f800;palette[2]=0xff000000;
  for(size_t i=0;i<356*49;++i)source[16+i]=uint8_t(pattern==1?0:pattern==2?2:i%3);
  sc::Builder builder;CHECK(builder.begin(binding,text,0x900000,source));
  for(unsigned y=0;y<49;++y){sc::Builder::Row before{},after{};size_t at=(280+y)*640+280;std::copy_n(post.data()+at,356,before.data());after=before;
   const uint32_t destination=binding.targetBitmap+16+(280+y)*1280+560;
   CHECK(builder.beginRow(0x900010+y*356,destination,356,palette,before));
   for(unsigned x=0;x<356;++x){unsigned index=source[16+y*356+x];if(index){after[x]=index==1?0xf800:0;CHECK(builder.word(destination+2*x,before[x],after[x],after[x]));}}
   CHECK(builder.endRow(after,palette));std::copy(after.begin(),after.end(),post.begin()+at);
  }
  packet=builder.finish(binding,source,post.data(),post.size());CHECK(packet);
 }
};
static void completeJoins(){
 for(unsigned level=0;level<4;++level)for(unsigned pattern=0;pattern<3;++pattern){
  Fixture f;auto background=f.verify();CHECK(background);ChartFixture c(f,background->pixels(),level,pattern);
  auto p=sf::SteeringFramePacket::create(background,c.packet,f.current,c.binding.handler,19,c.post.data(),c.post.size());CHECK(p);
  CHECK(p->binding()==f.current&&p->publication()==19&&p->background()==background&&p->chart()==c.packet&&p->text()==c.text&&p->original()==c.post);
  CHECK(p->matches(f.current,c.binding.handler,19,c.post.data(),c.post.size()));
  unsigned expected=0;for(size_t i=16;i<17460;++i)if(c.source[17476+4*size_t(c.source[i])+3]>=16)++expected;
  CHECK(c.packet->stores()==expected);CHECK(expected==(pattern==1?0:pattern==2?17444:11629));
  // A changed chart proves the join accepts FINAL, rather than pre-chart text.
  if(pattern!=1){CHECK(c.post!=c.text->original());CHECK(!sf::SteeringFramePacket::create(background,c.packet,f.current,c.binding.handler,19,c.text->original().data(),gf::Pixels));}
  auto retained=p->original();std::weak_ptr<const sc::Packet>weak=c.packet;c.packet.reset();c.text.reset();background.reset();c.source.clear();c.post.clear();CHECK(!weak.expired());CHECK(p->original()==retained&&p->text()&&p->background());
 }
}
static void rejectedJoins(){
 Fixture f;auto background=f.verify();ChartFixture c(f,background->pixels());
 auto create=[&](const af::Binding&b,uint32_t handler,uint64_t serial,const uint16_t*plane,size_t count){return sf::SteeringFramePacket::create(background,c.packet,b,handler,serial,plane,count);};
 auto p=create(f.current,c.binding.handler,19,c.post.data(),c.post.size());CHECK(p);
 for(unsigned field=0;field<9;++field){auto b=f.current;if(field==0)++b.form;if(field==1)++b.owner;if(field==2)++b.archive;if(field==3)++b.ownerEpoch;if(field==4)++b.pass;if(field==5)++b.lifetime;if(field==6)++b.surface.lifetime;if(field==7)b.surface.geometry[8]+=2;if(field==8)b.owner=0;CHECK(!create(b,c.binding.handler,19,c.post.data(),c.post.size()));CHECK(!p->matches(b,c.binding.handler,19,c.post.data(),c.post.size()));}
 for(uint32_t handler:{0u,c.binding.handler+1}){CHECK(!create(f.current,handler,19,c.post.data(),c.post.size()));CHECK(!p->matches(f.current,handler,19,c.post.data(),c.post.size()));}
 for(uint64_t serial:{uint64_t(0),uint64_t(18),uint64_t(20)}){CHECK(!create(f.current,c.binding.handler,serial,c.post.data(),c.post.size()));CHECK(!p->matches(f.current,c.binding.handler,serial,c.post.data(),c.post.size()));}
 auto wrong=c.post;for(size_t at:{size_t(0),size_t(280*640+280),gf::Pixels-1}){wrong=c.post;wrong[at]^=1;CHECK(!create(f.current,c.binding.handler,19,wrong.data(),wrong.size()));CHECK(!p->matches(f.current,c.binding.handler,19,wrong.data(),wrong.size()));}
 CHECK(!create(f.current,c.binding.handler,19,nullptr,gf::Pixels));CHECK(!create(f.current,c.binding.handler,19,c.post.data(),gf::Pixels-1));
 CHECK(!sf::SteeringFramePacket::create({},c.packet,f.current,c.binding.handler,19,c.post.data(),c.post.size()));CHECK(!sf::SteeringFramePacket::create(background,{},f.current,c.binding.handler,19,c.post.data(),c.post.size()));
 auto dirty=background->pixels();dirty.back()^=1;ChartFixture disconnected(f,dirty);CHECK(!sf::SteeringFramePacket::create(background,disconnected.packet,f.current,disconnected.binding.handler,19,disconnected.post.data(),disconnected.post.size()));
 // Independently valid chart source/text evidence can still belong to another
 // owner. It must not join just because final pixels happen to be plausible.
 for(unsigned field=0;field<3;++field){auto other=f;if(field==0)++other.current.form;if(field==1)++other.current.archive;if(field==2)++other.current.ownerEpoch;ChartFixture unrelated(other,background->pixels());CHECK(!sf::SteeringFramePacket::create(background,unrelated.packet,f.current,unrelated.binding.handler,19,unrelated.post.data(),unrelated.post.size()));}
 // Source corruption is rejected by the actual chart issuer, before a join is
 // possible; no mutable-packet back door is manufactured for this test.
 auto corrupt=c.source;corrupt[17469]^=1;sc::Builder rejected;CHECK(!rejected.begin(c.binding,c.packet->text(),0x900000,corrupt));
}
int main(){static_assert(!std::is_default_constructible<sf::SteeringFramePacket>::value,"join issuer only");try{completeJoins();rejectedJoins();std::cout<<"PASS steering frame packet checks="<<checks<<" synthetic-only=1 native-authority=none\n";}catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}}
