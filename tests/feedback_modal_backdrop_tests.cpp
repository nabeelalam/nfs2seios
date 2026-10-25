// SPDX-License-Identifier: MIT
// Synthetic bytes only: no original archive payloads, hashes or fixture pixels.
#include "../src/hd/FeedbackModalBackdropReplay.h"
#include <cstdlib>
#include <iostream>
#include <functional>
using namespace nfs_feedback_backdrop;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"FAIL line "<<__LINE__<<": " #x "\n";std::exit(1);}}while(0)
static void put32(std::vector<uint8_t>&b,size_t p,uint32_t v){for(unsigned k=0;k<4;++k)b[p+k]=uint8_t(v>>(k*8));}
static uint32_t read32(const std::vector<uint8_t>&b,size_t p){return uint32_t(b[p])|uint32_t(b[p+1])*256u|uint32_t(b[p+2])*65536u|uint32_t(b[p+3])*16777216u;}
static void setDispatchSlot(Row&row,uint32_t at){row.dispatchSlot=at;row.callerStackBefore=row.callerStackAfter=at>=0x38?at-0x38:0;}
static uint32_t paletteWord(uint32_t c){const unsigned r=(c>>16)&255,g=(c>>8)&255,b=c&255;return(c&0xff000000u)|((r/8)<<11)|((g/4)<<5)|(b/8);}
// Independent channel-wise oracle, including the original fixed-point truncation.
static uint16_t indexedOracle(uint16_t old,uint32_t rgba){unsigned a=rgba>>24;if(a<16)return old;uint32_t packed=paletteWord(rgba);if(a>=252)return uint16_t(packed);const unsigned inv=255-a;return uint16_t(packed+(((old>>11)*inv/256)<<11)+(((((old>>5)&63)*inv)/256)<<5)+((old&31)*inv/256));}
static uint16_t bodyOracle(uint16_t old){const unsigned r=(5+(old>>11)*76/32)/8,g=(5+((old>>5)&63)*76/64)/4,b=(5+(old&31)*76/32)/8;return uint16_t((r<<11)|(g<<5)|b);}
struct Fixture {
 std::array<SourceIdentity,2>id{};std::array<std::vector<uint8_t>,2>source{};std::shared_ptr<const SourceCatalog>catalog;SelectorContract selectors{};Record r{};
 static void emit(Row&row,Plane&plane,unsigned x,unsigned y,const std::vector<uint16_t>&expected,const std::vector<uint8_t>&write){
  row.before.resize(row.count);row.after=expected;for(unsigned n=0;n<row.count;++n){size_t at=(y*640)+x+n;row.before[n]=plane[at];if(write[n])row.stores.push_back(Store{row.destinationAddress+n*2,plane[at],expected[n],expected[n]});plane[at]=expected[n];}row.sourceAfter=row.source;
 }
 Fixture(){
  for(unsigned k=0;k<2;++k){auto&i=id[k];i.providerKey=100+k;i.tag=k?0x676f6c64:0x72616264;i.width=k?117:344;i.height=k?61:47;i.paletteOffset=(16+i.width*i.height+3)&~3u;i.span=i.paletteOffset+1040;i.paletteCount=256;i.hotX=k?-112:0;i.hotY=k?-103:0;i.archiveOffset=k?30096:9832;i.header={{0x7bu|(i.paletteOffset<<8),i.width|(i.height<<16),uint32_t(uint16_t(i.hotX))|(uint32_t(uint16_t(i.hotY))<<16),k?0x00670070u:0x006d00c1u}};i.paletteHeader={{42,0x10100,0x100,0x00f00000}};i.archiveSha.fill(0x73);auto&b=source[k];b.resize(i.span);for(unsigned n=0;n<4;++n){put32(b,n*4,i.header[n]);put32(b,i.paletteOffset+n*4,i.paletteHeader[n]);}for(unsigned n=0;n<i.width*i.height;++n)b[16+n]=uint8_t(n*17+k*13);for(unsigned n=0;n<256;++n){const uint32_t colors[4]={0x00101010,0x0f040506,0x7c123456,0xfeeecc44};put32(b,i.paletteOffset+16+n*4,colors[n%4]);}CHECK(nfs_texture_sha256(b.data(),b.size(),i.sha.data()));}
  catalog=SourceCatalog::create(id,source);CHECK(catalog);auto&b=r.binding;b.owner=0x10000;b.parent=0x20000;b.form=0x30000;b.archive=0x1000000;b.targetOwner=0x11000000;b.titleResource=0x90000;b.messageResource=0x91000;b.ownerEpoch=1;b.parentEpoch=2;b.modalEpoch=3;b.lifetime=4;b.resourceEpoch=5;for(unsigned k=0;k<2;++k){b.sources[k]=b.archive+id[k].archiveOffset;b.providerKeys[k]=id[k].providerKey;}
  auto&t=r.initial;t.owner=b.targetOwner;t.w={{0x5744,640,480,0,0,640,480,0x7810,0x20000000,0,1280,0x720000,0x730000,0}};t.ownerWords=t.w;t.header={{0x78,640|(480u<<16),0,0}};for(unsigned n=0;n<640;++n)t.x[n]=2*n;for(unsigned n=0;n<480;++n)t.y[n]=16+n*1280;b.surface.lifetime=b.lifetime;std::copy(t.w.begin(),t.w.end(),b.surface.geometry.begin());auto&g=b.surface.geometry;g[14]=0;g[15]=2;g[16]=1278;g[17]=16;g[18]=1296;g[19]=16+479*1280;
  r.complete=1;r.initialMainRestoreComplete=1;r.titleWidth=96;r.messageWidth=108;r.restoredMainAfterScratch=t;r.pretextTarget=t;r.before.resize(Pixels);for(unsigned n=0;n<Pixels;++n)r.before[n]=uint16_t((n*137u)^(n>>3));r.initialRestored=r.restored=r.before;Plane plane=r.before;
  r.steps={Step::initialMain,Step::allocateScratch,Step::selectScratch,Step::fillColor,Step::fillScratch,Step::restoreMainAfterScratch,Step::bodyBlit,Step::retireScratch,Step::bannerClip,Step::bannerResolve,Step::bannerDraw,Step::restoreClip,Step::emblemResolve,Step::emblemHotspot,Step::emblemDraw};
  // Intentionally synthetic selector function IDs, not claimed original evidence.
  selectors.outer=0xf000211c;selectors.convertSlot=0x760000;selectors.convert=0xf000a100;selectors.blendSlot=0x760004;selectors.blend=0xf000a104;selectors.opacitySlot=0x760008;selectors.opacity=180;selectors.lutAddress=0x760100;for(unsigned n=0;n<256;++n)selectors.lut[n]=uint8_t(n*180/256);
  auto&s=r.body;s.descriptor=0x2f000000;s.source=0x30000000;s.allocated=s.selected=s.fillComplete=s.retired=1;s.words={{0x5744,186,60,0,0,186,60,0x7810,s.source,0,372,0x700000,0x710000,0}};s.retireWords=s.words;s.header={{0x78,186|(60u<<16),0,0}};for(unsigned n=0;n<186;++n)s.x.push_back(n*2);for(unsigned n=0;n<60;++n)s.y.push_back(16+n*372);s.fillColor=0xff080808;s.fillWord=0x0841;s.fillCallback=0xf0001aa8;s.fillDestination=s.source+16;s.fillCount=186*60;s.opacity=180;s.postFill.resize(16+186*60*2);for(unsigned n=0;n<4;++n)put32(s.postFill,n*4,s.header[n]);for(unsigned n=0;n<186*60;++n){s.postFill[16+n*2]=0x41;s.postFill[17+n*2]=8;}s.afterBlit=s.postFill;
  uint32_t sequence=0;for(unsigned yy=0;yy<60;++yy){Row row;row.sequence=sequence++;row.callback=row.dispatchBefore=row.dispatchAfter=0xf000211c;setDispatchSlot(row,0x800000);row.format=0x78;row.sourceAddress=s.source+16+yy*372;row.destinationAddress=b.surface.geometry[8]+16+((210+yy)*640+227)*2;row.count=186;row.selectors.complete=1;row.selectors.before=row.selectors.after=selectors;row.source.assign(s.postFill.begin()+16+yy*372,s.postFill.begin()+16+(yy+1)*372);std::vector<uint16_t>expected(186);for(unsigned xx=0;xx<186;++xx)expected[xx]=bodyOracle(plane[(210+yy)*640+227+xx]);emit(row,plane,227,210+yy,expected,std::vector<uint8_t>(186,1));s.rows.push_back(std::move(row));}
  for(unsigned k=0;k<2;++k){auto&d=r.draws[k];d.role=Role(k);d.sequence=k;d.complete=d.namedResolves=1;d.archive=b.archive;d.sourceAddress=d.resolvedAddress=b.sources[k];std::memcpy(d.name.data(),k?"dlog":"dbar",4);d.hotspot=k!=0;d.inputX=k?63:98;d.inputY=k?68:179;d.x=k?175:98;d.y=k?171:179;d.before=t;if(!k){d.before.w[3]=247;d.before.ownerWords[3]=247;}d.after=d.before;d.sourceBefore=d.sourceAfter=source[k];unsigned left=k?175:247,top=unsigned(d.y),right=unsigned(d.x)+id[k].width,bottom=top+id[k].height;d.uv={{left-unsigned(d.x),0,right-unsigned(d.x),id[k].height}};
   for(unsigned yy=top;yy<bottom;++yy){Row row;row.sequence=sequence++;row.callback=row.dispatchBefore=row.dispatchAfter=0xf0001850;setDispatchSlot(row,0x800100+k*256);row.format=0x7b;row.count=right-left;size_t off=16+(yy-top)*id[k].width+left-unsigned(d.x);row.sourceAddress=d.sourceAddress+uint32_t(off);row.destinationAddress=b.surface.geometry[8]+16+(yy*640+left)*2;row.paletteAddress=0x740000;row.source.assign(source[k].begin()+off,source[k].begin()+off+row.count);for(unsigned n=0;n<256;++n)row.paletteBefore[n]=paletteWord(read32(source[k],id[k].paletteOffset+16+n*4));row.paletteAfter=row.paletteBefore;std::vector<uint16_t>expected(row.count);std::vector<uint8_t>write(row.count);for(unsigned xx=0;xx<row.count;++xx){uint32_t c=read32(source[k],id[k].paletteOffset+16+row.source[xx]*4);expected[xx]=indexedOracle(plane[yy*640+left+xx],c);write[xx]=uint8_t((c>>24)>=16);}emit(row,plane,left,yy,expected,write);d.rows.push_back(std::move(row));}
  }r.pretext=std::move(plane);
 }
};
static void reject(Fixture&f,const std::function<void(Record&)>&mutate){Record r=f.r;mutate(r);CHECK(!ReplayVerifier::verify(r,f.r.binding,f.catalog,f.selectors));}
int main(){
 Fixture f;CHECK(f.r.binding.valid());CHECK(f.selectors.valid());CHECK(detail::converted(0xff123456)==0xff0011aa);CHECK(detail::rgbaBlend(0xffff,detail::modulate(detail::expanded(0x0841),180))==bodyOracle(0xffff));
 // Last X entries lie beyond the height-sized prefix. Dispatch slots may
 // never alias them, including when every row reports matching fake IDs.
 reject(f,[](Record&r){for(auto&row:r.body.rows)setDispatchSlot(row,r.binding.surface.geometry[12]+639*4);});
 reject(f,[](Record&r){for(auto&row:r.body.rows)setDispatchSlot(row,r.body.words[12]+185*4);});
 auto accepted=ReplayVerifier::verify(f.r,f.r.binding,f.catalog,f.selectors);CHECK(accepted);CHECK(accepted->rows()==168);CHECK(accepted->stores()>11160);CHECK(accepted->before()==f.r.before);CHECK(accepted->pretext()==f.r.pretext);CHECK(accepted->matches(f.r.binding,f.r.pretext.data(),Pixels));CHECK(!accepted->matches(f.r.binding,nullptr,Pixels));CHECK(!accepted->matches(f.r.binding,f.r.pretext.data(),Pixels-1));
 // Admission identity/source/catalog corruption and allocation/target bounds.
 for(unsigned k=0;k<2;++k){auto data=f.source;data[k][20]^=1;CHECK(!SourceCatalog::create(f.id,data));auto identities=f.id;identities[k].header[0]^=1;CHECK(!SourceCatalog::create(identities,f.source));identities=f.id;identities[k].paletteHeader[2]=0;CHECK(!SourceCatalog::create(identities,f.source));identities=f.id;identities[k].providerKey=0;CHECK(!SourceCatalog::create(identities,f.source));identities=f.id;identities[k].span=UINT32_MAX;CHECK(!SourceCatalog::create(identities,f.source));data=f.source;data[k].pop_back();CHECK(!SourceCatalog::create(f.id,data));}
 {auto ids=f.id;ids[1].providerKey=ids[0].providerKey;CHECK(!SourceCatalog::create(ids,f.source));ids=f.id;ids[1].archiveOffset=ids[0].archiveOffset;CHECK(!SourceCatalog::create(ids,f.source));}
 {auto current=f.r.binding;++current.modalEpoch;CHECK(!ReplayVerifier::verify(f.r,current,f.catalog,f.selectors));CHECK(!accepted->matches(current,f.r.pretext.data(),Pixels));}
 reject(f,[](Record&r){r.initialMainRestoreComplete=0;});reject(f,[](Record&r){r.initialRestored[0]^=1;});reject(f,[](Record&r){r.complete=0;});reject(f,[](Record&r){r.error=1;});reject(f,[](Record&r){r.binding.owner=0;});reject(f,[](Record&r){r.binding.parentEpoch=0;});reject(f,[](Record&r){r.binding.resourceEpoch=0;});reject(f,[](Record&r){r.restored[0]^=1;});reject(f,[](Record&r){r.pretext.back()^=1;});reject(f,[](Record&r){r.before.pop_back();});reject(f,[](Record&r){std::swap(r.steps[4],r.steps[5]);});reject(f,[](Record&r){r.steps.pop_back();});reject(f,[](Record&r){r.restoredMainAfterScratch.x[20]^=1;});reject(f,[](Record&r){r.pretextTarget.w[3]=1;});
 reject(f,[](Record&r){r.body.retired=0;});reject(f,[](Record&r){r.body.allocated=2;});reject(f,[](Record&r){r.body.retireWords[8]^=2;});reject(f,[](Record&r){r.body.postFill[16]^=1;r.body.afterBlit=r.body.postFill;});reject(f,[](Record&r){r.body.afterBlit.back()^=1;});reject(f,[](Record&r){r.body.opacity=179;});reject(f,[](Record&r){r.body.fillCount-=1;});reject(f,[](Record&r){r.body.source=0xfffffff0;});reject(f,[](Record&r){r.body.descriptor=r.body.source;});reject(f,[](Record&r){r.body.words[11]=r.body.source;r.body.retireWords=r.body.words;});
 reject(f,[](Record&r){r.body.rows[0].selectors.complete=0;});reject(f,[](Record&r){r.body.rows[0].selectors.after.convert^=4;});reject(f,[](Record&r){r.body.rows[0].selectors.after.lut[15]^=1;});reject(f,[](Record&r){r.body.rows[0].sourceAfter[0]^=1;});reject(f,[](Record&r){r.body.rows[0].stores.pop_back();});reject(f,[](Record&r){std::swap(r.body.rows[0],r.body.rows[1]);});reject(f,[](Record&r){std::swap(r.body.rows[0].stores[0],r.body.rows[0].stores[1]);});reject(f,[](Record&r){r.body.rows[0].stores[0].computed^=1;});
 {auto missing=f.selectors;missing.convert=0;CHECK(!ReplayVerifier::verify(f.r,f.r.binding,f.catalog,missing));missing=f.selectors;missing.lut[255]^=1;CHECK(!ReplayVerifier::verify(f.r,f.r.binding,f.catalog,missing));}
 for(unsigned k=0;k<2;++k){reject(f,[k](Record&r){r.draws[k].sourceAfter[20]^=1;});reject(f,[k](Record&r){r.draws[k].rows[0].sourceAfter[0]^=1;});reject(f,[k](Record&r){r.draws[k].rows[0].paletteAfter[255]^=1;});reject(f,[k](Record&r){r.draws[k].rows[0].paletteBefore[255]^=1;r.draws[k].rows[0].paletteAfter=r.draws[k].rows[0].paletteBefore;});reject(f,[k](Record&r){r.draws[k].rows[0].sourceAddress+=1;});reject(f,[k](Record&r){r.draws[k].uv[0]+=1;});reject(f,[k](Record&r){r.draws[k].rows.pop_back();});reject(f,[k](Record&r){r.draws[k].rows[0].destinationAddress=UINT32_MAX;});reject(f,[k](Record&r){r.draws[k].rows[0].count=641;});reject(f,[k](Record&r){r.draws[k].rows[0].paletteAddress=r.binding.sources[k];});}
 reject(f,[](Record&r){r.draws[0].before.w[3]=246;r.draws[0].after=r.draws[0].before;});reject(f,[](Record&r){r.draws[1].inputX=64;});reject(f,[](Record&r){r.draws[1].hotspot=false;});reject(f,[](Record&r){r.draws[0].name[0]='x';});
 // A skipped transparent index must never acquire a store, even when old==after.
 {Record r=f.r;auto&row=r.draws[0].rows[0];unsigned n=0;while(n<row.count&&(row.paletteBefore[row.source[n]]>>24)>=16)++n;CHECK(n<row.count);row.stores.insert(row.stores.begin(),Store{row.destinationAddress+n*2,row.before[n],row.after[n],row.after[n]});CHECK(!ReplayVerifier::verify(r,f.r.binding,f.catalog,f.selectors));}
 reject(f,[](Record&r){auto&row=r.body.rows[0];row.stores.push_back(row.stores.back());});
 reject(f,[](Record&r){r.body.rows[0].before[0]^=1;});
 reject(f,[](Record&r){r.body.rows[0].after[0]^=1;});
 reject(f,[](Record&r){r.draws[1].rows[1].paletteAddress+=1024;});
 reject(f,[](Record&r){std::swap(r.draws[0].rows[0],r.draws[0].rows[1]);});
 {auto ids=f.id;ids[0].archiveSha.fill(0);CHECK(!SourceCatalog::create(ids,f.source));ids=f.id;ids[1].hotX=0;CHECK(!SourceCatalog::create(ids,f.source));ids=f.id;ids[0].paletteCount=255;CHECK(!SourceCatalog::create(ids,f.source));}
 // A forged expected digest cannot excuse an incompatible indexed source header.
 {auto ids=f.id;auto data=f.source;data[0][0]=0x78;CHECK(nfs_texture_sha256(data[0].data(),data[0].size(),ids[0].sha.data()));CHECK(!SourceCatalog::create(ids,data));}
 // Relocated BSS fields use byte arrays. Safe owned copies can carry genuinely
 // unaligned scalar/LUT addresses; this does not relax exact expected identity.
 {Record r=f.r;auto unaligned=f.selectors;unaligned.convertSlot=0x760003;unaligned.blendSlot=0x760007;unaligned.opacitySlot=0x760011;unaligned.lutAddress=0x760103;
  CHECK(unaligned.valid());for(auto&row:r.body.rows)row.selectors.before=row.selectors.after=unaligned;
  auto unalignedAccepted=ReplayVerifier::verify(r,r.binding,f.catalog,unaligned);CHECK(unalignedAccepted);CHECK(unalignedAccepted->pretext()==f.r.pretext);
  CHECK(!ReplayVerifier::verify(r,r.binding,f.catalog,f.selectors));
 }
 // Expected raw relocated ranges are validated even when every copied row agrees.
 {auto rejectSelectors=[&](SelectorContract bad){Record r=f.r;for(auto&row:r.body.rows)row.selectors.before=row.selectors.after=bad;CHECK(!ReplayVerifier::verify(r,r.binding,f.catalog,bad));};
  auto bad=f.selectors;bad.convertSlot=0;rejectSelectors(bad);
  bad=f.selectors;bad.convertSlot=0xfffffffe;rejectSelectors(bad);
  bad=f.selectors;bad.blendSlot=bad.convertSlot;rejectSelectors(bad);
  bad=f.selectors;bad.opacitySlot=bad.lutAddress+4;rejectSelectors(bad);
  bad=f.selectors;bad.lutAddress=0xfffffffc;rejectSelectors(bad);
  bad=f.selectors;bad.outer=0x4a1090;rejectSelectors(bad);
  bad=f.selectors;bad.convertSlot=f.r.body.source;rejectSelectors(bad);
  bad=f.selectors;bad.blendSlot=f.r.binding.surface.geometry[8];rejectSelectors(bad);
  bad=f.selectors;bad.opacitySlot=f.r.body.descriptor;rejectSelectors(bad);
  bad=f.selectors;bad.lutAddress=f.r.binding.sources[0];rejectSelectors(bad);
 }
 reject(f,[](Record&r){r.body.rows[0].callerStackAfter+=4;});
 reject(f,[](Record&r){r.body.rows[0].callerStackBefore=0;});
 reject(f,[](Record&r){r.body.rows[0].callerStackBefore=r.body.rows[0].callerStackAfter=0xfffffff0;});
 reject(f,[](Record&r){r.body.rows[0].callerStackBefore=r.body.rows[0].callerStackAfter=0x800000;});
 // The dispatch witness is caller esp+0x38, never a fabricated callback global.
 reject(f,[](Record&r){r.body.rows[0].dispatchAfter^=4;});
 reject(f,[](Record&r){r.body.rows[0].dispatchBefore^=4;});
 reject(f,[](Record&r){r.body.rows[0].dispatchSlot=0;});
 reject(f,[](Record&r){r.body.rows[0].dispatchSlot+=1;});
 reject(f,[](Record&r){setDispatchSlot(r.body.rows[1],r.body.rows[1].dispatchSlot+4);});
 reject(f,[](Record&r){for(auto&row:r.body.rows)setDispatchSlot(row,r.body.source);});
 reject(f,[](Record&r){for(auto&row:r.body.rows)setDispatchSlot(row,r.body.descriptor);});
 reject(f,[](Record&r){for(auto&row:r.body.rows)setDispatchSlot(row,r.binding.surface.geometry[8]);});
 reject(f,[](Record&r){for(auto&row:r.body.rows)setDispatchSlot(row,0x760004);});
 reject(f,[](Record&r){for(auto&row:r.body.rows)setDispatchSlot(row,r.binding.targetOwner);});
 reject(f,[](Record&r){r.draws[0].rows[0].dispatchAfter=0;});
 reject(f,[](Record&r){for(auto&row:r.draws[1].rows)setDispatchSlot(row,r.binding.sources[1]);});
 reject(f,[](Record&r){for(auto&row:r.draws[0].rows)setDispatchSlot(row,row.paletteAddress);});
 // Self-consistent changed binding still cannot alias or wrap the source target.
 {Record r=f.r;r.binding.sources[0]=r.binding.surface.geometry[8];CHECK(!ReplayVerifier::verify(r,r.binding,f.catalog,f.selectors));r=f.r;r.binding.archive=0xfffffff0;CHECK(!ReplayVerifier::verify(r,r.binding,f.catalog,f.selectors));}
 // Inputs and retained results survive failed/restarted observations unchanged.
 CHECK(f.r.pretext==accepted->pretext());auto retained=accepted;accepted.reset();f.source[0].clear();f.source[1].clear();CHECK(retained->catalog()->bytes(0)->size()==17224);CHECK(retained->matches(f.r.binding,f.r.pretext.data(),Pixels));CHECK(!f.catalog->identity(2));CHECK(!f.catalog->bytes(2));
 std::cout<<"PASS "<<checks<<" synthetic Feedback backdrop checks; no actual owner/native authority claimed\n";
}
