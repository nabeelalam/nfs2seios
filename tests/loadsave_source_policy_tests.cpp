// SPDX-License-Identifier: MIT
// Synthetic owned bytes only; not an original-source or live-owner acceptance.
#include "hd/SoftwareFontBuiltin.h"
#include <functional>
#include <iostream>
#include <stdexcept>
// Production config.h defines this macro before frontend policy headers.
#define packed __attribute__((__packed__))
#include "hd/LoadSaveSourcePolicy.h"
#undef packed
namespace ls=nfs_loadsave_source;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x))throw std::runtime_error(#x);}while(0)
struct Fixture {
 ls::Profile profile{};ls::Evidence evidence{};ls::Authority current{};
 std::vector<uint16_t>post;std::shared_ptr<const nfs_soft::FramePacket>packet;
 Fixture(unsigned selected=1,unsigned disabled=1){
  profile.identity=71;profile.displayFont=nfs_soft::createBuiltinFontCatalog();profile.atlasWidth=64;profile.atlasHeight=12;profile.atlasStride=32;profile.fontBytes.resize(16+32*12);
  auto put=[&](unsigned at,uint32_t v){for(unsigned k=0;k<4;++k)profile.fontBytes[at+k]=uint8_t(v>>(8*k));};put(0,0x7a);put(4,64|(12u<<16));
  for(unsigned i=0;i<96;++i){auto&g=profile.glyphs[i];g.code=uint16_t(32+i);g.sourceX=(i%16)*4;g.sourceY=(i/16)*2;g.width=3;g.height=2;g.advance=4;g.bearingX=-1;g.bearingY=-2;for(unsigned y=0;y<2;++y){unsigned at=16+(g.sourceY+y)*32+g.sourceX/2;profile.fontBytes[at]=0xf8;profile.fontBytes[at+1]=0x4f;}}
  profile.glyphs[0].width=1;profile.glyphs[0].height=1;profile.glyphs[0].bearingX=-3;profile.glyphs[0].bearingY=-10;
  profile.glyphRecords.resize(96*11);for(unsigned i=0;i<96;++i){auto&g=profile.glyphs[i];g.recordOffset=i*11;auto*r=profile.glyphRecords.data()+i*11;r[0]=uint8_t(g.code);r[1]=uint8_t(g.code>>8);r[2]=uint8_t(g.width);r[3]=uint8_t(g.height);r[4]=uint8_t(g.sourceX);r[5]=uint8_t(g.sourceX>>8);r[6]=uint8_t(g.sourceY);r[7]=uint8_t(g.sourceY>>8);r[8]=uint8_t(g.advance);r[9]=uint8_t(g.bearingX);r[10]=uint8_t(g.bearingY);}
  profile.paletteObserved.fill(true);
  for(unsigned i=0;i<8;++i)profile.roles[i]=i==7?0:1;
  for(unsigned st=0;st<4;++st)for(unsigned fg=0;fg<2;++fg){uint32_t rgb=fg?(st==3?0x08f0f0:st==2?0x484040:st==1?0xffffff:0x908080):8;for(unsigned nib=1;nib<16;++nib)profile.palettes[2*st+fg][nib]=(nib==15?0xff000000u:0x80000000u)|rgb;}
  auto&o=current.owner;o.formSymbol=ls::FormSymbol;o.handlerSymbol=ls::HandlerSymbol;o.type=9;o.runtimeForm=0x11000;o.archive=0x20000;o.targetOwner=0x30000;o.game=3;o.thread=4;o.epoch=5;o.lifetime=6;o.serial=7;o.context=8;o.depth=1;
  current.table=0x40000;current.selected=uint16_t(selected);
  for(unsigned i=0;i<8;++i){auto&a=current.rows[i];a.symbol=ls::descriptorSymbol(i);a.descriptor=0x50000+32*i;a.textGuest=0x60000+128*i;a.descriptorWord=ls::resource(i)<<16|(i==7?0:2);a.inputCategory=i==7?0:2;a.flags=i==0?disabled&1:i==3?(disabled>>1)&1:0;}
  evidence.entry=evidence.exit=current;
  auto&snap=evidence.post;snap.owner=o;snap.profileIdentity=profile.identity;snap.font=0x70000;snap.palette=0x80000;snap.callbackSymbol=0x4a5830;snap.format=16;snap.size=18;snap.fontBytes=profile.fontBytes;
  auto&surface=snap.surface;surface.lifetime=o.lifetime;auto&g=surface.geometry;g[0]=0x57494e44;g[1]=640;g[2]=480;g[5]=640;g[6]=480;g[7]=0x7810;g[8]=0x100000;g[10]=1280;g[11]=0x120000;g[12]=0x130000;g[15]=2;g[16]=1278;g[17]=16;g[18]=1296;g[19]=613136;
  std::vector<uint16_t>clean(ls::Pixels);for(size_t i=0;i<clean.size();++i)clean[i]=uint16_t((i*97+13)&65535);post=clean;
  auto target=[&](ls::Target&t,unsigned width,uint32_t owner,uint32_t bitmap){t.owner=owner;t.lifetime=o.lifetime;t.descriptor[0]=0x57494e44;t.descriptor[1]=t.descriptor[5]=width;t.descriptor[2]=t.descriptor[6]=480;t.descriptor[7]=0x7810;t.descriptor[8]=bitmap;t.descriptor[10]=width*2;t.descriptor[11]=owner+4;t.descriptor[12]=owner+8;t.header={{0x78,width|(480u<<16),0,0}};for(unsigned x=0;x<width;++x)t.x.push_back(x*2);for(unsigned y=0;y<480;++y)t.y.push_back(16+width*2*y);};
  auto&ts=evidence.targetsBefore;target(ts.full,640,o.targetOwner,0x100000);ts.full.descriptor[11]=g[11];ts.full.descriptor[12]=g[12];target(ts.slots[0],343,0x31000,0x200000);target(ts.slots[1],343,0x32000,0x300000);ts.selected=0;ts.selectedOwner=ts.slots[0].owner;evidence.targetsAfter=ts;
  evidence.fullBeforeCopy=clean;evidence.fullAfterCopy=clean;evidence.retainedCopy.resize(343*480);evidence.fullBeforeCopy[0]=0;evidence.retainedCopy[0]=clean[0];
  nfs_soft::FramePacketBuilder builder(profile.displayFont);CHECK(builder.begin(surface,o.serial,clean.data(),clean.size()));
  for(unsigned k=0;k<16;++k){unsigned i=k/2;bool shadow=k%2==0;ls::String s;s.word=i;s.shadow=shadow;s.descriptor=current.rows[i].descriptor;s.textGuest=current.rows[i].textGuest;s.resourcePolicy=4;s.resolved=s.alignmentObserved=s.measured=true;s.before=s.after=snap;s.paletteWords=profile.palettes[2*unsigned(ls::style(current,i))+!shadow];
   nfs_soft::TextCommand c;c.length=uint32_t(std::strlen(ls::text(i)));std::memcpy(c.text.data(),ls::text(i),c.length);c.role=profile.roles[i];c.alignment=2;c.sourceSize=18;c.advance=int(c.length*4);s.measuredAdvance=c.advance;c.x=426-c.advance+int(shadow);c.y=ls::top(i)+int(shadow);c.color=shadow?1:ls::color(ls::style(current,i));CHECK(builder.beginString(c));
   int pen=c.x;for(unsigned n=0;n<c.length;++n){uint8_t code=c.text[n];const auto&glyph=profile.glyphs[size_t(nfs_soft::index(code))];if(code==32){pen+=glyph.advance;continue;}ls::GlyphUse use;use.code=code;use.textIndex=n;use.x=pen+glyph.bearingX;use.y=c.y+glyph.bearingY;use.sourceX=glyph.sourceX;use.sourceY=glyph.sourceY;use.width=glyph.width;use.height=glyph.height;use.stride=profile.atlasStride;
    for(unsigned y=0;y<glyph.height;++y)for(unsigned x=0;x<(glyph.width+1)/2*2;++x){uint32_t offset=16+(glyph.sourceY+y)*profile.atlasStride+(glyph.sourceX+x)/2;uint8_t byte=profile.fontBytes[offset];unsigned parity=(glyph.sourceX+x)%2,nib=parity?byte&15:byte>>4;uint32_t palette=s.paletteWords[nib];if(palette<0x10000000u)continue;size_t at=size_t(use.y+y)*640+size_t(use.x+x);uint16_t before=post[at],after=ls::blend(before,palette);ls::Store store;store.index=uint32_t(at);store.before=before;store.computed=store.after=after;store.sourceOffset=offset;store.paletteValue=palette;store.sourceByte=byte;store.sourceParity=uint8_t(parity);use.stores.push_back(store);CHECK(builder.word(at,before,after,after));post[at]=after;}
    s.glyphs.push_back(use);pen+=glyph.advance;
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
 fault([&]{++f.evidence.post.font;});fault([&]{++f.evidence.post.palette;});fault([&]{f.evidence.post.flags=1;});fault([&]{f.evidence.post.format=32;});fault([&]{f.evidence.post.surface.geometry[5]=639;});fault([&]{f.profile.fontBytes[0]^=1;});fault([&]{f.profile.atlasStride=2;});fault([&]{f.profile.glyphs[0].sourceY=0xffffffffu;});fault([&]{f.profile.palettes[4][15]^=1;});fault([&]{f.evidence.post.fontBytes[0]^=1;});fault([&]{++f.evidence.post.profileIdentity;});fault([&]{f.evidence.post.mode=1;});fault([&]{f.evidence.post.originX=1;});fault([&]{f.evidence.post.size=16;});fault([&]{f.evidence.post.callbackSymbol=0x4a5d00;});fault([&]{f.evidence.post.surface.geometry[10]=640;});
 fault([&]{f.evidence.strings[0].word=1;});fault([&]{std::swap(f.evidence.strings[0],f.evidence.strings[1]);});fault([&]{f.evidence.strings.pop_back();});fault([&]{f.evidence.strings.push_back(f.evidence.strings[0]);});fault([&]{++f.evidence.strings[0].descriptor;});fault([&]{++f.evidence.strings[0].textGuest;});fault([&]{f.evidence.strings[0].resourcePolicy=3;});fault([&]{f.evidence.strings[0].measured=false;});fault([&]{++f.evidence.strings[0].measuredAdvance;});fault([&]{f.evidence.strings[0].paletteWords[15]^=1;});fault([&]{f.evidence.strings[0].glyphs[0].code='X';});fault([&]{++f.evidence.strings[0].glyphs[0].x;});fault([&]{f.evidence.strings[0].glyphs[0].stores[0].computed^=1;});fault([&]{f.evidence.strings[0].glyphs[0].stores[0].before^=1;});fault([&]{++f.evidence.strings[0].glyphs[0].stores[0].index;});fault([&]{f.evidence.strings[0].glyphs[0].stores.pop_back();});fault([&]{std::swap(f.evidence.strings[0].glyphs[0].stores[0],f.evidence.strings[0].glyphs[0].stores[1]);});fault([&]{f.post[0]^=1;});
 fault([&]{f.profile.glyphRecords[9]^=1;});fault([&]{f.profile.glyphs[size_t(nfs_soft::index('s'))].recordOffset=0;});fault([&]{f.profile.kerningCount=1;});fault([&]{f.profile.paletteObserved[4]=false;});fault([&]{++f.profile.glyphs[size_t(nfs_soft::index('s'))].bearingX;});fault([&]{++f.profile.glyphs[0].advance;});fault([&]{++f.profile.glyphs[size_t(nfs_soft::index('s'))].sourceX;});
 fault([&]{++f.evidence.strings[0].glyphs[0].sourceX;});fault([&]{++f.evidence.strings[0].glyphs[0].stride;});fault([&]{++f.evidence.strings[0].glyphs[0].textIndex;});fault([&]{++f.evidence.strings[0].glyphs[0].stores[0].sourceOffset;});fault([&]{f.evidence.strings[0].glyphs[0].stores[0].sourceParity^=1;});fault([&]{f.evidence.strings[0].glyphs[0].stores[0].sourceByte^=1;});fault([&]{f.evidence.strings[0].glyphs[0].stores[0].paletteValue^=1;});
 fault([&]{f.evidence.targetsAfter.slots[1].lifetime++;});fault([&]{f.evidence.targetsBefore.slots[1].y[10]++;});fault([&]{f.evidence.targetsAfter.selectedOwner++;});fault([&]{f.evidence.targetsAfter.slots[0].owner++;});fault([&]{f.evidence.targetsBefore.full.descriptor[8]++;});fault([&]{f.evidence.retainedCopy[0]^=1;});fault([&]{f.evidence.fullBeforeCopy[1]^=1;});
 for(unsigned slot:{0u,1u})fault([&]{f.evidence.targetsBefore.slots[slot].descriptor[7]=0x7808;f.evidence.targetsAfter.slots[slot].descriptor[7]=0x7808;});
 // Switching to the second protected slot is legitimate; neither identity changes.
 f.evidence.targetsAfter.selected=1;f.evidence.targetsAfter.selectedOwner=f.evidence.targetsAfter.slots[1].owner;CHECK(f.valid());f.evidence.targetsAfter=f.evidence.targetsBefore;
 CHECK(!ls::consistent(f.packet,f.evidence,f.profile,f.current,nullptr,f.post.size()));CHECK(!ls::consistent(f.packet,f.evidence,f.profile,f.current,f.post.data(),f.post.size()-1));
 std::cout<<"LoadSave source policy: "<<checks<<" checks PASS (synthetic; no live authority)\n";return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
