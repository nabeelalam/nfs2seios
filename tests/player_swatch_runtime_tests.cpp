// SPDX-License-Identifier: MIT
// Real swatch lexical observers over checked synthetic guest memory. The owner
// and selected-car preview below are explicit stubs, not Player scene admission.
#define main existing_scene_runtime_fixture_main
#include "hd_scene_runtime_tests.cpp"
#undef main
#include "hd/PlayerSwatchPacket.h"
static constexpr uint32_t dword_4D2F80=0xe000,dword_4D2B68=0xe100,byte_512DBF=0xe200,byte_512DDA=0xe201,byte_512DC1=0xe220,byte_512DDC=0xe240,dword_4D62C8=0xe400,off_4DB648=0xc100,dword_564140=0xc200;
namespace nfs_player_scene_live {
namespace bg=nfs_background_scene_cpu;
struct SelectedCarStub{struct Selection{uint32_t selectedCar=4;}value;const Selection&selection()const noexcept{return value;}};
struct State {
 bg::Base base;std::shared_ptr<const SelectedCarStub>preview;std::shared_ptr<const nfs_logo_capture::Copy>currentCopy;
 std::shared_ptr<const void>published;std::shared_ptr<const nfs_player_swatch::Packet>swatch;nfs_player_swatch::Authority swatchAuthority;
 bg::Plane swatchCopyPost{},actualSwatchPost{};bg::Event currentCopyEvent{},swatchCopyEvent{},swatchEvent{},previewEvent{};
 uint32_t form=0,formGuest=0,sourceOwner=0,archive=0;uintptr_t game=0,thread=0;uint64_t iteration=0,ownerEpoch=0,sequence=0,nextSwatchSerial=0,acceptedSwatchCopySerial=0;
 bool on=false,invalid=false,baseVerified=false,swatchCurrent=false;
 void clearSwatch()noexcept{swatch.reset();swatchAuthority={};swatchCurrent=false;acceptedSwatchCopySerial=0;swatchCopyEvent={};swatchEvent={};}
 bg::Event event(uint64_t iteration)noexcept{return {ownerEpoch,++sequence,iteration};}
};
inline State*&slot()noexcept{static thread_local State*s=nullptr;return s;}static bool feature=true;
inline State*current(Game&g)noexcept{auto*s=slot();return feature&&s&&s->on&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==nfs_logo_publisher::threadToken()?s:nullptr;}
inline bool readPlane(bg::Plane&p)noexcept{uint32_t error=0;nfs_logo_publisher::plane(p,error);return !error&&p.valid;}
inline bool targetNow(bg::Target&t)noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);return true;}catch(...){return false;}}
inline bool archiveCurrent(State&s)noexcept{try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D40B8)==s.archive;}catch(...){return false;}}
}
#define NFS_PLAYER_SWATCH_RUNTIME_TEST 1
#include "hd/PlayerSwatchGameHooks.inc"
#undef NFS_PLAYER_SWATCH_RUNTIME_TEST
namespace sl=nfs_player_swatch_live;namespace ps=nfs_player_scene_live;namespace sw=nfs_player_swatch;
struct ColorOracle{std::array<uint32_t,4>table;std::array<uint32_t,3>rgb;uint32_t raw;uint16_t pixel;};
// Independent elementary cases: undimmed red; half-bright cyan (255*82/164);
// zero-saturation gray, brightened by (255-210)*2/47 = 1, hence 211.
static const ColorOracle colors[]={{{{0,255,255,82}},{{255,0,0}},0xffff0000u,0xf800},{{{128,255,255,164}},{{0,255,255}},0xff007f7fu,0x03ef},{{{0,0,210,80}},{{210,210,210}},0xffd3d3d3u,0xd69a}};
static const uint32_t fillTable[9]={0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0};
static void originalSwatch(Game&g,Fixture&f,ps::State&owner,unsigned player,const std::array<uint32_t,4>&clip,const ColorOracle&c,unsigned variant=0){
 prepareMemory(f);auto target=f.base.post.target;for(unsigned k=0;k<4;++k)target.w[3+k]=clip[k];target.ownerWords=target.w;select(target);std::memcpy(memory.data()+target.w[8]+16,f.base.post.pixels.data(),sw::Full*2);memory[byte_4DB25C]=16;m32(dword_571264,0);
 owner.iteration=0;owner.invalid=false;owner.baseVerified=true;owner.form=player==1?0x4d3050:0x4d3100;owner.formGuest=player==1?0xe800:0xe900;owner.sourceOwner=owner.formGuest;owner.archive=0x123456;owner.base.event=owner.event(0);owner.preview=std::make_shared<const ps::SelectedCarStub>();owner.previewEvent=owner.event(0);
 auto copy=std::make_shared<nfs_logo_capture::Copy>();copy->complete=1;copy->serial=100+owner.sequence;CHECK(ps::readPlane(copy->post));owner.currentCopy=copy;owner.currentCopyEvent=owner.event(0);
 const uint32_t carAt=player==1?byte_512DBF:byte_512DDA,colorAt=(player==1?byte_512DC1:byte_512DDC)+4;memory[carAt]=4;memory[colorAt]=7;for(unsigned k=0;k<4;++k)m32(dword_4D62C8+16*7+4*k,c.table[k]);for(unsigned k=0;k<9;++k)m32(off_4DB648+4*k,fillTable[k]);
 g.esp=0x9000;g.eax=player==1?dword_4D2F80:dword_4D2B68;m32(g.eax+0xc,player);g.edx=365;g.ebx=136;
 if(variant==29)m32(g.eax+12,player==1?2:1);
 try{
  sl::Scope frame(g,sl::Site::Refresh);
  CHECK(frame.entered&&(sl::current(g)||variant==29));if(variant==27){sl::RgbScope wrongOrder(g);}
  // Original410090 caller ESP=S. 458670 takes H/S/V output pointers in
  // EDX/EBX/ECX, plus D on stack, then consumes that one argument.
  const uint32_t s=0x9000;g.esp=s-4;m32(g.esp,s+0x2c);g.eax=7;g.edx=s+0x20;g.ebx=s+0x1c;g.ecx=s+0x18;
  if(variant==28)g.edx=target.w[8]+16;{sl::HsvScope hsv(g);m32(s+0x20,c.table[0]);m32(s+0x1c,c.table[1]);m32(s+0x18,c.table[2]);m32(s+0x2c,c.table[3]);if(variant==1)m32(s+0x20,c.table[0]+1);}
  g.esp=s-8;m32(s-8,s+0x24);m32(s-4,s+0x28);g.eax=c.table[0];g.edx=c.table[1];g.ebx=c.table[2];g.ecx=s+0x30;
  {sl::RgbScope rgb(g);m32(s+0x30,c.rgb[0]);m32(s+0x24,c.rgb[1]);m32(s+0x28,c.rgb[2]);if(variant==2)m32(s+0x24,c.rgb[1]+1);}
  g.esp=s;for(unsigned k=0;k<3;++k){m32(s+4*k,c.rgb[k]);m32(s+0xc+4*k,k==1?210:0);}g.eax=s+0xc;g.edx=c.table[3];g.ebx=s;g.ecx=s+0x34;
  {sl::ShadeScope shade(g);memory[s+0x34]=uint8_t(c.raw>>16);memory[s+0x35]=uint8_t(c.raw>>8);memory[s+0x36]=uint8_t(c.raw);if(variant==3)memory[s+0x35]^=1;}
  for(unsigned fillIndex=0;fillIndex<2;++fillIndex){const uint16_t value=fillIndex?c.pixel:0;const uint32_t raw=fillIndex?c.raw:0xff000000u;g.eax=raw;
   {sl::ColorScope convert(g);g.eax=value;if(variant==4&&fillIndex==1)g.eax^=1;}
   const unsigned x=fillIndex?366:367,y=fillIndex?141:142;g.eax=x;g.edx=y;g.ebx=84;g.ecx=21;m32(s,value);
   {sl::FillScope fill(g);m32(dword_564140,uint32_t(value)|(uint32_t(value)<<16));m32(s+0x38,sw::FillCallback);
    const unsigned left=std::max(x,clip[0]),top=std::max(y,clip[1]),right=std::min(x+84,clip[2]),bottom=std::min(y+21,clip[3]);
    if(left<right&&top<bottom)for(unsigned yy=top;yy<bottom;++yy){if(variant==5&&fillIndex==1&&yy+1==bottom)continue;g.eax=0xc500;g.edx=target.w[8]+16+2*(yy*640+left);g.ebx=right-left;g.ecx=0;
     if(variant==7&&fillIndex==1)m32(s+0x38,sw::FillCallback+4);if(variant==8&&fillIndex==1)m32(dword_571264,1);if(variant==9&&fillIndex==1)m32(dword_564140,1);
     {sl::RowScope row(g);for(unsigned xx=0;xx<right-left;++xx)m16(g.edx+2*xx,value);if(variant==6&&fillIndex==1&&yy+1==bottom)m16(g.edx,uint16_t(value^1));}
    }
   }
  }
  if(variant==10)m32(dword_4D62C8+16*7,c.table[0]^1);if(variant==11)memory[carAt]=5;if(variant==12)memory[colorAt]=8;if(variant==13)m32(dword_4D40B8,0x654321);
  if(variant==14)owner.currentCopy=std::make_shared<const nfs_logo_capture::Copy>(*copy);if(variant==15)++owner.currentCopyEvent.sequence;if(variant==16)++owner.base.event.sequence;if(variant==17)++owner.nextSwatchSerial;
  if(variant==18)m16(target.w[8]+16,0x2345);if(variant==19)m32(dword_4DB240+3*4,1);if(variant==20)owner.form=player==1?0x4d3100:0x4d3050;if(variant==21)++owner.formGuest;
  if(variant==22){auto p=std::make_shared<ps::SelectedCarStub>();p->value.selectedCar=5;owner.preview=p;}
  if(variant==31)owner.iteration=1;
  if(variant==30)m32((player==1?dword_4D2F80:dword_4D2B68)+12,player==1?2:1);
  if(variant==24)++owner.sourceOwner;if(variant==25)++owner.previewEvent.sequence;if(variant==26)throw 7;
  if(variant==23){g.eax=player==1?dword_4D2F80:dword_4D2B68;g.edx=365;g.ebx=136;sl::Scope nested(g,sl::Site::Refresh);}
 }catch(int){CHECK(variant==26);}
 g.esp=0x9000;
}
int main(){try{
 auto fixture=std::unique_ptr<Fixture>(new Fixture);Game g;auto owner=std::unique_ptr<ps::State>(new ps::State);ps::slot()=owner.get();owner->on=true;owner->game=reinterpret_cast<uintptr_t>(&g);owner->thread=pub::threadToken();owner->ownerEpoch=3;
 std::shared_ptr<const sw::Packet>retained;const std::array<uint32_t,4>full={{0,0,640,480}},partial={{433,144,640,480}},empty={{0,0,343,480}};
 for(unsigned player:{1u,2u})for(const auto&clip:{full,partial,empty})for(const auto&c:colors){originalSwatch(g,*fixture,*owner,player,clip,c);if(!owner->swatch)std::fprintf(stderr,"swatch failure=%u pure=%u player=%u\n",unsigned(sl::slot()->failure),unsigned(sl::slot()->verifyFailure),player);CHECK(owner->swatch&&owner->swatchCurrent);CHECK(owner->swatch->form()==owner->form&&owner->swatch->player()==player&&owner->swatch->rawColor()==c.raw);CHECK(owner->swatch->binding().owner==owner->formGuest);CHECK(owner->acceptedSwatchCopySerial==owner->currentCopy->serial);CHECK(sl::eventSame(owner->swatchCopyEvent,owner->currentCopyEvent));CHECK(owner->swatch->preMatches(owner->currentCopy->post));CHECK(owner->swatch->matches(owner->swatchAuthority,owner->actualSwatchPost.target,owner->actualSwatchPost.pixels.data(),sw::Full));
  // Independent original-operation oracle for every word, including untouched
  // source pixels. Neither implementation color() nor pack() generates it.
  auto expected=owner->currentCopy->post.pixels;for(unsigned k=0;k<2;++k){unsigned x=k?366:367,y=k?141:142;for(unsigned yy=std::max(y,clip[1]);yy<std::min(y+21,clip[3]);++yy)for(unsigned xx=std::max(x,clip[0]);xx<std::min(x+84,clip[2]);++xx)expected[yy*640+xx]=k?c.pixel:0;}CHECK(std::equal(expected.begin(),expected.end(),owner->swatch->post().begin()));
  const size_t coverage=std::count(owner->swatch->coverage().begin(),owner->swatch->coverage().end(),uint8_t(255));CHECK(coverage==(clip==full?1868u:clip==partial?342u:0u));retained=owner->swatch;
 }
 for(unsigned variant=1;variant<=31;++variant){originalSwatch(g,*fixture,*owner,1,full,colors[0],variant);CHECK(!owner->swatch&&!owner->swatchCurrent&&!owner->acceptedSwatchCopySerial);CHECK(sl::slot()->failure!=sl::Failure::None);}
 originalSwatch(g,*fixture,*owner,2,full,colors[0]);CHECK(owner->swatch);retained=owner->swatch;auto saved=retained->post();owner->actualSwatchPost.pixels.fill(0);owner->currentCopy.reset();owner->clearSwatch();CHECK(retained->post()==saved&&retained->pixels()[0]==0xf800);
 // All disabled/foreign/precondition paths remain inert before guest reads or
 // observer allocation. Start from a coherent active owner for the gate tests.
 originalSwatch(g,*fixture,*owner,1,full,colors[0]);g.eax=dword_4D2F80;g.edx=365;g.ebx=136;const auto before=reads,alloc=allocationCount.load();ps::feature=false;
 {sl::Scope off(g,sl::Site::Refresh);sl::HsvScope h(g);sl::RgbScope r(g);sl::ShadeScope s(g);sl::ColorScope c(g);sl::FillScope f(g);sl::RowScope row(g);}ps::feature=true;CHECK(reads==before&&allocationCount==alloc);
 g.eax=dword_4D2B68;{sl::Scope foreignRow(g,sl::Site::Refresh);}g.eax=dword_4D2F80;g.edx=0;{sl::Scope wrongArgs(g,sl::Site::Refresh);}g.edx=365;{sl::Scope wrongSite(g,static_cast<sl::Site>(99));}owner->iteration=1;{sl::Scope lazyOwner(g,sl::Site::Refresh);}owner->iteration=0;CHECK(reads==before&&allocationCount==alloc);
 std::thread foreign([&]{Game other;other.eax=dword_4D2F80;other.edx=365;other.ebx=136;sl::Scope unavailable(other,sl::Site::Refresh);sl::HsvScope h(other);sl::RowScope r(other);CHECK(!sl::slot());});foreign.join();CHECK(reads==before&&allocationInLease==0&&!leaseDepth);
 CHECK(retained&&retained->post()==saved);owner->on=false;ps::slot()=nullptr;std::puts("PASS Player swatch lexical runtime: both forms, independent RGB/shade/565 oracles, full/partial/no-write clips,31 rejection cases,current-copy binding,strong packets,foreign/disabled zero reads,no lease allocations; explicit owner stub only");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
