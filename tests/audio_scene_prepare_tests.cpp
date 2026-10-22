// SPDX-License-Identifier: MIT
#include "AudioSidebarFixture.h"
#include "AudioPacketFixture.h"
#include "hd/AudioScenePrepare.h"
#include "hd/AudioSceneLease.h"
#include <thread>
#include <cmath>
namespace ac=nfs_audio_scene_cpu;namespace af=nfs_audio_frame;
struct JoinFixture {
 nfs_audio_test::Fixture a;SidebarFixture s;
 std::shared_ptr<const af::AudioFramePacket>frame;std::shared_ptr<const as::Packet>cache;
 JoinFixture(){
  s.catalog->mapSha[0]=a.mapSha;CHECK(s.catalog->mapSha[1]!=a.mapSha);cache=s.verify();CHECK(cache);
  auto&d=a.background.sidebar;d.args[0]=s.binding.bitmap;d.sidebarHandle=d.sidebarTableEntry=s.binding.sourceOwner;d.sidebarBitmap=s.binding.bitmap;d.sourceBefore=d.sourceAfter=s.before;
  a.background.preBars=a.background.postReset;
  for(unsigned y=0;y<480;++y){auto&r=d.rows[y];r.args[0]=s.binding.bitmap+16+y*686;r.source.assign(s.before.begin()+16+y*686,s.before.begin()+16+(y+1)*686);for(unsigned x=0;x<343;++x){auto v=logo::le16(r.source.data()+x*2);r.after[x]=v?v:r.before[x];a.background.preBars[y*640+x]=r.after[x];}}
  a.buildBars({{70,12,140}},1,0);
  auto bg=af::BackgroundReplayVerifier::verify(a.background,a.binding,a.mapCatalog);CHECK(bg);auto bars=af::BarReplayVerifier::verify(a.bars,a.binding,a.sourceCatalog,a.art);CHECK(bars);frame=af::AudioFramePacket::create(bg,bars,a.text,a.binding,41,a.postText.data(),a.postText.size());CHECK(frame);
 }
 std::shared_ptr<const ac::Prepared>verify(){return ac::Prepared::create(frame,cache,a.binding,41,a.postText.data(),a.postText.size(),s.binding,s.f->scene.preSide.target,s.before.data(),s.before.size());}
};
static void differentCaption(SidebarFixture&s){
 const unsigned at=369*343+123;auto identity=as::surface(s.f->group.post.target,s.captionLifetime);nfs_soft::FramePacketBuilder builder(s.caption->font());CHECK(builder.begin(identity,++s.captionSerial,s.f->group.post.pixels.data(),as::Pixels));nfs_soft::TextCommand text{};std::memcpy(text.text.data(),"options",7);text.length=7;text.role=1;text.alignment=1;text.sourceSize=18;text.x=122;text.y=364;text.advance=92;text.color=0x0ff0;text.clip={0,0,343,480};CHECK(builder.beginString(text));auto post=s.f->group.post.pixels;CHECK(builder.word(at,post[at],0x0ff0,0x0ff0));post[at]=0x0ff0;CHECK(builder.endString(text));s.caption=builder.finish(identity,post.data(),as::Pixels);CHECK(s.caption);for(unsigned k=0;k<as::Pixels;++k)put16(s.before.data()+16+2*k,post[k]);s.after=s.before;
}
int main(){try{
 auto x=std::unique_ptr<JoinFixture>(new JoinFixture);auto p=x->verify();CHECK(p);auto v=p->view(19);CHECK(v.schema==1&&v.serial==19&&v.contentRevision==7&&v.layers.background.mapRole==0);CHECK(v.layers.background.wordCount==1&&v.layers.sidebarCaption.logicalWidth==343&&v.layers.text.logicalWidth==640&&v.layers.barVertices==72);CHECK(v.original565==x->a.text->original().data()&&v.planeWords==307200);CHECK(!p->view(0).schema);
 unsigned negative=0;auto reject=[&]{CHECK(!x->verify());++negative;};
 x->a.binding.form++;reject();x->a.binding.form--;x->a.binding.archive++;reject();x->a.binding.archive--;x->a.binding.ownerEpoch++;reject();x->a.binding.ownerEpoch--;x->a.binding.pass++;reject();x->a.binding.pass--;
 x->s.binding.form++;reject();x->s.binding.form--;x->s.binding.archive++;reject();x->s.binding.archive--;x->s.binding.ownerEpoch++;reject();x->s.binding.ownerEpoch--;x->s.binding.lifetime++;reject();x->s.binding.lifetime--;x->s.binding.sourceOwner++;reject();x->s.binding.sourceOwner--;x->s.binding.bitmap++;reject();x->s.binding.bitmap--;
 x->a.postText[500]^=1;reject();x->a.postText[500]^=1;x->s.before[700]^=1;reject();x->s.before[700]^=1;x->s.f->scene.preSide.target.x[127]^=2;reject();x->s.f->scene.preSide.target.x[127]^=2;
 CHECK(!ac::Prepared::create(x->frame,x->cache,x->a.binding,42,x->a.postText.data(),x->a.postText.size(),x->s.binding,x->s.f->scene.preSide.target,x->s.before.data(),x->s.before.size()));
 // Valid packets from different cache contents/map must still fail their join.
 auto old=x->cache;x->s.catalog->mapSha[0][0]^=1;x->cache=x->s.verify();CHECK(x->cache);reject();x->s.catalog->mapSha[0][0]^=1;x->cache=old;
 x->s.catalog->mapSha[1]=x->s.catalog->mapSha[0];x->cache=x->s.verify();CHECK(!x->cache);x->cache=old;
 // Individually valid cache packets still cannot substitute different source
 // authority or different caption pixels behind an otherwise valid full frame.
 auto other=std::unique_ptr<JoinFixture>(new JoinFixture);other->s.binding.archive++;other->cache=other->s.verify();CHECK(other->cache&&other->cache->matches(other->s.binding,other->s.f->scene.preSide.target,other->s.before.data(),other->s.before.size()));
 CHECK(!ac::Prepared::create(x->frame,other->cache,x->a.binding,41,x->a.postText.data(),x->a.postText.size(),other->s.binding,other->s.f->scene.preSide.target,other->s.before.data(),other->s.before.size()));++negative;other->s.binding.archive--;
 differentCaption(other->s);other->cache=other->s.verify();CHECK(other->cache&&other->cache->raw()!=x->cache->raw()&&other->cache->matches(other->s.binding,other->s.f->scene.preSide.target,other->s.before.data(),other->s.before.size()));
 CHECK(!ac::Prepared::create(x->frame,other->cache,x->a.binding,41,x->a.postText.data(),x->a.postText.size(),other->s.binding,other->s.f->scene.preSide.target,other->s.before.data(),other->s.before.size()));++negative;
 // Either independently authenticated map may be selected; role1 is not a
 // fallback allowance or a fabricated role0 alias.
 auto role1=std::unique_ptr<JoinFixture>(new JoinFixture);std::swap(role1->s.catalog->mapSha[0],role1->s.catalog->mapSha[1]);role1->cache=role1->s.verify();CHECK(role1->cache);auto p1=role1->verify();CHECK(p1&&p1->view(21).layers.background.mapRole==1&&p1->frame()->background()->catalog()->sha()==role1->s.catalog->mapSha[1]);
 CHECK(x->verify());const auto originalMap=p->frame()->background()->catalog()->source();const auto originalSidebar=p->frame()->background()->sidebarSource();const auto expectedArt=x->a.artSha;x->a.map[16]^=1;x->a.background.sidebar.sourceBefore[600]^=1;x->a.sources[0][16]^=1;CHECK(p->frame()->background()->catalog()->source()==originalMap&&p->frame()->background()->sidebarSource()==originalSidebar&&p->sidebar()->raw()==originalSidebar);
 auto bytes=p->sidebar()->raw();other.reset();role1.reset();x.reset();v=p->view(20);CHECK(v.schema==1&&v.layers.text.font.alpha&&v.layers.sidebarCaption.font.alpha&&p->sidebar()->raw()==bytes);CHECK(v.original565==p->frame()->text()->original().data());
 CHECK(v.width==640&&v.height==480&&v.layers.sidebarCaption.logicalHeight==480&&v.layers.text.logicalHeight==480);CHECK(v.layers.background.panelClip[0]==0&&v.layers.background.panelClip[1]==0&&v.layers.background.panelClip[2]==343&&v.layers.background.panelClip[3]==480);
 CHECK(v.layers.sidebarCaption.font.schema==1&&v.layers.text.font.schema==1&&v.layers.sidebarCaption.font.width==1024&&v.layers.sidebarCaption.font.height==512&&v.layers.text.font.width==1024&&v.layers.text.font.height==512&&v.layers.sidebarCaption.font.bytes==1024*512&&v.layers.text.font.bytes==1024*512);
 CHECK(v.layers.bars==p->frame()->layout().vertices.data()&&v.layers.sidebarCaption.vertices==p->sidebar()->caption()->vertices().data()&&v.layers.text.vertices==p->frame()->text()->vertices().data());
 for(unsigned k=0;k<v.layers.barVertices;++k){const auto&q=v.layers.bars[k];CHECK(std::isfinite(q.x)&&std::isfinite(q.y)&&std::isfinite(q.u)&&std::isfinite(q.v)&&q.x>=0&&q.x<=640&&q.y>=0&&q.y<=480&&q.u>=0&&q.u<=1&&q.v>=0&&q.v<=1);}
 CHECK(v.art.schema==2&&v.art.width==2048&&v.art.height==768&&v.art.bytes==NFS_AUDIO_ART_BYTES&&v.art.premultiplied==1&&v.art.opacityBaked==1);std::array<unsigned char,32>digest{};CHECK(nfs_texture_sha256(v.art.pixels,v.art.bytes,digest.data())&&digest==expectedArt&&!std::memcmp(digest.data(),v.art.sha256,32));CHECK(nfs_texture_sha256(p->frame()->background()->catalog()->source().data(),p->frame()->background()->catalog()->source().size(),digest.data())&&digest==p->frame()->background()->catalog()->sha());
 // Model accepted renderer begin followed by worker cancellation: the queue
 // handle may die, while renderer-held C retain keeps every immutable borrow.
 std::weak_ptr<const ac::Prepared>weak=p;auto lease=ac::Lease::create(p);CHECK(lease);auto retained=lease->view(44);CHECK(retained.lifetimeOwner&&retained.retainOwner&&retained.releaseOwner);retained.retainOwner(retained.lifetimeOwner);
 std::vector<std::thread>threads;for(unsigned t=0;t<4;++t)threads.emplace_back([retained]{for(unsigned k=0;k<1000;++k){retained.retainOwner(retained.lifetimeOwner);retained.releaseOwner(retained.lifetimeOwner);}});for(auto&t:threads)t.join();
 p.reset();lease.reset();CHECK(!weak.expired()&&retained.original565&&retained.layers.text.font.alpha&&retained.art.pixels);CHECK(retained.original565[0]==v.original565[0]);retained.releaseOwner(retained.lifetimeOwner);CHECK(weak.expired());
 std::printf("PASS Audio scene immutable join, %u mutation cases, distinct owner lifetimes, cross-valid cache rejection,map-role1,retained full/sidebar/font/art/source layers\n",negative);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
