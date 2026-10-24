// SPDX-License-Identifier: MIT
// Synthetic indexed source and target only; no original data/native authority.
#include "hd/SteeringChartPacket.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
using nfs_steering_chart::Binding;using nfs_steering_chart::Builder;using nfs_steering_chart::Packet;
// Independent expanded-register oracle for original indexed483288. This does
// not call either production blend helper or the builder's palette converter.
static uint16_t reference(uint32_t ebx,uint16_t before){
 if(ebx<0x10000000u)return before;if(ebx>=0xfc000000u)return uint16_t(ebx);
 uint32_t eax=before,edx=eax,ecx=ebx;ecx^=0xffffffffu;eax<<=16;eax=(eax&0xffffff00u)|(edx&255u);edx&=0x7e0;edx<<=8;eax&=0xf800001fu;ecx>>=24;eax+=edx;
 const uint64_t product=uint64_t(eax)*ecx;eax=uint32_t(product);edx=uint32_t(product>>32);edx<<=8;ecx=eax;eax>>=16;edx&=0xf800;ecx>>=8;eax&=0x7e0;ebx+=edx;ecx&=31;ebx+=eax;ebx+=ecx;return uint16_t(ebx);
}
struct Fixture {
 Binding b{};std::shared_ptr<const nfs_soft::FramePacket>text;std::vector<uint8_t>source=std::vector<uint8_t>(18500);std::vector<uint16_t>plane;Builder::Palette palette{};uint32_t address=0x900000;
 Fixture(unsigned level=0){
  b.lifetime=7;b.ownerEpoch=9;b.textSerial=3;b.form=0x110;b.handler=0x220;b.archive=0x330;b.sensitivity=level;b.targetBitmap=0x10000;
  nfs_soft::FontDescription d;for(unsigned r=0;r<2;++r){d.baseline[r]=2;for(auto&g:d.glyphs[r])g={2,2,8,8,0,-8,3*65536};d.glyphs[r][0]={0,0,0,0,0,0,2*65536};}
  std::vector<uint8_t>alpha(1024*512);for(unsigned y=2;y<10;++y)for(unsigned x=2;x<10;++x)alpha[y*1024+x]=128;
  auto font=nfs_soft::FontCatalog::create(d,alpha.data(),alpha.size(),17);CHECK(font);
  nfs_soft::SurfaceIdentity s;s.lifetime=b.lifetime;auto&g=s.geometry;g[1]=640;g[2]=480;g[5]=640;g[6]=480;g[7]=16;g[8]=b.targetBitmap;g[10]=1280;g[11]=0x20000;g[12]=0x30000;g[15]=2;g[16]=1278;g[17]=16;g[18]=1296;g[19]=16+479*1280;
  plane.resize(307200);for(size_t i=0;i<plane.size();++i)plane[i]=uint16_t(i*719u+level);
  nfs_soft::FramePacketBuilder tb(font);CHECK(tb.begin(s,b.textSerial,plane.data(),plane.size()));nfs_soft::TextCommand c;c.text[0]='A';c.length=1;c.sourceSize=18;c.x=2;c.y=2;c.advance=3;c.color=65535;CHECK(tb.beginString(c));CHECK(tb.word(0,plane[0],plane[0],plane[0]));CHECK(tb.endString(c));text=tb.finish(s,plane.data(),plane.size());CHECK(text);
  const uint8_t header[]={0x7b,0x34,0x44,0,0x64,1,0x31,0,0xec,0xfe,0xbc,0xff,0x14,1,0x44,0};const uint8_t attachment[]={0x2a,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0};std::memcpy(source.data(),header,16);std::memcpy(source.data()+17460,attachment,16);
  const unsigned alphas[]={0,15,16,128,251,252,255};for(unsigned i=0;i<256;++i){const unsigned a=alphas[i%7],r=(i*11)&255,green=(i*13)&255,blue=(i*17)&255;const uint32_t raw=(a<<24)|(r<<16)|(green<<8)|blue;for(unsigned j=0;j<4;++j)source[17476+4*i+j]=uint8_t(raw>>(8*j));palette[i]=(a<<24)|((r>>3)<<11)|((green>>2)<<5)|(blue>>3);}
  for(unsigned i=0;i<356*49;++i)source[16+i]=uint8_t(i);
 }
 uint32_t destination(unsigned row)const{return b.targetBitmap+16+(280+row)*1280+560;}
 Builder::Row before(unsigned row)const{Builder::Row a{};std::copy_n(plane.data()+(280+row)*640+280,356,a.data());return a;}
 // All stores are driven by the independently encoded palette and register
 // oracle, including nontransparent stores that leave a value unchanged.
 void rows(Builder&builder,unsigned first=0,unsigned last=49){for(unsigned row=first;row<last;++row){auto pre=before(row),post=pre;CHECK(builder.beginRow(address+16+row*356,destination(row),356,palette,pre));for(unsigned x=0;x<356;++x){const uint32_t color=palette[source[16+row*356+x]];post[x]=reference(color,pre[x]);if(color>=0x10000000u)CHECK(builder.word(destination(row)+2*x,pre[x],post[x],post[x]));}CHECK(builder.endRow(post,palette));std::copy(post.begin(),post.end(),plane.begin()+(280+row)*640+280);}}
 std::shared_ptr<const Packet> complete(Builder&builder){CHECK(builder.begin(b,text,address,source));rows(builder);return builder.finish(b,source,plane.data(),plane.size());}
};
static void success(){
 for(unsigned level=0;level<4;++level){Fixture f(level);Builder builder;auto p=f.complete(builder);CHECK(p&&p->rows()==49&&p->stores()>0&&p->stores()<Builder::maxStores);CHECK(p->binding()==f.b&&p->text()==f.text&&p->source()==f.source&&p->original()==f.plane);CHECK(p->matches(f.b,f.plane.data(),f.plane.size()));CHECK(!p->matches(f.b,nullptr,f.plane.size()));CHECK(!p->matches(f.b,f.plane.data(),f.plane.size()-1));auto changed=f.plane;changed.back()^=1;CHECK(!p->matches(f.b,changed.data(),changed.size()));auto retired=f.b;++retired.ownerEpoch;CHECK(!p->matches(retired,f.plane.data(),f.plane.size()));
  const auto original=p->original();const auto bytes=p->source();builder.reset();CHECK(!builder.active());CHECK(p->original()==original&&p->source()==bytes);CHECK(builder.begin(f.b,f.text,f.address,f.source));builder.reject();CHECK(!builder.finish(f.b,f.source,f.plane.data(),f.plane.size()));CHECK(p->original()==original&&p->matches(f.b,f.plane.data(),f.plane.size()));
 }
}
static void extremeRows(){
 for(unsigned index:{0u,5u}){Fixture f;std::fill(f.source.begin()+16,f.source.begin()+17460,uint8_t(index));Builder b;auto p=f.complete(b);CHECK(p&&p->stores()==(index?Builder::maxStores:0));CHECK(p->sourceAddress()==f.address);}
 // Opaque black still generates mandatory stores; alpha is preserved without
 // premultiplication. Transparent black above generates none.
 Fixture f;std::fill(f.source.begin()+16,f.source.begin()+17460,uint8_t(0));f.source[17479]=255;f.palette[0]=0xff000000u;Builder b;CHECK(b.begin(f.b,f.text,f.address,f.source));f.rows(b);auto p=b.finish(f.b,f.source,f.plane.data(),f.plane.size());CHECK(p&&p->stores()==Builder::maxStores);
 // A real equal-value nontransparent write cannot be silently omitted.
 Fixture equal;const auto pre=equal.before(0);const uint16_t color=pre[0];const uint32_t raw=0xff000000u|((uint32_t(color)&0xf800u)<<8)|((uint32_t(color)&0x7e0u)<<5)|((uint32_t(color)&31u)<<3);for(unsigned j=0;j<4;++j)equal.source[17476+j]=uint8_t(raw>>(8*j));equal.palette[0]=0xff000000u|color;std::fill(equal.source.begin()+16,equal.source.begin()+17460,uint8_t(0));CHECK(reference(equal.palette[0],pre[0])==pre[0]);
 Builder omitted;CHECK(omitted.begin(equal.b,equal.text,equal.address,equal.source));CHECK(omitted.beginRow(equal.address+16,equal.destination(0),356,equal.palette,pre));CHECK(!omitted.word(equal.destination(0)+2,pre[1],color,color));Builder accepted;auto same=equal.complete(accepted);CHECK(same&&same->stores()==Builder::maxStores);
}
static void invalidBegins(){
 Fixture f;{auto wrong=f.source;std::swap(wrong[17469],wrong[17470]);Builder rejected;CHECK(!rejected.begin(f.b,f.text,f.address,wrong));}for(unsigned field=0;field<9;++field){auto b=f.b;if(field==0)b.lifetime=0;if(field==1)b.ownerEpoch=0;if(field==2)b.textSerial=0;if(field==3)b.form=0;if(field==4)b.handler=0;if(field==5)b.archive=0;if(field==6)b.sensitivity=4;if(field==7)b.targetBitmap=0;if(field==8)b.targetBitmap=0xfffffff0;CHECK(!b.valid());Builder x;CHECK(!x.begin(b,f.text,f.address,f.source));}
 for(unsigned field=0;field<3;++field){auto b=f.b;if(field==0)++b.lifetime;if(field==1)++b.textSerial;if(field==2)b.targetBitmap+=2;Builder x;CHECK(!x.begin(b,f.text,f.address,f.source));}
 for(unsigned offset:{0u,15u,17460u,17475u}){auto source=f.source;source[offset]^=1;Builder x;CHECK(!x.begin(f.b,f.text,f.address,source));}for(unsigned count:{0u,18499u,18501u}){auto source=f.source;source.resize(count);Builder x;CHECK(!x.begin(f.b,f.text,f.address,source));}
 for(uint32_t address:{0u,0xfffffff0u}){Builder x;CHECK(!x.begin(f.b,f.text,address,f.source));}Builder missing;CHECK(!missing.begin(f.b,{},f.address,f.source));
 Builder nested;CHECK(nested.begin(f.b,f.text,f.address,f.source));CHECK(!nested.begin(f.b,f.text,f.address,f.source));CHECK(nested.failure()==Builder::Failure::nesting&&!nested.active());
 // Maximum identities are opaque caller evidence; no builder counter increments
 // these values and no wrap can revive a rejected or incomplete collection.
 auto high=f.b;high.ownerEpoch=UINT64_MAX;Builder bounded;CHECK(bounded.begin(high,f.text,f.address,f.source));bounded.reject();CHECK(!bounded.endRow({},f.palette));bounded.reset();CHECK(bounded.begin(high,f.text,f.address,f.source));
}
static void rowFailures(){
 Fixture f;const auto before=f.before(0);for(unsigned mode=0;mode<7;++mode){Builder b;CHECK(b.begin(f.b,f.text,f.address,f.source));auto palette=f.palette;auto pre=before;uint32_t src=f.address+16,dst=f.destination(0);unsigned count=356;if(mode==0)++src;if(mode==1)dst+=2;if(mode==2)count=355;if(mode==3)palette[255]^=1;if(mode==4)pre[355]^=1;if(mode==5)src+=356;if(mode==6)dst=0xfffffff0;CHECK(!b.beginRow(src,dst,count,palette,pre));CHECK(!b.active());}
 Builder nested;CHECK(nested.begin(f.b,f.text,f.address,f.source));CHECK(nested.beginRow(f.address+16,f.destination(0),356,f.palette,before));CHECK(!nested.beginRow(f.address+16,f.destination(0),356,f.palette,before));
 Builder noRow;CHECK(noRow.begin(f.b,f.text,f.address,f.source));CHECK(!noRow.word(f.destination(0),before[0],before[0],before[0]));Builder noEnd;CHECK(noEnd.begin(f.b,f.text,f.address,f.source));CHECK(!noEnd.endRow(before,f.palette));
 // The first two indices have alpha0/15 and must NEVER produce stores.
 for(unsigned x:{0u,1u,3u,356u}){Builder b;CHECK(b.begin(f.b,f.text,f.address,f.source));CHECK(b.beginRow(f.address+16,f.destination(0),356,f.palette,before));CHECK(!b.word(f.destination(0)+2*x,before[x%356],0,0));}
 const unsigned first=2;const uint16_t value=reference(f.palette[2],before[2]);for(unsigned mode=0;mode<3;++mode){Builder b;CHECK(b.begin(f.b,f.text,f.address,f.source));CHECK(b.beginRow(f.address+16,f.destination(0),356,f.palette,before));CHECK(!b.word(f.destination(0)+2*first,uint16_t(before[2]^(mode==0)),uint16_t(value^(mode==1)),uint16_t(value^(mode==2))));}
 Builder missing;CHECK(missing.begin(f.b,f.text,f.address,f.source));CHECK(missing.beginRow(f.address+16,f.destination(0),356,f.palette,before));CHECK(!missing.endRow(before,f.palette)&&missing.failure()==Builder::Failure::missingStore);
 for(unsigned mode=0;mode<3;++mode){Builder b;CHECK(b.begin(f.b,f.text,f.address,f.source));CHECK(b.beginRow(f.address+16,f.destination(0),356,f.palette,before));auto after=before;for(unsigned x=0;x<356;++x){const auto color=f.palette[f.source[16+x]];after[x]=reference(color,before[x]);if(color>=0x10000000u)CHECK(b.word(f.destination(0)+2*x,before[x],after[x],after[x]));}if(mode==0){CHECK(!b.word(f.destination(0)+710,before[355],after[355],after[355]));}else{auto palette=f.palette;if(mode==1)palette[1]^=1;else after[0]^=1;CHECK(!b.endRow(after,palette));}}
}
static void finishFailures(){
 for(unsigned mode=0;mode<13;++mode){Fixture f;Builder b;CHECK(b.begin(f.b,f.text,f.address,f.source));if(mode!=0)f.rows(b);auto binding=f.b;auto source=f.source;auto plane=f.plane;const uint16_t*data=plane.data();size_t count=plane.size();if(mode==1)++binding.ownerEpoch;if(mode==2)++binding.lifetime;if(mode==3)++binding.textSerial;if(mode==4)++binding.form;if(mode==5)++binding.handler;if(mode==6)++binding.archive;if(mode==7)++binding.sensitivity;if(mode==8)binding.targetBitmap+=2;if(mode==9)source[17476]^=1;if(mode==10)plane[0]^=1;if(mode==11)data=nullptr;if(mode==12)--count;CHECK(!b.finish(binding,source,data,count));CHECK(!b.active());}
 Fixture f;Builder b;CHECK(b.begin(f.b,f.text,f.address,f.source));f.rows(b);CHECK(!b.beginRow(f.address+16+49*356,f.destination(49),356,f.palette,{}));CHECK(!b.finish(f.b,f.source,f.plane.data(),f.plane.size()));
 // Source indices drift is rejected even if the changed source would select an
 // equivalent transparent entry. Final comparison has no excluded region.
 Fixture g;Builder q;CHECK(q.begin(g.b,g.text,g.address,g.source));g.rows(q);g.source[16]=1;CHECK(!q.finish(g.b,g.source,g.plane.data(),g.plane.size()));
}
int main(){success();extremeRows();invalidBegins();rowFailures();finishFailures();std::printf("PASS steering chart packet checks=%u synthetic-only=1 owner-authority=none\n",checks);}
