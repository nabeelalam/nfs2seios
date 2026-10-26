// SPDX-License-Identifier: MIT
// Synthetic owned bytes only; not an original-source or live-owner acceptance.
#include "hd/LoadSaveSourcePolicy.h"
#include "hd/SoftwareFontBuiltin.h"
#include <functional>
#include <iostream>
#include <stdexcept>
namespace ls=nfs_loadsave_source;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x))throw std::runtime_error(#x);}while(0)
struct Fixture {
 ls::Profile profile{};ls::Evidence evidence{};ls::Authority current{};
 std::vector<uint16_t>post;std::shared_ptr<const nfs_soft::FramePacket>packet;
 Fixture(unsigned selected=1,unsigned disabled=1){
  profile.identity=71;profile.displayFont=nfs_soft::createBuiltinFontCatalog();profile.fontBytes.resize(192);
  for(unsigned i=0;i<96;++i){profile.glyphs[i]={i*2,2,2,1,3};profile.fontBytes[i*2]=0xf8;profile.fontBytes[i*2+1]=0x40;}
  for(unsigned i=0;i<8;++i)profile.roles[i]=i==7?0:1;
  for(unsigned st=0;st<4;++st)for(unsigned fg=0;fg<2;++fg){uint32_t rgb=fg?(st==3?0x08f0f0:st==2?0x484040:st==1?0xffffff:0x908080):8;for(unsigned nib=1;nib<16;++nib)profile.palettes[2*st+fg][nib]=(nib==15?0xff000000u:0x80000000u)|rgb;}
  auto&o=current.owner;o.formSymbol=ls::FormSymbol;o.handlerSymbol=ls::HandlerSymbol;o.type=9;o.runtimeForm=0x11000;o.archive=0x20000;o.targetOwner=0x30000;o.game=3;o.thread=4;o.epoch=5;o.lifetime=6;o.serial=7;o.context=8;o.depth=1;
  current.table=0x40000;current.selected=uint16_t(selected);
  for(unsigned i=0;i<8;++i){auto&a=current.rows[i];a.symbol=ls::descriptorSymbol(i);a.descriptor=0x50000+32*i;a.textGuest=0x60000+128*i;a.descriptorWord=ls::resource(i)<<16|(i==7?0:2);a.inputCategory=i==7?0:2;a.flags=i==0?disabled&1:i==3?(disabled>>1)&1:0;}
  evidence.entry=evidence.exit=current;
  auto&snap=evidence.post;snap.owner=o;snap.profileIdentity=profile.identity;snap.font=0x70000;snap.palette=0x80000;snap.callbackSymbol=0x4a5830;snap.format=16;snap.size=18;snap.fontBytes=profile.fontBytes;
  auto&surface=snap.surface;surface.lifetime=o.lifetime;auto&g=surface.geometry;g[1]=640;g[2]=480;g[5]=640;g[6]=480;g[7]=0x7810;g[8]=0x100000;g[10]=1280;g[11]=0x120000;g[12]=0x130000;g[15]=2;g[16]=1278;g[17]=16;g[18]=1296;g[19]=613136;
  std::vector<uint16_t>clean(ls::Pixels);for(size_t i=0;i<clean.size();++i)clean[i]=uint16_t((i*97+13)&65535);post=clean;
  nfs_soft::FramePacketBuilder builder(profile.displayFont);CHECK(builder.begin(surface,o.serial,clean.data(),clean.size()));
  for(unsigned k=0;k<16;++k){unsigned i=k/2;bool shadow=k%2==0;ls::String s;s.word=i;s.shadow=shadow;s.descriptor=current.rows[i].descriptor;s.textGuest=current.rows[i].textGuest;s.resourcePolicy=4;s.resolved=s.alignmentObserved=s.measured=true;s.before=s.after=snap;s.paletteWords=profile.palettes[2*unsigned(ls::style(current,i))+!shadow];
   nfs_soft::TextCommand c;c.length=uint32_t(std::strlen(ls::text(i)));std::memcpy(c.text.data(),ls::text(i),c.length);c.role=profile.roles[i];c.alignment=2;c.sourceSize=18;c.advance=int(c.length*3);s.measuredAdvance=c.advance;c.x=426-c.advance+int(shadow);c.y=ls::top(i)+int(shadow);c.color=shadow?1:ls::color(ls::style(current,i));CHECK(builder.beginString(c));
   int pen=c.x;for(unsigned n=0;n<c.length;++n){ls::GlyphUse use;use.code=c.text[n];use.x=pen;use.y=c.y;const auto&glyph=profile.glyphs[size_t(nfs_soft::index(use.code))];
    for(unsigned y=0;y<2;++y)for(unsigned x=0;x<2;++x){unsigned nib=x%2?profile.fontBytes[glyph.offset+y]&15:profile.fontBytes[glyph.offset+y]>>4;uint32_t p=s.paletteWords[nib];if(p<0x10000000u)continue;size_t at=size_t(c.y+y)*640+size_t(pen)+x;uint16_t before=post[at],after=ls::blend(before,p);use.stores.push_back({uint32_t(at),before,after,after});CHECK(builder.word(at,before,after,after));post[at]=after;}
    s.glyphs.push_back(use);pen+=3;
   }
   evidence.strings.push_back(s);CHECK(builder.endString(c));
  }
  packet=builder.finish(surface,post.data(),post.size());CHECK(packet);
 }
 bool valid()const{return ls::consistent(packet,evidence,profile,current,post.data(),post.size());}
};
int main(){try{
 CHECK(ls::resource(0)==0x179&&ls::resource(7)==0x178&&ls::resource(8)==0);
 CHECK(ls::top(0)==144&&ls::top(6)==317&&ls::top(7)==105);
 CHECK(ls::blend(0,0xffffffffu)==65535&&ls::blend(0xffff,0xff000008u)==1);
 for(unsigned selected:{0u,1u,2u,3u,4u,5u,6u})for(unsigned disabled=0;disabled<4;++disabled){Fixture f(selected,disabled);CHECK(f.valid());CHECK(!ls::supportedOriginalProfile(f.profile));}
 Fixture f;CHECK(f.valid());
 for(unsigned invalid:{7u,8u,65535u}){Fixture invalidFixture(invalid);CHECK(!invalidFixture.valid());CHECK(!ls::authority(invalidFixture.current));CHECK(ls::sameAuthority(invalidFixture.current,invalidFixture.evidence.entry)&&ls::sameAuthority(invalidFixture.evidence.entry,invalidFixture.evidence.exit));}

 auto fault=[&](const std::function<void()>&change){auto e=f.evidence;auto c=f.current;auto p=f.profile;auto pixels=f.post;change();CHECK(!f.valid());f.evidence=std::move(e);f.current=c;f.profile=std::move(p);f.post=std::move(pixels);CHECK(f.valid());};
 fault([&]{++f.current.owner.runtimeForm;});fault([&]{f.current.owner.formSymbol=0x4d3a88;});fault([&]{f.current.owner.handlerSymbol=0x40c160;});fault([&]{f.current.owner.type=2;});
 fault([&]{++f.current.owner.game;});fault([&]{++f.current.owner.thread;});fault([&]{++f.current.owner.archive;});fault([&]{++f.current.owner.targetOwner;});fault([&]{++f.current.owner.epoch;});fault([&]{++f.current.owner.context;});fault([&]{++f.current.owner.lifetime;});fault([&]{++f.current.owner.serial;});
 for(unsigned mode=0;mode<4;++mode)fault([&]{auto&o=f.current.owner;if(mode==0)o.depth=2;if(mode==1)o.nested=true;if(mode==2)o.retired=true;if(mode==3)o.unwound=true;});
 fault([&]{++f.evidence.exit.selected;});fault([&]{f.current.selected=7;});fault([&]{++f.current.rows[0].descriptorWord;});fault([&]{++f.current.rows[0].inputCategory;});fault([&]{f.current.rows[2].flags=1;});
 fault([&]{++f.evidence.post.font;});fault([&]{++f.evidence.post.palette;});fault([&]{f.evidence.post.flags=1;});fault([&]{f.evidence.post.format=32;});fault([&]{f.evidence.post.surface.geometry[5]=639;});fault([&]{f.profile.fontBytes[0]^=1;});fault([&]{f.profile.glyphs[0].pitch=2;});fault([&]{f.profile.glyphs[0].offset=0xffffffffu;});fault([&]{f.profile.palettes[4][15]^=1;});fault([&]{f.evidence.post.fontBytes[0]^=1;});fault([&]{++f.evidence.post.profileIdentity;});fault([&]{f.evidence.post.mode=1;});fault([&]{f.evidence.post.originX=1;});fault([&]{f.evidence.post.size=16;});fault([&]{f.evidence.post.callbackSymbol=0x4a5d00;});fault([&]{f.evidence.post.surface.geometry[10]=640;});
 fault([&]{f.evidence.strings[0].word=1;});fault([&]{std::swap(f.evidence.strings[0],f.evidence.strings[1]);});fault([&]{f.evidence.strings.pop_back();});fault([&]{f.evidence.strings.push_back(f.evidence.strings[0]);});fault([&]{++f.evidence.strings[0].descriptor;});fault([&]{++f.evidence.strings[0].textGuest;});fault([&]{f.evidence.strings[0].resourcePolicy=3;});fault([&]{f.evidence.strings[0].measured=false;});fault([&]{++f.evidence.strings[0].measuredAdvance;});fault([&]{f.evidence.strings[0].paletteWords[15]^=1;});fault([&]{f.evidence.strings[0].glyphs[0].code='X';});fault([&]{++f.evidence.strings[0].glyphs[0].x;});fault([&]{f.evidence.strings[0].glyphs[0].stores[0].computed^=1;});fault([&]{f.evidence.strings[0].glyphs[0].stores[0].before^=1;});fault([&]{++f.evidence.strings[0].glyphs[0].stores[0].index;});fault([&]{f.evidence.strings[0].glyphs[0].stores.pop_back();});fault([&]{std::swap(f.evidence.strings[0].glyphs[0].stores[0],f.evidence.strings[0].glyphs[0].stores[1]);});fault([&]{f.post[0]^=1;});
 CHECK(!ls::consistent(f.packet,f.evidence,f.profile,f.current,nullptr,f.post.size()));CHECK(!ls::consistent(f.packet,f.evidence,f.profile,f.current,f.post.data(),f.post.size()-1));
 std::cout<<"LoadSave source policy: "<<checks<<" checks PASS (synthetic; no live authority)\n";return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
