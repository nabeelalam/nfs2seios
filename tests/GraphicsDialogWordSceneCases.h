// SPDX-License-Identifier: MIT
#pragma once
#include "AudioSidebarFixture.h"
#include "hd/GraphicsScenePrepare.h"
#include "hd/GraphicsSceneLease.h"
// Included after the genuine Graphics lexical source fixture. Original full640
// reset/sidebar replay and independently current retained343 bindings remain
// mandatory; no caller-created finite proof or public proof factory is used.
namespace graphics_dialog_word_scene_cases {
namespace gf=nfs_graphics_frame;namespace af=nfs_audio_frame;namespace gc=nfs_graphics_scene_cpu;
struct Capture {
 std::shared_ptr<const nfs_soft::FramePacket>text;
 std::shared_ptr<const nfs_graphics_dialog_word::Proof>proof;
 std::shared_ptr<const nfs_graphics_dialog_display::Prepared>dialog;
 std::shared_ptr<const gf::GraphicsFramePacket>frame;
 std::vector<uint16_t>post;uint64_t serial=0;
};
inline void run(){
 prepare();graphics_dialog_word_cases::resources();auto sidebar=std::unique_ptr<SidebarFixture>(new SidebarFixture);
 Game game;game.eax=off_4D38D8;NfsGraphicsSceneFrameView retainedView{};std::weak_ptr<const gc::Prepared>weakScene;
 {cap::OwnerScope owner(game);auto*source=cap::current(game);assert(source);
  const auto full=target(640,0x100000,0x41000);af::Binding current{};current.form=off_4D38D8;current.owner=full.owner;current.archive=0x1234;current.ownerEpoch=3;current.pass=7;current.lifetime=current.surface.lifetime=source->lifetime;
  std::copy(full.w.begin(),full.w.end(),current.surface.geometry.begin());auto&geometry=current.surface.geometry;geometry[14]=full.x[0];geometry[15]=full.x[1];geometry[16]=full.x[639];geometry[17]=full.y[0];geometry[18]=full.y[1];geometry[19]=full.y[479];assert(current.valid());
  std::vector<uint8_t>map(614416);put32(map.data(),0x78);put32(map.data()+4,640u|(480u<<16));for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)put16(map.data()+16+2*(y*640+x),uint16_t(1+(x*13+y*31)%65535));
  std::array<unsigned char,32>mapSha{};assert(nfs_texture_sha256(map.data(),map.size(),mapSha.data()));auto mapCatalog=af::BackgroundCatalog::create(mapSha,map);assert(mapCatalog);
  sidebar->binding.form=off_4D38D8;sidebar->binding.archive=current.archive;sidebar->binding.ownerEpoch=current.ownerEpoch;sidebar->f->scene.form=current.form;sidebar->catalog->mapSha[0]=mapSha;auto cache=sidebar->verify();assert(cache&&!cache->captionWord());
  gf::SidebarBinding sidebarBinding;sidebarBinding.target=sidebar->f->scene.preSide.target;sidebarBinding.target.ownerState[0]=0;sidebarBinding.word=0;sidebarBinding.handle=sidebarBinding.tableEntry=sidebar->binding.sourceOwner;sidebarBinding.lifetime=17;sidebarBinding.source=sidebar->before;assert(sidebarBinding.valid());
  gf::BackgroundRecord record{};record.binding=current;record.begin=1;record.end=6;record.complete=1;record.sidebarBefore=record.sidebarAfter=sidebarBinding;record.preReset.resize(gf::Pixels);record.postReset.resize(gf::Pixels);record.postSidebar.resize(gf::Pixels);
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){const size_t at=y*640+x;const auto word=logo::le16(map.data()+16+2*at);record.preReset[at]=uint16_t(word^0xffff);record.postReset[at]=record.postSidebar[at]=word;if(x<343){const auto side=logo::le16(sidebar->before.data()+16+2*(y*343+x));if(side)record.postSidebar[at]=side;}}
  auto draw=[&](gf::ScopedDraw&d,uint64_t begin,uint32_t address,uint32_t mode,const std::vector<uint8_t>&bytes){d.entry=d.exit=current;d.before=d.after=full;d.begin=begin;d.end=begin+1;d.complete=1;d.args={{address,mode,0,0}};d.sourceBefore=d.sourceAfter=bytes;};draw(record.reset,2,0x800000,0x400,map);draw(record.sidebar,4,sidebar->binding.bitmap,0x1000,sidebar->before);
  auto background=gf::BackgroundVerifier::verify(record,current,sidebarBinding,mapCatalog);assert(background&&background->pixels()==record.postSidebar);
  auto capture=[&](){Capture result;selected=full;std::memcpy(memory.data()+full.w[8],full.header.data(),16);std::memcpy(memory.data()+full.w[8]+16,background->pixels().data(),gf::Pixels*2);game.eax=off_4D38D8;{cap::TailScope tail(game);graphics_dialog_word_cases::fullTail(game,"normal",0,0x1e0000);}result.text=cap::lastPacket(game);result.proof=cap::lastDialogProof(game);assert(result.text&&result.proof&&result.proof->packet()==result.text&&cap::acquireCurrentDialogProof(game)==result.proof&&result.text->clean()==background->pixels());result.serial=result.text->serial();result.post=result.text->original();result.dialog=nfs_graphics_dialog_display::Prepared::create(result.proof,result.text,{current.form,current.archive,reinterpret_cast<uintptr_t>(&game),result.serial,source->lifetime});assert(result.dialog);result.frame=gf::GraphicsFramePacket::create(background,result.text,current,result.serial,result.post.data(),result.post.size());assert(result.frame&&result.frame->text()==result.text);return result;};
  auto first=capture();auto currentBinding=current;auto currentSidebar=sidebarBinding;auto currentCache=sidebar->binding;auto currentTarget=sidebar->f->scene.preSide.target;auto currentBytes=sidebar->before;auto currentPost=first.post;uint64_t accepted=first.serial;
  auto join=[&](const Capture&candidate,const std::shared_ptr<const nfs_graphics_dialog_display::Prepared>&dialog){return gc::Prepared::create(candidate.frame,cache,currentBinding,accepted,currentPost.data(),currentPost.size(),currentSidebar,currentCache,currentTarget,currentBytes.data(),currentBytes.size(),dialog);};
  auto scene=join(first,first.dialog);assert(scene&&scene->frame()->text()==first.text);auto view=scene->view(180);assert(view.schema==NFS_GRAPHICS_SCENE_RUNTIME_SCHEMA&&view.layers.dialogWords.schema==NFS_GRAPHICS_DIALOG_WORD_SCHEMA&&view.layers.dialogWords.count==16&&view.original565==first.text->original().data()&&view.layers.text.vertices==first.text->vertices().data());
  for(unsigned i=0;i<8;++i)for(unsigned j=0;j<2;++j){const auto&r=first.text->records()[nfs_graphics_dialog_word::record(i)+j];const auto&p=view.layers.dialogWords.placements[2*i+j];assert(p.vertexFirst==r.vertexFirst&&p.vertexCount==r.vertexCount&&p.color565==r.command.color&&p.left==r.command.x&&p.top==r.command.y);}
  auto restore=[&](){currentBinding=current;currentSidebar=sidebarBinding;currentCache=sidebar->binding;currentTarget=sidebar->f->scene.preSide.target;currentBytes=sidebar->before;currentPost=first.post;accepted=first.serial;};
  for(unsigned fault=0;fault<10;++fault){restore();if(fault==0)++currentBinding.form;if(fault==1)++currentBinding.archive;if(fault==2)++currentBinding.lifetime;if(fault==3)++accepted;if(fault==4)currentPost[500]^=1;if(fault==5)currentBytes[16]^=1;if(fault==6)currentTarget.x[100]^=2;if(fault==7)currentSidebar.source[16]^=1;if(fault==8)++currentSidebar.lifetime;if(fault==9)currentSidebar.target.ownerState[3]^=4;assert(!join(first,first.dialog));}restore();assert(join(first,first.dialog));
  auto second=capture();assert(second.text!=first.text&&second.proof!=first.proof&&second.serial>first.serial&&first.dialog->current()&&second.dialog->current());accepted=second.serial;currentPost=second.post;assert(join(second,second.dialog));assert(!join(second,first.dialog));accepted=first.serial;currentPost=first.post;assert(join(first,first.dialog));assert(!join(first,second.dialog));assert(cap::acquireCurrentDialogProof(game)==second.proof);
  auto lease=gc::Lease::create(scene);assert(lease);retainedView=lease->view(190);assert(retainedView.lifetimeOwner&&retainedView.retainOwner&&retainedView.releaseOwner);retainedView.retainOwner(retainedView.lifetimeOwner);weakScene=scene;scene.reset();lease.reset();first={};second={};cache.reset();assert(!weakScene.expired());cap::beforeOwnerRetire(game,selected.owner);assert(!cap::lastDialogProof(game)&&!cap::acquireCurrentDialogProof(game));
 }
 assert(!cap::current(game)&&!cap::acquireCurrentDialogProof(game)&&!weakScene.expired());assert(retainedView.layers.dialogWords.count==16&&retainedView.layers.dialogWords.images[0].rgba&&retainedView.layers.text.font.alpha&&retainedView.original565[200*640]==0xffff);const auto&image=retainedView.layers.dialogWords.images[0];unsigned char sha[32];assert(nfs_texture_sha256(image.rgba,image.bytes,sha)&&!std::memcmp(sha,image.sha256,32));retainedView.releaseOwner(retainedView.lifetimeOwner);assert(weakScene.expired());assert(!leaseDepth&&allocationsInLease==0);
}
}
