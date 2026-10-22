// SPDX-License-Identifier: MIT
#include "hd/SoftwareFramePacket.h"
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
using namespace nfs_soft;
struct Fixture{
 FontDescription d;std::vector<uint8_t>a=std::vector<uint8_t>(1024*512);
 Fixture(){for(unsigned r=0;r<2;++r){d.baseline[r]=2;for(auto&g:d.glyphs[r])g={2,2,8,8,0,-8,3*65536};d.glyphs[r][0]={0,0,0,0,0,0,2*65536};d.kern[r][33*96+34]=-32768;}for(unsigned y=2;y<10;++y)for(unsigned x=2;x<10;++x)a[y*1024+x]=uint8_t(x+y);}
 std::shared_ptr<const FontCatalog>font()const{return FontCatalog::create(d,a.data(),a.size(),17);}
};
static SurfaceIdentity identity(unsigned width=640){SurfaceIdentity s;s.lifetime=1;auto&g=s.geometry;g[1]=width;g[2]=480;g[5]=width;g[6]=480;g[7]=16;g[8]=0x10000;g[10]=width*2;g[11]=0x20000;g[12]=0x30000;g[14]=0;g[15]=2;g[16]=2*(width-1);g[17]=16;g[18]=16+width*2;g[19]=16+479*width*2;return s;}
static TextCommand command(){TextCommand c;c.text[0]='A';c.text[1]='B';c.length=2;c.sourceSize=18;c.x=10;c.y=20;c.advance=6;c.color=0xf81f;return c;}
static std::vector<uint16_t>plane(){std::vector<uint16_t>p(FramePacket::pixelCount);for(size_t i=0;i<p.size();++i)p[i]=uint16_t(i*13);return p;}
static void fontTests(){Fixture f;auto font=f.font();CHECK(font);CHECK(font->alpha()==f.a);f.a[2050]=0;f.d.baseline[0]=5;CHECK(font->alpha()[2050]==4);CHECK(font->description().baseline[0]==2);
 Fixture good;CHECK(!FontCatalog::create(good.d,good.a.data(),good.a.size(),0));CHECK(!FontCatalog::create(good.d,nullptr,good.a.size(),1));CHECK(!FontCatalog::create(good.d,good.a.data(),good.a.size()-1,1));
 auto d=good.d;d.glyphs[0][33].x=0;CHECK(!FontCatalog::create(d,good.a.data(),good.a.size(),1));d=good.d;d.glyphs[0][33].x=1016;CHECK(!FontCatalog::create(d,good.a.data(),good.a.size(),1));d=good.d;d.glyphs[0][33].h=0;CHECK(!FontCatalog::create(d,good.a.data(),good.a.size(),1));d=good.d;d.glyphs[0][33].advance=-1;CHECK(!FontCatalog::create(d,good.a.data(),good.a.size(),1));d=good.d;d.baseline[1]=513;CHECK(!FontCatalog::create(d,good.a.data(),good.a.size(),1));d=good.d;d.kern[0][0]=std::numeric_limits<int32_t>::max();CHECK(!FontCatalog::create(d,good.a.data(),good.a.size(),1));good.a[1025]=1;CHECK(!good.font());}
