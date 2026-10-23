// SPDX-License-Identifier: MIT
#include "AudioSidebarFixture.h"
#define main controlsSyntheticProofUnusedMain
#undef CHECK
#include "controls_dialog_word_proof_tests.cpp"
#undef main
#include "hd/ControlsScenePrepare.h"

#include <thread>
#include <type_traits>
namespace gc=nfs_controls_scene_cpu;namespace gf=nfs_graphics_frame;namespace af=nfs_audio_frame;
// EXPLICIT SYNTHETIC sealers only. This TU never includes or executes either
// Controls collector. Neutral replay verifiers below are real production code.
namespace nfs_controls_text {struct TailScope {static std::shared_ptr<const word::Proof>synthetic(Packet p,const word::Evidence&e){return std::shared_ptr<const word::Proof>(new word::Proof(std::move(p),e));}};}
namespace nfs_controls_sidebar_text {struct TailScope {static std::shared_ptr<const nfs_nested_sidebar_caption::Proof>synthetic(Packet p,const nfs_nested_sidebar_caption::Evidence&e){return std::shared_ptr<const nfs_nested_sidebar_caption::Proof>(new nfs_nested_sidebar_caption::Proof(std::move(p),e));}};}
namespace cd=nfs_controls_dialog_display;namespace caption=nfs_nested_sidebar_caption;
struct JoinFixture {
 SidebarFixture s;af::Binding current{};gf::SidebarBinding selected{};gf::BackgroundRecord record{};
 std::vector<uint8_t>map;std::vector<uint16_t>post;std::array<unsigned char,32>mapSha{};
 std::shared_ptr<const af::BackgroundCatalog>mapCatalog;
 std::shared_ptr<const gf::GraphicsFramePacket>frame;
 std::shared_ptr<const as::Packet>cache;word::Evidence dialogEvidence{};caption::Evidence captionEvidence{};std::shared_ptr<const cd::Prepared>dialog;
 JoinFixture(){
  s.binding.form=0x410000;s.f->scene.form=s.binding.form;rebuildCaption();
  auto full=target(640,0x100000,0x41000);
  current.form=s.binding.form;current.owner=full.owner;current.archive=s.binding.archive;
  current.ownerEpoch=s.binding.ownerEpoch;current.pass=7;current.lifetime=current.surface.lifetime=31;
  std::copy(full.w.begin(),full.w.end(),current.surface.geometry.begin());
  auto&g=current.surface.geometry;g[14]=full.x[0];g[15]=full.x[1];g[16]=full.x[639];g[17]=full.y[0];g[18]=full.y[1];g[19]=full.y[479];CHECK(current.valid());
  map.resize(614416);put32(map.data(),0x78);put32(map.data()+4,640u|(480u<<16));
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)put16(map.data()+16+2*(y*640+x),uint16_t(1+(x*13+y*31)%65535));
  CHECK(nfs_texture_sha256(map.data(),map.size(),mapSha.data()));
  mapCatalog=af::BackgroundCatalog::create(mapSha,map);CHECK(mapCatalog);
  s.catalog->mapSha[0]=mapSha;CHECK(s.catalog->mapSha[1]!=mapSha);cache=makeCache();CHECK(cache);
  selected.target=s.f->scene.preSide.target;selected.target.ownerState[0]=0;selected.word=0;
  selected.handle=selected.tableEntry=s.binding.sourceOwner;selected.lifetime=17;selected.source=s.before;CHECK(selected.valid());
  record.binding=current;record.begin=1;record.end=6;record.complete=1;
  record.sidebarBefore=record.sidebarAfter=selected;
  record.preReset.resize(gf::Pixels);record.postReset.resize(gf::Pixels);record.postSidebar.resize(gf::Pixels);
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){size_t at=y*640+x;uint16_t m=logo::le16(map.data()+16+2*at);record.preReset[at]=uint16_t(m^0xffff);record.postReset[at]=record.postSidebar[at]=m;if(x<343){uint16_t v=logo::le16(s.before.data()+16+2*(y*343+x));if(v)record.postSidebar[at]=v;}}
  auto draw=[&](gf::ScopedDraw&d,uint64_t begin,uint32_t source,uint32_t mode,const std::vector<uint8_t>&bytes){d.entry=d.exit=current;d.before=d.after=full;d.begin=begin;d.end=begin+1;d.complete=1;d.args={{source,mode,0,0}};d.sourceBefore=d.sourceAfter=bytes;};
  draw(record.reset,2,0x800000,0x400,map);draw(record.sidebar,4,s.binding.bitmap,0x1000,s.before);
  auto background=gf::BackgroundVerifier::verify(record,current,selected,mapCatalog);CHECK(background);
  nfs_soft::FramePacketBuilder builder(font());CHECK(builder.begin(current.surface,41,background->pixels().data(),gf::Pixels));
  post=background->pixels();auto strings=commands(0);
  for(unsigned i=0;i<strings.size();++i){CHECK(builder.beginString(strings[i]));const uint16_t value=uint16_t(0x8000+i);CHECK(builder.word(i,post[i],value,value));post[i]=value;CHECK(builder.endString(strings[i]));}
  auto text=builder.finish(current.surface,post.data(),post.size());CHECK(text&&text->commands()==16&&text->vertices().size()==696);
  dialogEvidence=evidence(0,41);dialogEvidence.form=dialogEvidence.source.form=current.form;dialogEvidence.authority.parentWords[1]=current.form;dialogEvidence.authority.archive=current.archive;dialogEvidence.thread=s.binding.thread;dialogEvidence.lifetime=current.lifetime;
  auto&snap=dialogEvidence.post;snap.identity=current.surface;snap.target=full;snap.archive=current.archive;
  for(auto&r:dialogEvidence.resources){const auto color=r.after.color;r.before=r.resolvedState=r.measure=r.after=snap;r.after.color=color;}
  dialog=makeDialog(dialogEvidence,text);CHECK(dialog);
  frame=gf::GraphicsFramePacket::create(background,text,current,41,post.data(),post.size());CHECK(frame);
 }
 static std::shared_ptr<const cd::Prepared>makeDialog(const word::Evidence&e,Packet p){auto proof=nfs_controls_text::TailScope::synthetic(p,e);return cd::Prepared::create(proof,p,{e.form,e.authority.archive,e.game,e.serial,e.lifetime});}
 void rebuildCaption(){
  const auto id=as::surface(s.f->group.post.target,s.captionLifetime);nfs_soft::FramePacketBuilder b(font());CHECK(b.begin(id,s.captionSerial,s.f->group.post.pixels.data(),as::Pixels));
  nfs_soft::TextCommand c{};std::memcpy(c.text.data(),"options",7);c.length=7;c.role=1;c.alignment=2;c.sourceSize=18;c.x=120;c.y=364;c.advance=48;c.color=65535;c.clip={0,0,343,480};
  auto pixels=s.f->group.post.pixels;const unsigned at=369*343+123;CHECK(b.beginString(c)&&b.word(at,pixels[at],65535,65535)&&b.endString(c));pixels[at]=65535;s.caption=b.finish(id,pixels.data(),as::Pixels);CHECK(s.caption);
  s.before.resize(as::Bytes);for(unsigned k=0;k<4;++k)put32(s.before.data()+4*k,s.f->group.post.target.header[k]);for(unsigned k=0;k<as::Pixels;++k)put16(s.before.data()+16+2*k,pixels[k]);s.after=s.before;
  auto&e=captionEvidence;e.policy=caption::Policy::Controls;e.form=e.incomingEbp=s.binding.form;e.textGuest=0x12300;e.game=0x900001;e.thread=s.binding.thread;e.serial=s.captionSerial;e.lifetime=s.captionLifetime;e.measureWidth=48;e.measureAnchor=168;e.measureKind=2;e.command=c;
  caption::Snapshot snap;snap.target=s.f->group.post.target;snap.identity=id;snap.archive=s.binding.archive;snap.selector=6;snap.entry=0x12400;snap.entryWord=0x70002;e.pre=e.measure=e.before=e.after=e.post=snap;
 }
 std::shared_ptr<const as::Packet>makeCache(){auto proof=nfs_controls_sidebar_text::TailScope::synthetic(s.caption,captionEvidence);as::Input input{s.binding,s.f->scene,s.f->group,s.caption,s.captionSerial,s.captionLifetime,s.before,s.after,proof,{captionEvidence.policy,s.binding.form,s.binding.archive,captionEvidence.game,s.captionSerial,s.captionLifetime}};return as::Packet::create(input,s.binding,s.f->scene.preSide.target,s.catalog);}
 std::shared_ptr<const gc::Prepared>verify(){return gc::Prepared::create(frame,cache,current,41,post.data(),post.size(),selected,s.binding,s.f->scene.preSide.target,s.before.data(),s.before.size(),dialog);}
};

