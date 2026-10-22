// SPDX-License-Identifier: MIT
// Original-operation simulator using the existing checked guest-memory fixture.
#define main existing_scene_runtime_fixture_main
#include "hd_scene_runtime_tests.cpp"
#undef main
static constexpr uint32_t off_4DB648=0xc100,dword_564140=0xc200;
#include "hd/LocationPreviewGameHooks.inc"
namespace preview=nfs_location_preview_live;
static constexpr uint32_t sourceAt=0x600000,paletteAt=0xd000,nameAt=0xc300;
static void originalPreview(Game&g,Fixture&f,nfs_location_scene_live::State&location,unsigned variant){
 prepareMemory(f);select(f.base.post.target);std::memcpy(memory.data()+0x100010,f.base.post.pixels.data(),614400);memory[byte_4DB25C]=16;m32(dword_571264,0);
 location.base=f.base;location.base.form=nfs_location_preview::LocationForm;location.base.event=location.event(0);location.baseVerified=true;location.invalid=false;location.archive=0x123456;
 const uint32_t fillTable[9]={0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0};for(unsigned k=0;k<9;++k)m32(off_4DB648+4*k,fillTable[k]);
 g.eax=off_4D2A20;g.edx=211;g.ebx=316;
 {preview::Scope frame(g,preview::Site::Initial);
  g.eax=0xff000000;{preview::ColorScope originalColor(g);g.eax=0;}
  g.eax=211;g.edx=316;g.ebx=280;g.ecx=128;m32(g.esp,0);
  {preview::FillScope originalFill(g);m32(dword_564140,0);m32(g.esp+0x38,nfs_location_preview::FillCallback);
   for(unsigned y=0;y<(variant==1?127u:128u);++y){g.eax=0xc500;g.edx=0x100010+(316+y)*1280+211*2;g.ebx=280;{preview::RowScope row(g);std::memset(memory.data()+g.edx,0,560);}}
  }
  std::vector<uint8_t>source(48);put32(source.data(),0x7b|(24u<<8));put32(source.data()+4,3|(2u<<16));std::fill(source.begin()+16,source.begin()+22,1);put32(source.data()+24,0x2a);put32(source.data()+28,2);put32(source.data()+44,0xff112233);std::memcpy(memory.data()+sourceAt,source.data(),source.size());std::memset(memory.data()+paletteAt,0,1024);m32(paletteAt+4,0xff000000|opaque(0xff112233));m32(dword_5636EC,paletteAt);std::memcpy(memory.data()+nameAt,variant==7?"bad!":"trk4",5);
  std::array<uint32_t,9>priorTable{};table(priorTable,true,0,0x7d);for(unsigned k=0;k<9;++k)m32(off_4DAD88+4*k,priorTable[k]);
  g.eax=nameAt;g.edx=0x3000;g.ebx=211;g.ecx=316;
  {preview::NameScope name(g);g.eax=sourceAt;{preview::ResolvedScope resolved(g);std::array<uint32_t,9>after{};table(after,true,nfs_location_preview::IndexedCallback,0x7b);for(unsigned k=0;k<9;++k)m32(off_4DAD88+4*k,after[k]);m32(g.esp+0x38,nfs_location_preview::IndexedCallback);
   for(unsigned y=0;y<2;++y){g.eax=sourceAt+16+y*3;g.edx=0x100010+(316+y)*1280+211*2;g.ebx=3;{preview::RowScope row(g);for(unsigned x=0;x<3;++x)m16(g.edx+2*x,opaque(0xff112233));if(variant==2&&y==1)m16(g.edx,0x9876);if(variant==3&&y==1)m32(paletteAt+4,0xff000000);}}
   if(variant==4)memory[sourceAt+16]^=1;
  }}
  if(variant==5)location.baseVerified=false;
  if(variant==6)m32(dword_4D40B8,0x765432);
 }
}
int main(){try{
 setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD","1",1);auto f=std::unique_ptr<Fixture>(new Fixture);CHECK(f->verify()==bg::Failure::None);CHECK(nfs_background_scene_runtime::installCatalog(std::make_shared<const bg::SourceCatalog>(f->catalog)));prepareMemory(*f);Game g;g.eax=off_4D2A20;std::shared_ptr<const nfs_location_preview::Packet>retained;
 {pub::LocationMenuOwnerScope menu(g);nfs_location_scene_live::LocationScope owner(g);auto*l=nfs_location_scene_live::current(g);CHECK(l);l->baseVerified=true;
  const auto before=reads,alloc=allocationCount.load();g.edx=0;g.ebx=0;{preview::Scope warmup(g,preview::Site::Initial);}CHECK(reads==before&&allocationCount==alloc&&!preview::slot());
  g.eax=off_4D3EC4;g.edx=211;g.ebx=316;{preview::Scope wrongForm(g,preview::Site::Initial);}CHECK(reads==before&&allocationCount==alloc&&!preview::slot());
  originalPreview(g,*f,*l,0);if(!l->preview){auto*s=preview::slot();std::fprintf(stderr,"producer failure=%u verify=%u fill=%u rows=%zu draws=%zu invalid=%u base=%u serial=%llu\n",unsigned(s->failure),unsigned(s->verifyFailure),s->input.fill.complete,s->input.fill.rows.size(),s->input.draws.size(),s->invalid,l->baseVerified,(unsigned long long)s->serial);}CHECK(l->preview&&l->previewSerial&&l->previewLifetime==l->ownerEpoch);CHECK(preview::slot()->failure==preview::Failure::None);CHECK(preview::sameEvent(l->previewBaseEvent,l->base.event)&&l->previewEvent.sequence>l->base.event.sequence);retained=l->preview;const auto serial=l->previewSerial;
  // Independent whole-plane oracle: original map, exact black fill and six opaque source pixels.
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){uint16_t expected=0x2a55;if(x>=211&&x<491&&y>=316&&y<444)expected=0;if(x>=211&&x<214&&y>=316&&y<318)expected=opaque(0xff112233);CHECK(retained->post()[y*640+x]==expected);}
  CHECK(retained->coverage().size()==280*128&&std::all_of(retained->coverage().begin(),retained->coverage().end(),[](uint8_t v){return v==255;}));
  for(unsigned variant=1;variant<=7;++variant){originalPreview(g,*f,*l,variant);CHECK(!l->preview&&!l->previewSerial&&!l->previewLifetime);CHECK(preview::slot()->failure!=preview::Failure::None);}
  originalPreview(g,*f,*l,0);CHECK(l->preview&&l->previewSerial>serial&&l->preview!=retained);CHECK(retained->post()[316*640+211]==opaque(0xff112233));
  const auto accepted=l->preview;const auto counter=l->nextPreviewSerial;l->freshScene();CHECK(l->preview==accepted&&l->nextPreviewSerial==counter);
  l->fail(nfs_location_scene_live::Reason::Target,true);CHECK(!l->preview&&!l->previewSerial&&!l->previewLifetime&&!l->previewEvent.sequence&&!l->previewBaseEvent.sequence&&!l->baseVerified&&l->nextPreviewSerial==counter);
  originalPreview(g,*f,*l,0);CHECK(l->preview);const auto retiredCounter=l->nextPreviewSerial;g.eax=l->base.post.target.owner;nfs_location_scene_live::retire(g);CHECK(!l->preview&&!l->baseVerified&&l->nextPreviewSerial==retiredCounter);
  originalPreview(g,*f,*l,0);CHECK(l->preview);const auto baseCounter=l->nextPreviewSerial;g.eax=aBlk4;g.edx=0x400;g.ebx=g.ecx=0;{nfs_location_scene_live::RoleScope newBase(g,nfs_location_scene_live::Phase::Base,nfs_location_scene_live::BaseArgument::Blk4Direct);CHECK(!l->preview&&!l->previewSerial&&!l->previewLifetime&&l->nextPreviewSerial==baseCounter);}
  originalPreview(g,*f,*l,0);CHECK(l->preview);
  // Reproduce a completed old redraw followed by a normal base refresh and its
  // original warmup upload. The upload must not seal the previous scene.
  l->scene=f->scene;l->scene.form=nfs_location_preview::LocationForm;l->iteration=l->nextIteration=3;l->scene.event={l->ownerEpoch,++l->sequence,3};l->captionSerial=17;l->captionLifetime=l->ownerEpoch;l->groupSerial=17;l->copySerial=18;l->nativeUploadEvent=19;l->groupEvent=l->tailEvent=l->copyEvent=l->uploadEvent=l->scene.event;l->captured483438=true;
  // A transitional upload has already invalidated the prior scene. Only a
  // fresh independent Base capture may recover; Side and nesting cannot.
  l->fail(nfs_location_scene_live::Reason::Upload);l->detail=999;CHECK(l->invalid&&!l->preview);
  {nfs_location_scene_live::RoleScope side(g,nfs_location_scene_live::Phase::Side);CHECK(!side.active&&l->invalid&&l->reason==nfs_location_scene_live::Reason::Upload&&l->detail==999);}
  g.eax=aBlk4;g.edx=0x400;g.ebx=g.ecx=0;l->phase=nfs_location_scene_live::Phase::Base;
  {nfs_location_scene_live::RoleScope nested(g,nfs_location_scene_live::Phase::Base,nfs_location_scene_live::BaseArgument::Blk4Direct);CHECK(!nested.active&&l->invalid&&l->reason==nfs_location_scene_live::Reason::Nested);}
  l->phase=nfs_location_scene_live::Phase::None;l->fail(nfs_location_scene_live::Reason::Upload);l->detail=999;
  m32(off_4D2A20+4,0x7860);m32(0x7860,879455330u);m32(aBlk4,879455330u);
  const auto refreshSequence=l->sequence,refreshNext=l->nextIteration;g.eax=aBlk4;g.edx=0x400;g.ebx=g.ecx=0;
  {nfs_location_scene_live::RoleScope refresh(g,nfs_location_scene_live::Phase::Base,nfs_location_scene_live::BaseArgument::Blk4Direct);
   CHECK(!l->invalid&&l->reason==nfs_location_scene_live::Reason::None&&!l->detail&&!l->baseVerified);CHECK(!l->iteration&&l->nextIteration==refreshNext&&l->sequence==refreshSequence);CHECK(l->phase==nfs_location_scene_live::Phase::Base&&!l->preview&&!l->caption&&!l->captionSerial&&!l->captionLifetime);
   CHECK(l->scene.draws.empty()&&!l->scene.complete&&!l->scene.tailKind&&!l->scene.preSide.valid&&!l->scene.postSide.valid&&!l->scene.tailPre.valid&&!l->scene.tailPost.valid);CHECK(!l->groupSerial&&!l->copySerial&&!l->nativeUploadEvent&&!l->groupEvent.sequence&&!l->tailEvent.sequence&&!l->copyEvent.sequence&&!l->uploadEvent.sequence&&!l->captured483438);
  }
  originalPreview(g,*f,*l,0);CHECK(l->preview&&!l->iteration);const auto refreshed=l->preview;const auto refreshedSerial=l->previewSerial,previewSequence=l->previewEvent.sequence;
  g.edx=1;{nfs_location_scene_live::UploadEndScope warmupUpload(g);}CHECK(!l->invalid&&l->preview==refreshed&&l->previewSerial==refreshedSerial&&!l->iteration&&l->nextIteration==refreshNext);
  g.eax=off_4D2A20;{nfs_location_scene_live::IterationScope lazy(g);CHECK(!l->iteration&&l->preview==refreshed);lazy.start();CHECK(!l->invalid&&l->iteration==refreshNext+1&&l->nextIteration==refreshNext+1&&l->scene.event.sequence>previewSequence&&l->scene.event.iteration==l->iteration&&l->preview==refreshed);}
  CHECK(allocationInLease==0);const auto now=reads;std::thread foreign([&]{Game other;other.eax=off_4D2A20;other.edx=211;other.ebx=316;preview::Scope unavailable(other,preview::Site::Initial);CHECK(!preview::slot());});foreign.join();CHECK(reads==now);
 }
 auto*ended=nfs_location_scene_live::slot();CHECK(ended&&!ended->on&&!ended->preview&&!ended->previewSerial&&!ended->previewLifetime);const auto exitCounter=ended->nextPreviewSerial;
 // A stale independently retained artifact must be cleared by a fresh owner entry.
 ended->preview=retained;ended->previewSerial=1;ended->previewLifetime=1;g.eax=off_4D2A20;{pub::LocationMenuOwnerScope menu(g);nfs_location_scene_live::LocationScope owner(g);CHECK(nfs_location_scene_live::current(g)==ended&&!ended->preview&&!ended->previewSerial&&!ended->previewLifetime&&ended->nextPreviewSerial==exitCounter);}
 CHECK(retained&&retained->post()[316*640+211]==opaque(0xff112233)&&allocationInLease==0);std::puts("PASS actual preview producer: full307200 oracle,7 corruption/lifetime/order rejections,warmup/foreign zero guest reads,real lifecycle invalidation,base-refresh warmup disarming/recovery with Side/nested rejection,immutable retention,no lease allocations");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
