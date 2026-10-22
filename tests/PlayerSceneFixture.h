// SPDX-License-Identifier: MIT
#pragma once
#include "HdSceneFixture.h"
#include "hd/PlayerScenePrepare.h"
namespace player_scene_fixture {
namespace pf{namespace preview=nfs_player_preview;
static void put32(uint8_t*p,uint32_t v){for(unsigned k=0;k<4;++k)p[k]=uint8_t(v>>(k*8));}
static preview::Target target(){preview::Target t{};t.w[1]=640;t.w[2]=480;t.w[5]=640;t.w[6]=480;t.w[7]=0x7810;t.w[8]=0x100000;t.w[10]=1280;t.w[11]=0x30000;t.w[12]=0x31000;t.header[0]=0x78;t.header[1]=640|(480u<<16);for(unsigned k=0;k<640;++k)t.x[k]=2*k;for(unsigned k=0;k<480;++k)t.y[k]=16+1280*k;t.owner=0x41000;t.ownerWords=t.w;t.ownerState[3]=t.owner;return t;}
// Independent packed arithmetic reference, including both original alpha thresholds.
static uint16_t blend(uint16_t old,uint32_t color){if((color>>24)<16)return old;if((color>>24)>=252)return uint16_t(color);uint64_t spread=(old&31)|uint64_t(old&0x7e0)*256|uint64_t(old&0xf800)*65536;uint64_t faded=spread*(255-(color>>24));return uint16_t(color+((faded>>24)&0xf800)+((faded>>16)&0x7e0)+((faded>>8)&31));}
struct Fixture{
 preview::Input in;std::vector<uint16_t>oracle;std::vector<uint8_t>written;
 Fixture(bool graph=false,uint32_t form=preview::Player1Form,unsigned selected=0,unsigned cached=0){auto&t=in.pre.target;t=target();in.pre.valid=1;in.pre.width=640;in.pre.height=480;for(unsigned k=0;k<preview::Full;++k)in.pre.pixels[k]=uint16_t(k*13+7);in.pre.pixels[316*640+211]=0;in.post=in.pre;in.lifetime=3;in.serial=7;in.archive=0x20000;in.form=form;in.selectionBefore.selectedCar=selected;in.selectionBefore.cachedCarIndex=cached;in.selectionBefore.graphEnabled=graph?1:0;if(graph&&selected<=11)in.selectionBefore.graphValues=preview::statistics(cached);in.selectionAfter=in.selectionBefore;in.complete=1;in.fillCalls=1;oracle.assign(in.pre.pixels.begin(),in.pre.pixels.end());written.resize(preview::Full);
 auto&f=in.fill;f.before=f.after=t;f.call={{211,316,280,128}};f.rawColor=0xff000000;f.callback=preview::FillCallback;f.format=16;f.complete=1;f.tableBefore=f.tableAfter={{0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0}};
 for(unsigned y=316;y<444;++y){preview::Row row;row.callback=preview::FillCallback;row.format=16;row.args[1]=t.w[8]+16+(y*640+211)*2;row.args[2]=280;for(unsigned x=211;x<491;++x){row.before.push_back(oracle[y*640+x]);row.after.push_back(0);oracle[y*640+x]=0;written[y*640+x]=255;}f.rows.push_back(std::move(row));}
 if(graph&&selected<=11){add(32,16,211,316,3,0,"grph");const char*names[]={"grpg","grpb","grpr","grpy"};for(unsigned k=0;k<4;++k)add(12,8,237-9*(12-int(in.selectionBefore.graphValues[k])),327+30*k,3,0,names[k]);}
 else{auto name=preview::carName(cached);char n[5]={char(name[0]),char(name[1]),char(name[2]),char(name[3]),0};add(307,147,211,316,247,2,n);}
 in.drawCalls=unsigned(in.draws.size());std::copy(oracle.begin(),oracle.end(),in.post.pixels.begin());}
 void add(unsigned w,unsigned h,int x,int y,unsigned count,unsigned padding,const char*name){preview::Draw d;d.before=d.after=in.pre.target;d.args={{uint32_t(0x400000+in.draws.size()*0x100000),0x3000,uint32_t(x),uint32_t(y)}};std::copy_n(name,4,d.name.begin());unsigned offset=16+w*h+padding;d.sourceBefore.resize(offset+16+count*4);put32(d.sourceBefore.data(),(offset<<8)|0x7b);put32(d.sourceBefore.data()+4,w|(h<<16));put32(d.sourceBefore.data()+offset,0x2a);put32(d.sourceBefore.data()+offset+4,count);for(unsigned k=0;k<w*h;++k)d.sourceBefore[16+k]=uint8_t(k%7==0?255:k%count);
 for(unsigned k=0;k<count;++k){uint32_t raw=(k%3==0?0x08000000u:k%3==1?0xff000000u:0x80000000u)|(k*0x10203u&0xffffff);put32(d.sourceBefore.data()+offset+16+4*k,raw);d.paletteBefore[k]=(raw&0xff000000)|uint16_t(((raw>>8)&0xf800)|((raw>>5)&0x7e0)|((raw>>3)&31));}if(count<256)d.paletteBefore[255]=0;d.paletteAfter=d.paletteBefore;d.sourceAfter=d.sourceBefore;CHECK(nfs_texture_sha256(d.sourceBefore.data(),d.sourceBefore.size(),d.sha.data()));d.callback=preview::IndexedCallback;d.paletteGuest=0x900000;d.format=16;d.complete=1;d.tableBefore={{0xf000006c,0xf000183c,0xf0001840,0xf0001844,0xf0001848,0xf000184c,0,3,0}};d.tableAfter=d.tableBefore;d.tableAfter[6]=preview::IndexedCallback;d.tableAfter[8]=0x7b;
 for(int yy=std::max(0,y);yy<std::min(480,y+int(h));++yy){preview::Row row;row.callback=d.callback;row.format=16;row.paletteGuest=d.paletteGuest;row.paletteBefore=d.paletteBefore;row.paletteAfter=d.paletteAfter;unsigned sx=unsigned(std::max(0,x)-x),sy=unsigned(yy-y),n=unsigned(std::min(640,x+int(w))-std::max(0,x));row.args={{d.args[0]+16+sy*w+sx,in.pre.target.w[8]+16+uint32_t((yy*640+std::max(0,x))*2),n,0}};for(unsigned k=0;k<n;++k){unsigned idx=d.sourceBefore[16+sy*w+sx+k],at=yy*640+std::max(0,x)+k;row.source.push_back(uint8_t(idx));row.before.push_back(oracle[at]);uint32_t color=d.paletteBefore[idx];oracle[at]=blend(oracle[at],color);if(color>=0x10000000)written[at]=255;row.after.push_back(oracle[at]);}d.rows.push_back(std::move(row));}in.draws.push_back(std::move(d));}
};
}
namespace sf{namespace sw=nfs_player_swatch;
static sw::Target target(){sw::Target t{};t.w[1]=640;t.w[2]=480;t.w[5]=640;t.w[6]=480;t.w[7]=0x7810;t.w[8]=0x100000;t.w[10]=1280;t.w[11]=0x30000;t.w[12]=0x31000;t.header[0]=0x78;t.header[1]=640|(480u<<16);for(unsigned k=0;k<640;++k)t.x[k]=2*k;for(unsigned k=0;k<480;++k)t.y[k]=16+1280*k;t.owner=0x41000;t.ownerWords=t.w;t.ownerState[3]=t.owner;return t;}
struct Fixture{
 sw::Authority authority;sw::Input in;sw::Plane base,current;
 Fixture(unsigned player=1,sw::Origin origin=sw::Origin::Refresh){auto&b=authority.binding;b.epoch=11;b.serial=12;b.lifetime=13;b.owner=0x77000;b.archive=0x78000;b.player=player;b.origin=origin;b.form=player==1?0x4d3050:0x4d3100;b.row=player==1?sw::Player1Row:(origin==sw::Origin::Refresh?sw::Player2Row:sw::Player2BodyRow);b.car=4;b.color=7;b.carAddress=player==1?0x512dbf:0x512dda;b.colorAddress=(player==1?0x512dc1:0x512ddc)+b.car;b.tableAddress=sw::ColorTable+16*b.color;b.table={{0,255,255,82}};authority.events={20,21,22,23,24,25,26};
  in.binding=b;in.events=authority.events;in.complete=1;in.rgb={{255,0,0}};in.rawColor=0xffff0000u;base.valid=1;base.width=640;base.height=480;base.target=target();for(unsigned k=0;k<sw::Full;++k)base.pixels[k]=uint16_t(k*13+7);in.pre=base;in.post=base;
  // Independent oracle: this synthetic table selects undimmed red; first fill
  // black, second fill RGB565 red. Do not call implementation color/pack here.
  for(unsigned k=0;k<2;++k){auto&f=in.fills[k];unsigned x=k?366:367,y=k?141:142;uint16_t value=k?0xf800:0;f.before=f.after=base.target;f.call={{x,y,84,21}};f.rawColor=k?0xffff0000u:0xff000000u;f.pattern=value|(uint32_t(value)<<16);f.callback=sw::FillCallback;f.format=16;f.complete=1;f.tableBefore=f.tableAfter={{0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0}};
   for(unsigned yy=0;yy<21;++yy){sw::Row row;row.callback=sw::FillCallback;row.format=16;row.pattern=f.pattern;row.args[1]=base.target.w[8]+16+((y+yy)*640+x)*2;row.args[2]=84;for(unsigned xx=0;xx<84;++xx){auto&pixel=in.post.pixels[(y+yy)*640+x+xx];row.before.push_back(pixel);row.after.push_back(value);pixel=value;}f.rows.push_back(std::move(row));}
  }current=in.post;
 }
};
}

namespace ps=nfs_player_scene_cpu;namespace sw=nfs_player_swatch;
struct Fixture: ::Fixture {
 static constexpr unsigned CaptionAt=369*343+123,TransparentAt=5*343+20;
 nfs_soft::FontDescription description{};std::vector<uint8_t>alpha;std::shared_ptr<const nfs_soft::FontCatalog>font;std::shared_ptr<const nfs_soft::FramePacket>caption;
 std::shared_ptr<const ps::SidebarProof>sidebar;std::shared_ptr<const nfs_player_preview::Packet>preview;std::shared_ptr<const sw::Packet>swatch;std::shared_ptr<const ps::VerifiedSource>source;
 std::unique_ptr<pf::Fixture>p;std::unique_ptr<sf::Fixture>s;ps::SourceBinding binding;rec::Plane oldCopyPost,actualUpload;unsigned mode=0;ps::Failure failure=ps::Failure::None;
 Fixture(unsigned player=1,unsigned selectedMode=1):alpha(1024*512),mode(selectedMode){
  const uint32_t form=player==1?ps::Player1Form:ps::Player2Form;base.form=scene.form=form;scene.tailKind=ps::tail(form);scene.tailPredicate=scene.tailSymbol=form;scene.event={1,20,1};
  auto&side=scene.draws[0];side.sourceBefore[16+TransparentAt]=0;side.sourceAfter=side.sourceBefore;side.rows[0].payload[TransparentAt]=0;catalog.identities[0].sha=digest(side.sourceBefore);scene.postSide.pixels[TransparentAt]=scene.postWords.pixels[TransparentAt]=group.pre.pixels[TransparentAt]=group.post.pixels[TransparentAt]=scene.tailPre.pixels[TransparentAt]=scene.tailPost.pixels[TransparentAt]=0;
  alpha[1025]=255;for(auto&role:description.glyphs)for(auto&g:role){g.x=g.y=g.w=g.h=1;g.advance=65536;}font=nfs_soft::FontCatalog::create(description,alpha.data(),alpha.size(),77);CHECK(font);
  nfs_soft::TextCommand cmd{};cmd.length=10;std::memcpy(cmd.text.data(),player==1?"player one":"player two",10);cmd.role=1;cmd.alignment=2;cmd.x=player==1?100:98;cmd.y=player==1?198:234;cmd.sourceSize=18;cmd.advance=player==1?68:70;cmd.color=65535;cmd.clip={0,0,343,480};nfs_soft::SurfaceIdentity identity{};CHECK(ps::captionIdentity(copy.sourceOwner,7,identity));nfs_soft::FramePacketBuilder builder(font);CHECK(builder.begin(identity,5,scene.tailPre.pixels.data(),bg::Side));CHECK(builder.beginString(cmd));CHECK(builder.word(CaptionAt,scene.tailPre.pixels[CaptionAt],65535,65535));scene.tailPost.pixels[CaptionAt]=65535;CHECK(builder.endString(cmd));caption=builder.finish(identity,scene.tailPost.pixels.data(),bg::Side);CHECK(caption);
  // This old original side/group/caption proof precedes the NEW map refresh.
  syncCopy(base.post);
  p.reset(new pf::Fixture(false,form));p->in.lifetime=1;p->in.pre.target=base.post.target;p->in.post.target=base.post.target;p->in.fill.before=p->in.fill.after=base.post.target;for(auto&draw:p->in.draws)draw.before=draw.after=base.post.target;
  base.post=p->in.pre;base.event={1,50,0};for(unsigned k=0;k<bg::Full;++k)put16(base.sourceBefore.data()+16+2*k,base.post.pixels[k]);base.sourceAfter=base.sourceBefore;catalog.mapSha[0]=digest(base.sourceBefore);preview=nfs_player_preview::createPacket(p->in,base.post);CHECK(preview);
  // Finish the one immutable synthetic catalog BEFORE sealing old sidebar
  // evidence. Its actual recorded events remain20/21/22 and originalCopy
  // remains the old copy above; no new historical draw/event is fabricated.
  ps::SidebarInput si{scene,group,copy,caption,19,5,7,{1,21,1},{1,22,1}};sidebar=ps::verifySidebar(si,workspace,catalog,&failure);if(!sidebar)std::fprintf(stderr,"Player fixture sidebar seal failed player=%u mode=%u failure=%u\n",player,mode,unsigned(failure));CHECK(sidebar);
  syncCopy(p->in.post);oldCopyPost=copy.post;actualUpload=copy.post;
  binding.form=form;binding.formGuest=player==1?0x1000100:0x1000200;binding.sourceOwner=binding.formGuest;binding.archive=0x20000;binding.gameOwner=0x7000;binding.thread=0x8000;binding.epoch=binding.lifetime=1;binding.generation=3;binding.contentSerial=4;binding.copySerial=copy.serial;binding.kind=mode==0?ps::SourceKind::Initial:ps::SourceKind::Refresh;
  if(mode){s.reset(new sf::Fixture(player,sw::Origin::Refresh));auto&a=s->authority;a.binding.owner=binding.formGuest;a.binding.epoch=a.binding.lifetime=1;a.binding.archive=0x20000;a.binding.car=0;a.binding.colorAddress=(player==1?0x512dc1:0x512ddc);a.events={54,55,56,57,58,59,60};s->in.binding=a.binding;s->in.events=a.events;s->base=copy.post;s->in.pre=s->base;s->in.post=s->base;
   for(unsigned k=0;k<2;++k){auto&f=s->in.fills[k];f.before=f.after=s->base.target;unsigned x=k?366:367,y=k?141:142;uint16_t value=k?0xf800:0;for(unsigned yy=0;yy<21;++yy)for(unsigned xx=0;xx<84;++xx){auto&pixel=s->in.post.pixels[(y+yy)*640+x+xx];f.rows[yy].before[xx]=pixel;f.rows[yy].after[xx]=value;pixel=value;}}s->current=s->in.post;swatch=sw::createPacket(s->in,a,s->base,s->current);CHECK(swatch);actualUpload=s->current;
   if(mode==2){copy.serial++;binding.copySerial=copy.serial;binding.kind=ps::SourceKind::Lazy;syncCopy(s->current);actualUpload=copy.post;}
  }
  seal();CHECK(source);
 }
 void syncCopy(const rec::Plane&pre){copy.pre=pre;copy.post=pre;for(unsigned k=0;k<bg::Side;++k)put16(copy.sourceBefore.data()+16+2*k,scene.tailPost.pixels[k]);copy.sourceAfter=copy.sourceBefore;CHECK(nfs_texture_sha256(copy.sourceBefore.data(),copy.span,copy.shaBefore.data()));copy.shaAfter=copy.shaBefore;for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){uint16_t v=scene.tailPost.pixels[y*343+x];put32(copy.row[y].source.data()+4*x,v?rgba(v):0);copy.post.pixels[y*640+x]=v?v:pre.pixels[y*640+x];}}
 ps::JoinedInput input(){ps::JoinedInput i(base,copy);i.binding=binding;i.sidebar=sidebar;i.acceptedSidebarSerial=19;i.sidebarLifetime=7;i.actualSidebarPost=&scene.tailPost;i.sidebarValidationEvent={1,mode==2?72u:52u,mode==2?1u:0u};i.preview=preview;i.acceptedPreviewSerial=7;i.previewLifetime=1;i.previewArchive=0x20000;i.previewBaseEvent=base.event;i.previewEvent={1,51,0};i.actualPreviewPost=&p->in.post;i.copyEvent={1,mode==2?73u:53u,mode==2?1u:0u};i.uploadEvent={1,mode==2?90u:80u,mode==2?1u:0u};i.actualUploadPost=&actualUpload;if(mode){i.swatch=swatch;i.swatchAuthority=s->authority;i.acceptedSwatchCopySerial=31;i.swatchCopyEvent={1,53,0};i.swatchEvent={1,60,0};i.swatchCopyPost=&oldCopyPost;i.actualSwatchPost=&s->current;}return i;}
 void seal(){source=ps::verifySource(input(),workspace,catalog,&failure);if(!source)std::fprintf(stderr,"Player fixture source seal failed player=%u mode=%u failure=%u\n",binding.form==ps::Player1Form?1u:2u,mode,unsigned(failure));}
};
} // namespace player_scene_fixture
