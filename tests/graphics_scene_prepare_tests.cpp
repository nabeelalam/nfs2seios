// SPDX-License-Identifier: MIT
#include "AudioSidebarFixture.h"
#include "hd/GraphicsScenePrepare.h"
#include "hd/GraphicsSceneLease.h"
#include <thread>
#include <type_traits>
namespace gc=nfs_graphics_scene_cpu;namespace gf=nfs_graphics_frame;namespace af=nfs_audio_frame;
struct JoinFixture {
 SidebarFixture s;af::Binding current{};gf::SidebarBinding selected{};gf::BackgroundRecord record{};
 std::vector<uint8_t>map;std::vector<uint16_t>post;std::array<unsigned char,32>mapSha{};
 std::shared_ptr<const af::BackgroundCatalog>mapCatalog;
 std::shared_ptr<const gf::GraphicsFramePacket>frame;
 std::shared_ptr<const as::Packet>cache;
 JoinFixture(){
  auto full=target(640,0x100000,0x41000);
  current.form=s.binding.form;current.owner=full.owner;current.archive=s.binding.archive;
  current.ownerEpoch=s.binding.ownerEpoch;current.pass=7;current.lifetime=current.surface.lifetime=31;
  std::copy(full.w.begin(),full.w.end(),current.surface.geometry.begin());
  auto&g=current.surface.geometry;g[14]=full.x[0];g[15]=full.x[1];g[16]=full.x[639];g[17]=full.y[0];g[18]=full.y[1];g[19]=full.y[479];CHECK(current.valid());
  map.resize(614416);put32(map.data(),0x78);put32(map.data()+4,640u|(480u<<16));
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)put16(map.data()+16+2*(y*640+x),uint16_t(1+(x*13+y*31)%65535));
  CHECK(nfs_texture_sha256(map.data(),map.size(),mapSha.data()));
  mapCatalog=af::BackgroundCatalog::create(mapSha,map);CHECK(mapCatalog);
  s.catalog->mapSha[0]=mapSha;CHECK(s.catalog->mapSha[1]!=mapSha);cache=s.verify();CHECK(cache);
  selected.target=s.f->scene.preSide.target;selected.target.ownerState[0]=0;selected.word=0;
  selected.handle=selected.tableEntry=s.binding.sourceOwner;selected.lifetime=17;selected.source=s.before;CHECK(selected.valid());
  record.binding=current;record.begin=1;record.end=6;record.complete=1;
  record.sidebarBefore=record.sidebarAfter=selected;
  record.preReset.resize(gf::Pixels);record.postReset.resize(gf::Pixels);record.postSidebar.resize(gf::Pixels);
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){size_t at=y*640+x;uint16_t m=logo::le16(map.data()+16+2*at);record.preReset[at]=uint16_t(m^0xffff);record.postReset[at]=record.postSidebar[at]=m;if(x<343){uint16_t v=logo::le16(s.before.data()+16+2*(y*343+x));if(v)record.postSidebar[at]=v;}}
  auto draw=[&](gf::ScopedDraw&d,uint64_t begin,uint32_t source,uint32_t mode,const std::vector<uint8_t>&bytes){d.entry=d.exit=current;d.before=d.after=full;d.begin=begin;d.end=begin+1;d.complete=1;d.args={{source,mode,0,0}};d.sourceBefore=d.sourceAfter=bytes;};
  draw(record.reset,2,0x800000,0x400,map);draw(record.sidebar,4,s.binding.bitmap,0x1000,s.before);
  auto background=gf::BackgroundVerifier::verify(record,current,selected,mapCatalog);CHECK(background);
  nfs_soft::FramePacketBuilder builder(s.caption->font());CHECK(builder.begin(current.surface,41,background->pixels().data(),gf::Pixels));
  nfs_soft::TextCommand c{};std::memcpy(c.text.data(),"GRAPHICS",8);c.length=8;c.sourceSize=18;c.x=350;c.y=100;c.advance=8;c.color=0xffff;c.clip={0,0,640,480};
  post=background->pixels();const size_t at=120*640+350;auto before=post[at];post[at]^=0xffff;
  CHECK(builder.beginString(c)&&builder.word(at,before,post[at],post[at])&&builder.endString(c));
  auto text=builder.finish(current.surface,post.data(),post.size());CHECK(text);
  frame=gf::GraphicsFramePacket::create(background,text,current,41,post.data(),post.size());CHECK(frame);
 }
 std::shared_ptr<const gc::Prepared>verify(){return gc::Prepared::create(frame,cache,current,41,post.data(),post.size(),selected,s.binding,s.f->scene.preSide.target,s.before.data(),s.before.size());}
};
static void differentCaption(SidebarFixture&s){
 const unsigned at=369*343+123;auto identity=as::surface(s.f->group.post.target,s.captionLifetime);nfs_soft::FramePacketBuilder builder(s.caption->font());CHECK(builder.begin(identity,++s.captionSerial,s.f->group.post.pixels.data(),as::Pixels));nfs_soft::TextCommand text{};std::memcpy(text.text.data(),"options",7);text.length=7;text.role=1;text.alignment=2;text.sourceSize=18;text.x=120;text.y=364;text.advance=48;text.color=0x0ff0;text.clip={0,0,343,480};CHECK(builder.beginString(text));auto post=s.f->group.post.pixels;CHECK(builder.word(at,post[at],0x0ff0,0x0ff0));post[at]=0x0ff0;CHECK(builder.endString(text));s.caption=builder.finish(identity,post.data(),as::Pixels);CHECK(s.caption);for(unsigned k=0;k<as::Pixels;++k)put16(s.before.data()+16+2*k,post[k]);s.after=s.before;
}
int main(){try{
 static_assert(!std::is_default_constructible<gc::Prepared>::value,"verified join issuer only");
 auto x=std::unique_ptr<JoinFixture>(new JoinFixture);auto p=x->verify();CHECK(p);auto v=p->view(19);
 CHECK(v.schema==NFS_GRAPHICS_SCENE_RUNTIME_SCHEMA&&v.serial==19&&v.contentRevision==7&&v.width==640&&v.height==480);
 CHECK(v.original565==x->frame->text()->original().data()&&v.planeWords==gf::Pixels);
 CHECK(v.layers.background.mapRole==0&&v.layers.background.wordCount==1&&v.layers.sidebarCaption.logicalWidth==343&&v.layers.text.logicalWidth==640);
 CHECK(!p->view(0).schema);CHECK(!gc::Lease::create({}));
 unsigned negative=0;auto reject=[&]{CHECK(!x->verify());++negative;};
 x->current.form++;reject();x->current.form--;x->current.archive++;reject();x->current.archive--;x->current.ownerEpoch++;reject();x->current.ownerEpoch--;x->current.pass++;reject();x->current.pass--;x->current.lifetime++;reject();x->current.lifetime--;
 x->s.binding.form++;reject();x->s.binding.form--;x->s.binding.archive++;reject();x->s.binding.archive--;x->s.binding.ownerEpoch++;reject();x->s.binding.ownerEpoch--;x->s.binding.lifetime++;reject();x->s.binding.lifetime--;x->s.binding.sourceOwner++;reject();x->s.binding.sourceOwner--;x->s.binding.bitmap++;reject();x->s.binding.bitmap--;
 x->post[12345]^=1;reject();x->post[12345]^=1;x->s.before[700]^=1;reject();x->s.before[700]^=1;x->s.f->scene.preSide.target.x[127]^=2;reject();x->s.f->scene.preSide.target.x[127]^=2;
 x->selected.lifetime++;reject();x->selected.lifetime--;x->selected.tableEntry++;reject();x->selected.tableEntry--;x->selected.handle++;reject();x->selected.handle--;x->selected.word^=0x10000;reject();x->selected.word^=0x10000;x->selected.source[200]^=1;reject();x->selected.source[200]^=1;x->selected.target.ownerState[3]++;reject();x->selected.target.ownerState[3]--;
 CHECK(!gc::Prepared::create(x->frame,x->cache,x->current,42,x->post.data(),x->post.size(),x->selected,x->s.binding,x->s.f->scene.preSide.target,x->s.before.data(),x->s.before.size()));
 CHECK(!gc::Prepared::create(x->frame,x->cache,x->current,41,nullptr,x->post.size(),x->selected,x->s.binding,x->s.f->scene.preSide.target,x->s.before.data(),x->s.before.size()));
 CHECK(!gc::Prepared::create(x->frame,x->cache,x->current,41,x->post.data(),x->post.size()-1,x->selected,x->s.binding,x->s.f->scene.preSide.target,x->s.before.data(),x->s.before.size()));
 CHECK(!gc::Prepared::create({},x->cache,x->current,41,x->post.data(),x->post.size(),x->selected,x->s.binding,x->s.f->scene.preSide.target,x->s.before.data(),x->s.before.size()));
 CHECK(!gc::Prepared::create(x->frame,{},x->current,41,x->post.data(),x->post.size(),x->selected,x->s.binding,x->s.f->scene.preSide.target,x->s.before.data(),x->s.before.size()));
 // A separately valid map catalog or caption/cache cannot replace the authority
 // already joined into the immutable full640 background/text proof.
 auto old=x->cache;x->s.catalog->mapSha[0][0]^=1;x->cache=x->s.verify();CHECK(x->cache);reject();x->s.catalog->mapSha[0][0]^=1;x->cache=old;
 auto other=std::unique_ptr<JoinFixture>(new JoinFixture);other->s.binding.archive++;other->cache=other->s.verify();CHECK(other->cache);CHECK(!gc::Prepared::create(x->frame,other->cache,x->current,41,x->post.data(),x->post.size(),x->selected,other->s.binding,other->s.f->scene.preSide.target,other->s.before.data(),other->s.before.size()));++negative;other->s.binding.archive--;
 differentCaption(other->s);other->cache=other->s.verify();CHECK(other->cache&&other->cache->raw()!=x->cache->raw());CHECK(!gc::Prepared::create(x->frame,other->cache,x->current,41,x->post.data(),x->post.size(),x->selected,other->s.binding,other->s.f->scene.preSide.target,other->s.before.data(),other->s.before.size()));++negative;
 auto role1=std::unique_ptr<JoinFixture>(new JoinFixture);std::swap(role1->s.catalog->mapSha[0],role1->s.catalog->mapSha[1]);role1->cache=role1->s.verify();CHECK(role1->cache);auto p1=role1->verify();CHECK(p1&&p1->view(21).layers.background.mapRole==1);
 CHECK(x->verify());const auto originalMap=p->frame()->background()->catalog()->source();const auto originalSidebar=p->frame()->background()->sidebar().source;
 x->map[16]^=1;x->record.sidebar.sourceBefore[600]^=1;x->selected.source[601]^=1;x->s.before[602]^=1;
 CHECK(p->frame()->background()->catalog()->source()==originalMap&&p->frame()->background()->sidebar().source==originalSidebar&&p->sidebar()->raw()==originalSidebar);
 auto bytes=p->sidebar()->raw();other.reset();role1.reset();x.reset();v=p->view(20);CHECK(v.schema==1&&p->sidebar()->raw()==bytes);
 CHECK(v.layers.sidebarCaption.logicalHeight==480&&v.layers.text.logicalHeight==480);
 CHECK(v.layers.background.panelClip[0]==0&&v.layers.background.panelClip[1]==0&&v.layers.background.panelClip[2]==343&&v.layers.background.panelClip[3]==480);
 CHECK(v.layers.sidebarCaption.font.schema==1&&v.layers.text.font.schema==1&&v.layers.sidebarCaption.font.width==1024&&v.layers.text.font.width==1024&&v.layers.sidebarCaption.font.bytes==1024*512&&v.layers.text.font.bytes==1024*512);
 CHECK(v.layers.sidebarCaption.vertices==p->sidebar()->caption()->vertices().data()&&v.layers.text.vertices==p->frame()->text()->vertices().data());
 std::array<unsigned char,32>digest{};
 CHECK(nfs_texture_sha256(v.layers.sidebarCaption.font.alpha,v.layers.sidebarCaption.font.bytes,digest.data())&&!std::memcmp(digest.data(),v.layers.sidebarCaption.font.sha256,32));
 CHECK(nfs_texture_sha256(v.layers.text.font.alpha,v.layers.text.font.bytes,digest.data())&&!std::memcmp(digest.data(),v.layers.text.font.sha256,32));
 CHECK(nfs_texture_sha256(p->frame()->background()->catalog()->source().data(),p->frame()->background()->catalog()->source().size(),digest.data())&&digest==p->frame()->background()->catalog()->sha());
 // Renderer retain outlives queue cancellation and all external fixture owners.
 std::weak_ptr<const gc::Prepared>weak=p;auto lease=gc::Lease::create(p);CHECK(lease);CHECK(!lease->view(0).schema);auto retained=lease->view(44);CHECK(retained.lifetimeOwner&&retained.retainOwner&&retained.releaseOwner);retained.retainOwner(retained.lifetimeOwner);
 std::vector<std::thread>threads;for(unsigned t=0;t<4;++t)threads.emplace_back([retained]{for(unsigned k=0;k<1000;++k){retained.retainOwner(retained.lifetimeOwner);retained.releaseOwner(retained.lifetimeOwner);}});for(auto&t:threads)t.join();
 p.reset();lease.reset();CHECK(!weak.expired()&&retained.original565&&retained.layers.text.font.alpha&&retained.layers.sidebarCaption.font.alpha);CHECK(retained.original565[0]==v.original565[0]);retained.releaseOwner(retained.lifetimeOwner);CHECK(weak.expired());
 std::printf("PASS Graphics scene immutable join, %u mutation cases, independent selected-cache and caption lifetimes, both map roles, retained full/sidebar/font/source layers\n",negative);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