int main(){try{
 static_assert(!std::is_default_constructible<gc::Prepared>::value,"private verified join only");
 auto x=std::unique_ptr<JoinFixture>(new JoinFixture);auto p=x->verify();CHECK(p);auto v=p->view(19);
 CHECK(v.schema==NFS_CONTROLS_SCENE_RUNTIME_SCHEMA&&v.layers.sidebarWord.word==3&&v.layers.dialogWords.count==10);
 CHECK(v.width==640&&v.height==480&&v.serial==19&&v.contentRevision==7&&v.planeWords==gf::Pixels);
 CHECK(!v.lifetimeOwner&&!v.retainOwner&&!v.releaseOwner&&!p->view(0).schema&&!gc::Lease::create({}));
 CHECK(v.layers.text.vertices==x->frame->text()->vertices().data()&&v.original565==x->frame->text()->original().data());
 CHECK(v.layers.background.mapRole==0&&v.layers.background.wordCount==1&&v.layers.sidebarCaption.logicalWidth==343&&v.layers.text.logicalWidth==640);
 unsigned negatives=0;auto reject=[&]{CHECK(!x->verify());++negatives;};
 x->current.form++;reject();x->current.form--;x->current.owner++;reject();x->current.owner--;x->current.archive++;reject();x->current.archive--;x->current.ownerEpoch++;reject();x->current.ownerEpoch--;x->current.pass++;reject();x->current.pass--;x->current.lifetime++;reject();x->current.lifetime--;x->current.surface.geometry[8]++;reject();x->current.surface.geometry[8]--;
 x->s.binding.form++;reject();x->s.binding.form--;x->s.binding.archive++;reject();x->s.binding.archive--;x->s.binding.ownerEpoch++;reject();x->s.binding.ownerEpoch--;x->s.binding.lifetime++;reject();x->s.binding.lifetime--;x->s.binding.sourceOwner++;reject();x->s.binding.sourceOwner--;x->s.binding.bitmap++;reject();x->s.binding.bitmap--;x->s.binding.thread++;reject();x->s.binding.thread--;
 x->post[12345]^=1;reject();x->post[12345]^=1;x->s.before[700]^=1;reject();x->s.before[700]^=1;x->s.f->scene.preSide.target.x[127]^=2;reject();x->s.f->scene.preSide.target.x[127]^=2;
 x->selected.lifetime++;reject();x->selected.lifetime--;x->selected.tableEntry++;reject();x->selected.tableEntry--;x->selected.handle++;reject();x->selected.handle--;x->selected.word^=0x10000;reject();x->selected.word^=0x10000;x->selected.source[200]^=1;reject();x->selected.source[200]^=1;x->selected.target.ownerState[3]++;reject();x->selected.target.ownerState[3]--;
 auto join=[&](std::shared_ptr<const gf::GraphicsFramePacket>frame,uint64_t serial,const uint16_t*post,size_t count){return gc::Prepared::create(frame,x->cache,x->current,serial,post,count,x->selected,x->s.binding,x->s.f->scene.preSide.target,x->s.before.data(),x->s.before.size(),x->dialog);};
 CHECK(!join({},41,x->post.data(),x->post.size()));CHECK(!join(x->frame,42,x->post.data(),x->post.size()));CHECK(!join(x->frame,41,nullptr,x->post.size()));CHECK(!join(x->frame,41,x->post.data(),x->post.size()-1));negatives+=4;
 auto savedDialog=x->dialog;x->dialog.reset();reject();x->dialog=savedDialog;
 for(unsigned n=0;n<2;++n){auto e=x->dialogEvidence;if(n)++e.thread;else ++e.game;x->dialog=JoinFixture::makeDialog(e,x->frame->text());CHECK(x->dialog);reject();}x->dialog=savedDialog;
 auto savedCache=x->cache;for(auto policy:{caption::Policy::Audio,caption::Policy::Graphics}){x->captionEvidence.policy=policy;x->cache=x->makeCache();CHECK(x->cache);reject();}x->captionEvidence.policy=caption::Policy::Controls;x->cache=savedCache;
 x->cache=x->s.verify();CHECK(x->cache&&!x->cache->captionWord());reject();x->cache=savedCache;
 auto other=std::unique_ptr<JoinFixture>(new JoinFixture);x->dialog=other->dialog;reject();x->dialog=savedDialog;
 // A valid independently rebuilt cache with an unrelated map catalog refuses.
 x->s.catalog->mapSha[0][0]^=1;x->cache=x->makeCache();CHECK(x->cache);reject();x->s.catalog->mapSha[0][0]^=1;x->cache=savedCache;
 auto role1=std::unique_ptr<JoinFixture>(new JoinFixture);std::swap(role1->s.catalog->mapSha[0],role1->s.catalog->mapSha[1]);role1->cache=role1->makeCache();CHECK(role1->cache);auto p1=role1->verify();CHECK(p1&&p1->view(21).layers.background.mapRole==1);
 CHECK(x->verify());auto retainedMap=p->frame()->background()->catalog()->source();auto retainedSidebar=p->sidebar()->raw();auto retainedPost=p->frame()->text()->original();
 x->map[16]^=1;x->record.sidebar.sourceBefore[600]^=1;x->selected.source[601]^=1;x->s.before[602]^=1;x->dialogEvidence.game++;
 CHECK(p->frame()->background()->catalog()->source()==retainedMap&&p->sidebar()->raw()==retainedSidebar&&p->frame()->text()->original()==retainedPost);
 std::weak_ptr<const gc::Prepared>weak=p;auto lease=gc::Lease::create(p);CHECK(lease&&!lease->view(0).schema);auto held=lease->view(44);CHECK(held.lifetimeOwner&&held.retainOwner&&held.releaseOwner);held.retainOwner(held.lifetimeOwner);
 savedDialog.reset();savedCache.reset();other.reset();role1.reset();p1.reset();x.reset();p.reset();lease.reset();CHECK(!weak.expired());
 std::vector<std::thread>threads;for(unsigned t=0;t<4;++t)threads.emplace_back([held]{for(unsigned k=0;k<1000;++k){held.retainOwner(held.lifetimeOwner);held.releaseOwner(held.lifetimeOwner);}});for(auto&t:threads)t.join();
 CHECK(held.original565[0]==retainedPost[0]&&held.layers.text.font.alpha&&held.layers.sidebarCaption.font.alpha&&held.layers.sidebarWord.rgba&&held.layers.dialogWords.images[0].rgba);
 std::thread foreignRelease([held]{held.releaseOwner(held.lifetimeOwner);});foreignRelease.join();CHECK(weak.expired());
 std::printf("PASS Controls scene %u negatives, SYNTHETIC sealers plus real original replay/cache verifiers, mandatory5word/10span dialog, retained immutable lease; no collector/native execution claimed\n",negatives);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}