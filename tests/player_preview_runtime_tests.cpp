// SPDX-License-Identifier: MIT
// Actual Player preview scopes with original-operation simulation and checked
// guest memory. Scene ownership is a minimal explicit stub, not live acceptance.
#define main existing_scene_runtime_fixture_main
#include "hd_scene_runtime_tests.cpp"
#undef main
#include "hd/PlayerPreviewPacket.h"
static constexpr uint32_t off_4D3050=0xe000,off_4D3100=0xe100,byte_512DBF=0xe200,byte_512DDA=0xe201,byte_512DD1=0xe202,dword_4E5924=0xe210,dword_4E5928=0xe214,dword_4D236C=0xe300,dword_4D2330=0xe340,dword_4D23E4=0xe380,dword_4D23A8=0xe3c0,off_4DB648=0xc100,dword_564140=0xc200;
namespace nfs_player_scene_live {
namespace bg=nfs_background_scene_cpu;
struct State{bg::Base base;std::shared_ptr<const nfs_player_preview::Packet>preview;std::shared_ptr<const void>published;bg::Plane actualPreviewPost{};nfs_player_preview::Selection previewSelection{};uint32_t form=0,formGuest=0,archive=0;uintptr_t game=0,thread=0;uint64_t ownerEpoch=0,sequence=0,nextPreviewSerial=0,previewSerial=0,previewLifetime=0;bg::Event previewEvent{},previewBaseEvent{};bool on=false,invalid=false,baseVerified=false;unsigned swatchClears=0;void clearSwatch()noexcept{++swatchClears;}bg::Event event(uint64_t iteration)noexcept{return {ownerEpoch,++sequence,iteration};}};
inline State*&slot()noexcept{static thread_local State*s=nullptr;return s;}static bool feature=true;
inline State*current(Game&g)noexcept{auto*s=slot();return feature&&s&&s->on&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==nfs_logo_publisher::threadToken()?s:nullptr;}
inline bool readPlane(bg::Plane&p)noexcept{uint32_t error=0;nfs_logo_publisher::plane(p,error);return !error&&p.valid;}
inline bool targetNow(bg::Target&t)noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);return true;}catch(...){return false;}}
inline bool archiveCurrent(State&s)noexcept{try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D40B8)==s.archive;}catch(...){return false;}}
inline bool readBytes(uint32_t at,size_t n,std::vector<uint8_t>&out)noexcept{try{out.resize(n);{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(at,n);std::memcpy(out.data(),nfs_host_address(at,n),n);}return true;}catch(...){return false;}}
}
#define NFS_PLAYER_PREVIEW_RUNTIME_TEST 1
#include "hd/PlayerPreviewGameHooks.inc"
#undef NFS_PLAYER_PREVIEW_RUNTIME_TEST
namespace pp=nfs_player_preview_live;namespace pl=nfs_player_scene_live;
static constexpr uint32_t sourceAt=0x600000,paletteAt=0xd000,nameAt=0xc300;
static void originalPlayer(Game&g,Fixture&f,pl::State&owner,unsigned player,bool graph,unsigned selected,unsigned cached,unsigned variant=0){
 prepareMemory(f);select(f.base.post.target);std::memcpy(memory.data()+0x100010,f.base.post.pixels.data(),614400);memory[byte_4DB25C]=16;m32(dword_571264,0);
 owner.base=f.base;owner.form=player==1?nfs_player_preview::Player1Form:nfs_player_preview::Player2Form;owner.formGuest=player==1?off_4D3050:off_4D3100;owner.base.form=owner.form;owner.base.event=owner.event(0);CHECK(pl::readPlane(owner.base.post));owner.baseVerified=true;owner.invalid=false;owner.archive=0x123456;
 memory[byte_512DBF]=memory[byte_512DDA]=uint8_t(selected);memory[byte_512DD1]=graph?1:0;m32(dword_4E5924+2,(cached<<16)|0x5a5a);const uint32_t arrays[4]={dword_4D236C,dword_4D2330,dword_4D23E4,dword_4D23A8};for(unsigned k=0;k<15;++k){auto stats=nfs_player_preview::statistics(k);for(unsigned i=0;i<4;++i)m32(arrays[i]+4*k,stats[i]);}
 const uint32_t fillTable[9]={0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0};for(unsigned k=0;k<9;++k)m32(off_4DB648+4*k,fillTable[k]);
 g.eax=owner.formGuest;g.edx=211;g.ebx=316;
 {pp::Scope frame(g,pp::Site::Initial);
  g.eax=0xff000000;{pp::ColorScope convert(g);g.eax=0;}
  g.eax=211;g.edx=316;g.ebx=280;g.ecx=128;m32(g.esp,0);
  {pp::FillScope fill(g);m32(dword_564140,0);m32(g.esp+0x38,nfs_player_preview::FillCallback);for(unsigned y=0;y<(variant==1?127u:128u);++y){g.eax=0xc500;g.edx=0x100010+(316+y)*1280+211*2;g.ebx=280;{pp::RowScope row(g);std::memset(memory.data()+g.edx,0,560);}}}
  // Exact original40CE28 write precedes every consumed DWORD5924+2/SAR16 read.
  if(variant!=12)m16(dword_4E5928,uint16_t(selected));
  const unsigned consumed=selected;
  const bool bars=graph&&selected<=11;const unsigned draws=bars?5:1;
  for(unsigned index=0;index<draws;++index){std::vector<uint8_t>source(48);put32(source.data(),0x7b|(24u<<8));put32(source.data()+4,3|(2u<<16));std::fill(source.begin()+16,source.begin()+22,1);put32(source.data()+24,0x2a);put32(source.data()+28,2);put32(source.data()+44,0xff112233);std::memcpy(memory.data()+sourceAt,source.data(),source.size());std::memset(memory.data()+paletteAt,0,1024);m32(paletteAt+4,0xff000000|opaque(0xff112233));m32(dword_5636EC,paletteAt);
   static const char names[5][5]={"grph","grpg","grpb","grpr","grpy"};auto name=nfs_player_preview::carName(consumed);if(bars)std::copy_n(names[index],4,name.begin());if(variant==7)name={{'b','a','d','!'}};std::memcpy(memory.data()+nameAt,name.data(),4);memory[nameAt+4]=0;
   const auto stats=nfs_player_preview::statistics(consumed);const int x=bars&&index?237-9*(12-int(stats[index-1])):211;const unsigned y=bars&&index?327+30*(index-1):316;
   std::array<uint32_t,9>prior{};table(prior,true,0,0x7d);for(unsigned k=0;k<9;++k)m32(off_4DAD88+4*k,prior[k]);g.eax=nameAt;g.edx=0x3000;g.ebx=uint32_t(x);g.ecx=y;
   {pp::NameScope named(g);g.eax=sourceAt;{pp::ResolvedScope resolved(g);std::array<uint32_t,9>after{};table(after,true,nfs_player_preview::IndexedCallback,0x7b);for(unsigned k=0;k<9;++k)m32(off_4DAD88+4*k,after[k]);m32(g.esp+0x38,nfs_player_preview::IndexedCallback);
    for(unsigned yy=0;yy<2;++yy){g.eax=sourceAt+16+yy*3;g.edx=0x100010+(y+yy)*1280+unsigned(x)*2;g.ebx=3;{pp::RowScope row(g);for(unsigned xx=0;xx<3;++xx)m16(g.edx+2*xx,opaque(0xff112233));if(variant==2&&yy==1)m16(g.edx,0x9876);if(variant==3&&yy==1)m32(paletteAt+4,0xff000000);}}
    if(variant==4)memory[sourceAt+16]^=1;
   }}
  }
  if(variant==5)owner.baseVerified=false;if(variant==6)m32(dword_4D40B8,0x765432);if(variant==8)memory[player==1?byte_512DBF:byte_512DDA]=uint8_t((selected+1)%15);if(variant==9)m32(dword_4E5924+2,(((cached+1)%15)<<16)|0x5a5a);if(variant==10)m32(dword_4D236C+4*selected,99);if(variant==11)owner.base.event=owner.event(0);
 }
}
int main(){try{
 setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD","1",1);auto f=std::unique_ptr<Fixture>(new Fixture);CHECK(f->verify()==bg::Failure::None);prepareMemory(*f);Game g;auto owner=std::unique_ptr<pl::State>(new pl::State);pl::slot()=owner.get();owner->on=true;owner->game=reinterpret_cast<uintptr_t>(&g);owner->thread=pub::threadToken();owner->ownerEpoch=3;
 std::shared_ptr<const nfs_player_preview::Packet>retained;
 for(unsigned player:{1u,2u})for(bool graph:{false,true}){originalPlayer(g,*f,*owner,player,graph,1,2);if(!owner->preview){auto*s=pp::slot();std::fprintf(stderr,"Player preview failure=%u pure=%u\n",unsigned(s->failure),unsigned(s->verifyFailure));}CHECK(owner->preview&&owner->preview->form()==owner->form);CHECK(owner->preview->selection().selectedCar==1&&owner->preview->selection().cachedCarIndex==1);CHECK(owner->preview->sources().size()==(graph?5:1));CHECK(owner->preview->post()==std::vector<uint16_t>(owner->actualPreviewPost.pixels.begin(),owner->actualPreviewPost.pixels.end()));CHECK(pp::sameEvent(owner->previewBaseEvent,owner->base.event));retained=owner->preview;}
 // Every car name including secretcars; graphenabled secretcar still takes photo.
 for(unsigned car=0;car<15;++car){originalPlayer(g,*f,*owner,2,true,car,car);CHECK(owner->preview&&owner->preview->sources().size()==(car<=11?5:1));}
 // Entry cache can be stale/sentinel; consumed index is the subsequent original write.
 originalPlayer(g,*f,*owner,1,true,1,999);CHECK(owner->preview&&owner->preview->selection().cachedCarIndex==1);
 // Corruptions and current-source changes invalidate rather than lending stale data.
 for(unsigned variant=1;variant<=12;++variant){originalPlayer(g,*f,*owner,1,true,1,2,variant);CHECK(!owner->preview&&!owner->previewSerial&&!owner->previewLifetime);CHECK(pp::slot()->failure!=pp::Failure::None);}
 originalPlayer(g,*f,*owner,1,false,0,0);CHECK(owner->preview);const auto now=reads,alloc=allocationCount.load();pl::feature=false;g.eax=off_4D3050;g.edx=211;g.ebx=316;{pp::Scope disabled(g,pp::Site::Initial);}pl::feature=true;CHECK(reads==now&&allocationCount==alloc);
 g.eax=off_4D3100;{pp::Scope foreignForm(g,pp::Site::Initial);}g.eax=off_4D3050;g.edx=0;{pp::Scope wrongArgs(g,pp::Site::Initial);}g.edx=211;{pp::Scope wrongSite(g,static_cast<pp::Site>(99));}CHECK(reads==now&&allocationCount==alloc);
 std::thread foreign([&]{Game other;other.eax=off_4D3050;other.edx=211;other.ebx=316;pp::Scope unavailable(other,pp::Site::Initial);CHECK(!pp::slot());});foreign.join();CHECK(reads==now&&allocationInLease==0);
 CHECK(retained&&retained->sources().size()==5&&owner->swatchClears>0);owner->on=false;pl::slot()=nullptr;CHECK(retained->post()[316*640+211]==opaque(0xff112233));std::puts("PASS Player preview capture: checked guest scopes, bothforms15cars/photo/fivegraphs,12corruptions/current-source failures,immutable ownership,foreign/disabled zeroreads,no lease allocations; scene owner stub only");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