static void layoutTests(){Fixture f;auto font=f.font();const uint8_t ab[]={'A','B',0};Layout l;CHECK(layout(*font,ab,3,0,10,20,0,0,{10,20,14,22},l));CHECK(l.count==2&&l.length==2&&l.advance==360448);CHECK(l.positions[0]==0&&l.positions[1]==163840);CHECK(l.quads[0].left==10&&l.quads[0].top==20&&l.quads[0].right==12.25f&&l.quads[0].bottom==22);CHECK(l.quads[1].left==12.25f&&l.quads[1].right==14);CHECK(l.quads[0].u0==2.0f/1024&&l.quads[0].v0==2.0f/512);CHECK(l.quads[0].u1==11.0f/1024); // one gutter texel and clipped quarter-pixel
 Layout before=l;CHECK(!layout(*font,ab,2,0,0,0,0,0,{0,0,640,480},l));CHECK(std::memcmp(&before,&l,sizeof l)==0);CHECK(!layout(*font,ab,3,2,0,0,0,0,{0,0,640,480},l));CHECK(!layout(*font,ab,3,0,NAN,0,0,0,{0,0,640,480},l));CHECK(!layout(*font,ab,3,0,0,0,4097,0,{0,0,640,480},l));const uint8_t bad[]={1,0};CHECK(!layout(*font,bad,2,0,0,0,0,0,{0,0,640,480},l));const uint8_t special[]={224,0};CHECK(layout(*font,special,2,1,0,0,0,0,{0,0,640,480},l));
 CHECK(layout(*font,ab,3,0,10,20,2.75f,0,{0,0,640,480},l));CHECK(l.fittedWidth==2.75f); // shrink full logical width
 CHECK(layout(*font,ab,3,0,10,20,10,1,{0,0,640,480},l));CHECK(l.quads[0].left==12.0f);CHECK(layout(*font,ab,3,0,10,20,10,2,{0,0,640,480},l));CHECK(l.quads[0].left==14.25f);
 Vertex v[12]{};size_t used=0;CHECK(append_vertices(l,0x8410,v,12,used));CHECK(used==12);CHECK(v[0].r==132.0f/255&&v[0].g==130.0f/255&&v[0].b==132.0f/255&&v[0].a==1);Vertex copy[12];std::memcpy(copy,v,sizeof v);size_t count=0;l.quads[1].u1=2;CHECK(!append_vertices(l,0,v,12,count));CHECK(count==0&&std::memcmp(v,copy,sizeof v)==0);
}
static std::shared_ptr<const FramePacket>makePacket(FramePacketBuilder&b,std::vector<uint16_t>&p,uint64_t serial=1){auto id=identity();CHECK(b.begin(id,serial,p.data(),p.size()));auto c=command();CHECK(b.beginString(c));const auto old=p[100];CHECK(b.word(100,old,0x1234,0x1234));CHECK(b.word(100,0x1234,0x4321,0x4321));p[100]=0x4321;CHECK(b.endString(c));return b.finish(id,p.data(),p.size());}
static void packetTests(){Fixture f;auto font=f.font();std::weak_ptr<const FontCatalog>weak=font;FramePacketBuilder b(font);auto original=plane();auto p=original;auto packet=makePacket(b,p);CHECK(packet);CHECK(packet->clean()==original&&packet->original()==p);CHECK(packet->writes()==2&&packet->commands()==1&&packet->vertices().size()==12);CHECK(packet->font()==font);CHECK(packet->vertices()[0].x==9.75f&&packet->vertices()[0].y==41.75f);CHECK(packet->vertices()[0].r==1&&packet->vertices()[0].g==0&&packet->vertices()[0].b==1);
 CHECK(packet->matches(identity(),1,p.data(),p.size()));auto id=identity();id.lifetime++;CHECK(!packet->matches(id,1,p.data(),p.size()));CHECK(!packet->matches(identity(),2,p.data(),p.size()));p[101]^=1;CHECK(!packet->matches(identity(),1,p.data(),p.size()));CHECK(packet->original()[101]==original[101]);CHECK(!b.begin(identity(),1,p.data(),p.size()));
 std::function<bool()>queued;{auto owned=packet;queued=[owned](){return owned->font()->alpha()[2050]==4&&owned->clean()[100]!=owned->original()[100];};}packet.reset();font.reset();CHECK(!weak.expired());CHECK(queued());queued={}; // builder still retains font until its own lifetime ends
 for(unsigned mode=0;mode<12;++mode){auto q=plane();FramePacketBuilder builder(f.font());auto c=command();auto s=identity();CHECK(builder.begin(s,1,q.data(),q.size()));CHECK(builder.beginString(c));if(mode==0){CHECK(!builder.word(0,q[0]^1,1,1));continue;}if(mode==1){CHECK(!builder.word(0,q[0],1,2));continue;}if(mode==2){CHECK(!builder.word(q.size(),0,1,1));continue;}CHECK(builder.word(100,q[100],1,1));q[100]=1;if(mode==3){CHECK(!builder.finish(s,q.data(),q.size()));continue;}if(mode==4){CHECK(!builder.beginString(c));continue;}if(mode==5)c.x++;if(mode==6)c.text[0]='Z';if(mode==7)c.advance=0;if(mode==8)c.role=2;if(mode>=5&&mode<=8){CHECK(!builder.endString(c));continue;}CHECK(builder.endString(c));if(mode==9)s.lifetime++;if(mode==10)q[0]^=1;if(mode==11){builder.invalidate();}CHECK(!builder.finish(s,q.data(),q.size()));}
 auto q=plane();FramePacketBuilder nofont({});CHECK(!nofont.begin(identity(),1,q.data(),q.size()));FramePacketBuilder empty(f.font());CHECK(empty.begin(identity(),1,q.data(),q.size()));CHECK(empty.beginString(command()));CHECK(!empty.endString(command()));
 FramePacketBuilder bad(f.font());id=identity();id.geometry[15]=3;CHECK(!bad.begin(id,1,q.data(),q.size()));id=identity();id.geometry[8]=0xfffffff0;CHECK(!bad.begin(id,1,q.data(),q.size()));
 std::weak_ptr<const FontCatalog>ownedWeak;std::function<bool()>late;{auto ownedFont=f.font();ownedWeak=ownedFont;FramePacketBuilder temp(ownedFont);auto data=plane();auto result=makePacket(temp,data);late=[result]{return result->font()->identity()==17;};}CHECK(!ownedWeak.expired());CHECK(late());late={};CHECK(ownedWeak.expired());
}
static void capacityTests(){Fixture f;auto data=plane();const auto id=identity();auto c=command();FramePacketBuilder b(f.font());CHECK(b.begin(id,1,data.data(),data.size()));for(size_t i=0;i<FramePacketBuilder::maxCommands;++i){CHECK(b.beginString(c));CHECK(b.word(0,data[0],data[0],data[0]));CHECK(b.endString(c));}CHECK(!b.beginString(c));CHECK(!b.finish(id,data.data(),data.size()));
 FramePacketBuilder writes(f.font());CHECK(writes.begin(id,1,data.data(),data.size()));CHECK(writes.beginString(c));for(size_t i=0;i<FramePacketBuilder::maxWrites;++i)CHECK(writes.word(0,data[0],data[0],data[0]));CHECK(!writes.word(0,data[0],data[0],data[0]));CHECK(!writes.endString(c));
 auto font=f.font();Layout l;const uint8_t ab[]={'A','B',0};CHECK(layout(*font,ab,3,0,0,0,0,0,{0,0,0,0},l));CHECK(l.count==0);CHECK(!layout(*font,ab,3,0,0,0,0,3,{0,0,640,480},l));CHECK(!layout(*font,nullptr,3,0,0,0,0,0,{0,0,640,480},l));CHECK(!layout(*font,ab,3,0,0,0,0,0,{0,0,641,480},l));CHECK(!layout(*font,ab,3,0,0,0,0,0,{0,10,640,9},l));std::array<uint8_t,97>text{};text.fill('A');text[95]=0;CHECK(layout(*font,text.data(),96,0,0,0,0,0,{0,0,640,480},l));text[95]='A';text[96]=0;CHECK(!layout(*font,text.data(),97,0,0,0,0,0,{0,0,640,480},l));
}
static void compactSurfaceTests(){
 Fixture f;const auto id=identity(343);CHECK(id.valid()&&id.pixelCount()==343u*480);
 CHECK(identity().pixelCount()==FramePacket::pixelCount&&FramePacket::maxPixelCount==640u*480);
 auto data=std::vector<uint16_t>(id.pixelCount(),0x2468);const auto pristine=data;
 auto c=command();c.x=340;c.clip={0,0,343,480};
 FramePacketBuilder builder(f.font());CHECK(builder.begin(id,1,data.data(),data.size()));CHECK(builder.beginString(c));
 const size_t last=343u*480-1;CHECK(builder.word(last,data[last],0x1234,0x1234));data[last]=0x1234;CHECK(builder.endString(c));
 auto packet=builder.finish(id,data.data(),data.size());CHECK(packet&&packet->pixels()==343u*480);
 CHECK(packet->clean()==pristine&&packet->original()==data&&packet->vertices().size()==12);
 for(const auto&v:packet->vertices())CHECK(v.x>=0&&v.x<=343&&v.y>=0&&v.y<=480);
 CHECK(packet->matches(id,1,data.data(),data.size()));CHECK(!packet->matches(id,1,data.data(),FramePacket::pixelCount));
 CHECK(!packet->matches(identity(),1,data.data(),data.size()));CHECK(!packet->matches(id,2,data.data(),data.size()));
 for(unsigned slot:{0u,3u,4u,5u,6u,8u,11u,12u,13u,14u,15u,16u,17u,18u,19u}){auto changed=id;++changed.geometry[slot];CHECK(!packet->matches(changed,1,data.data(),data.size()));}
 auto retired=id;++retired.lifetime;CHECK(!packet->matches(retired,1,data.data(),data.size()));
 data[last]^=1;CHECK(!packet->matches(id,1,data.data(),data.size()));CHECK(packet->original()[last]==0x1234);
 CHECK(!builder.begin(id,1,data.data(),data.size()));CHECK(builder.begin(id,2,data.data(),data.size()));builder.invalidate();
 for(unsigned width:{0u,342u,344u,641u}){auto bad=identity(width);CHECK(!bad.valid()&&bad.pixelCount()==0);FramePacketBuilder b(f.font());CHECK(!b.begin(bad,1,data.data(),data.size()));}
 for(unsigned slot:{2u,10u,14u,15u,16u,17u,18u,19u}){auto bad=id;++bad.geometry[slot];CHECK(!bad.valid());}
 auto bad=id;bad.geometry[10]=1280;CHECK(!bad.valid());bad=id;bad.geometry[5]=344;CHECK(!bad.valid());bad=id;bad.geometry[8]=0xfffffff0;CHECK(!bad.valid());
 FramePacketBuilder wrongCount(f.font());CHECK(!wrongCount.begin(id,1,data.data(),FramePacket::pixelCount));
 FramePacketBuilder overflow(f.font());CHECK(overflow.begin(id,1,data.data(),data.size()));CHECK(overflow.beginString(c));CHECK(!overflow.word(data.size(),0,0,0));CHECK(!overflow.finish(id,data.data(),data.size()));
 FramePacketBuilder finishCount(f.font());CHECK(finishCount.begin(id,1,data.data(),data.size()));CHECK(finishCount.beginString(c));CHECK(finishCount.word(last,data[last],data[last],data[last]));CHECK(finishCount.endString(c));CHECK(!finishCount.finish(id,data.data(),FramePacket::pixelCount));
 for(float right:{344.f,640.f}){FramePacketBuilder b(f.font());auto wide=c;wide.clip.right=right;CHECK(b.begin(id,1,data.data(),data.size()));CHECK(b.beginString(wide));CHECK(b.word(0,data[0],data[0],data[0]));CHECK(!b.endString(wide));CHECK(!b.finish(id,data.data(),data.size()));}
 // A smaller actual clip must also remain authoritative for both supported widths.
 for(unsigned width:{343u,640u}){auto clipped=identity(width);clipped.geometry[3]=5;clipped.geometry[4]=7;clipped.geometry[5]=width-1;clipped.geometry[6]=479;
  auto pixels=std::vector<uint16_t>(clipped.pixelCount());auto within=command();within.clip={5,7,float(width-1),479};
  FramePacketBuilder valid(f.font());CHECK(valid.begin(clipped,1,pixels.data(),pixels.size()));CHECK(valid.beginString(within));CHECK(valid.word(7*width+5,0,1,1));pixels[7*width+5]=1;CHECK(valid.endString(within));CHECK(valid.finish(clipped,pixels.data(),pixels.size()));
  for(unsigned edge=0;edge<4;++edge){auto outside=within;if(edge==0)outside.clip.left=4;if(edge==1)outside.clip.top=6;if(edge==2)outside.clip.right=float(width);if(edge==3)outside.clip.bottom=480;
   FramePacketBuilder rejected(f.font());CHECK(rejected.begin(clipped,1,pixels.data(),pixels.size()));CHECK(rejected.beginString(outside));CHECK(rejected.word(7*width+5,1,1,1));CHECK(!rejected.endString(outside));}
 }
}
int main(){fontTests();layoutTests();packetTests();capacityTests();compactSurfaceTests();std::printf("PASS software-frame-core checks=%u synthetic-only=1\n",checks);}
