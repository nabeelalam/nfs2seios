// SPDX-License-Identifier: MIT
#pragma once
#include "AudioSidebarFixture.h"
#include "AudioPacketFixture.h"
#include "hd/AudioScenePrepare.h"
#include "hd/AudioSceneLease.h"
// Included after the genuine lexical Audio mock and AudioDialogWordCases.
// The dialog proof is never constructed from caller metadata. Background/bar
// sources still enter their complete original replay verifiers unchanged.
namespace audio_dialog_word_scene_cases {
namespace af=nfs_audio_frame;namespace ac=nfs_audio_scene_cpu;
struct Capture {
 std::shared_ptr<const nfs_soft::FramePacket>text;
 std::shared_ptr<const nfs_audio_dialog_word::Proof>proof;
 std::shared_ptr<const nfs_audio_dialog_display::Prepared>dialog;
 std::shared_ptr<const af::AudioFramePacket>frame;
 std::vector<uint16_t>post;uint64_t serial=0;
};
inline void run(){
 prepare();audio_dialog_word_cases::resources();
 auto a=std::unique_ptr<nfs_audio_test::Fixture>(new nfs_audio_test::Fixture);
 auto sidebar=std::unique_ptr<SidebarFixture>(new SidebarFixture);
 Game g;g.eax=off_4D3348;
 NfsAudioSceneFrameView retainedView{};std::weak_ptr<const ac::Prepared>weakScene;
 {cap::OwnerScope owner(g);auto*source=cap::current(g);assert(source);
  a->binding.form=off_4D3348;a->binding.archive=0x1234;a->binding.lifetime=source->lifetime;a->binding.surface.lifetime=source->lifetime;assert(a->binding.surface.valid()&&a->binding.surface.geometry[16]==a->full.x[639]);a->background.binding=a->binding;
  sidebar->binding.form=off_4D3348;sidebar->binding.archive=0x1234;sidebar->binding.ownerEpoch=a->binding.ownerEpoch;sidebar->f->scene.form=off_4D3348;sidebar->catalog->mapSha[0]=a->mapSha;
  auto cache=sidebar->verify();assert(cache&&!cache->captionWord());
  auto&draw=a->background.sidebar;draw.args[0]=sidebar->binding.bitmap;draw.sidebarHandle=draw.sidebarTableEntry=sidebar->binding.sourceOwner;draw.sidebarBitmap=sidebar->binding.bitmap;draw.sourceBefore=draw.sourceAfter=sidebar->before;
  a->background.preBars=a->background.postReset;
  for(unsigned y=0;y<480;++y){auto&r=draw.rows[y];r.args[0]=sidebar->binding.bitmap+16+y*686;r.source.assign(sidebar->before.begin()+16+y*686,sidebar->before.begin()+16+(y+1)*686);for(unsigned x=0;x<343;++x){const auto value=logo::le16(r.source.data()+2*x);r.after[x]=value?value:r.before[x];a->background.preBars[y*640+x]=r.after[x];}}
  a->buildBars({{70,12,140}},1,0);
  // Preserve independent original verifiers; the fixture's one-command text
  // owner is deliberately discarded and never used for the dialog proof.
  a->text.reset();a->postText.clear();
  auto background=af::BackgroundReplayVerifier::verify(a->background,a->binding,a->mapCatalog);assert(background);
  auto bars=af::BarReplayVerifier::verify(a->bars,a->binding,a->sourceCatalog,a->art);assert(bars&&background->pixels()==bars->pre());
  auto capture=[&](){
   Capture result;selected=a->full;std::memcpy(memory.data()+selected.w[8],selected.header.data(),16);std::memcpy(memory.data()+selected.w[8]+16,bars->post().data(),af::Pixels*2);
   g.eax=off_4D3348;{cap::TailScope tail(g);audio_dialog_word_cases::fullTail(g,"normal",0x1e0000);}
   result.text=cap::lastPacket(g);result.proof=cap::lastDialogProof(g);assert(result.text&&result.proof&&result.proof->packet()==result.text&&cap::acquireCurrentDialogProof(g)==result.proof&&result.text->clean()==bars->post());result.serial=result.text->serial();result.post=result.text->original();
   result.dialog=nfs_audio_dialog_display::Prepared::create(result.proof,result.text,{a->binding.form,a->binding.archive,reinterpret_cast<uintptr_t>(&g),result.serial,source->lifetime});assert(result.dialog);
   result.frame=af::AudioFramePacket::create(background,bars,result.text,a->binding,result.serial,result.post.data(),result.post.size());assert(result.frame&&result.frame->text()==result.text);return result;
  };
  auto first=capture();auto current=a->binding;auto currentCache=sidebar->binding;auto currentTarget=sidebar->f->scene.preSide.target;auto currentBytes=sidebar->before;uint64_t accepted=first.serial;auto currentPost=first.post;
  auto join=[&](const Capture&candidate,const std::shared_ptr<const nfs_audio_dialog_display::Prepared>&dialog){return ac::Prepared::create(candidate.frame,cache,current,accepted,currentPost.data(),currentPost.size(),currentCache,currentTarget,currentBytes.data(),currentBytes.size(),dialog);};
  auto scene=join(first,first.dialog);assert(scene&&scene->frame()->text()==first.text);auto view=scene->view(180);assert(view.schema==NFS_AUDIO_SCENE_RUNTIME_SCHEMA&&view.layers.dialogWords.schema==NFS_AUDIO_DIALOG_WORD_SCHEMA&&view.layers.dialogWords.count==22&&view.original565==first.text->original().data()&&view.layers.text.vertices==first.text->vertices().data()&&view.layers.barVertices==bars->layout().vertexCount);
  for(unsigned i=0;i<11;++i)for(unsigned j=0;j<2;++j){const auto&r=first.text->records()[nfs_audio_dialog_word::record(i)+j];const auto&p=view.layers.dialogWords.placements[2*i+j];assert(p.vertexFirst==r.vertexFirst&&p.vertexCount==r.vertexCount&&p.color565==r.command.color&&p.left==r.command.x&&p.top==r.command.y);}
  for(unsigned fault=0;fault<7;++fault){current=a->binding;accepted=first.serial;currentCache=sidebar->binding;currentTarget=sidebar->f->scene.preSide.target;currentBytes=sidebar->before;currentPost=first.post;if(fault==0)++current.form;if(fault==1)++current.archive;if(fault==2)++current.lifetime;if(fault==3)++accepted;if(fault==4)currentPost[500]^=1;if(fault==5)currentBytes[16]^=1;if(fault==6)currentTarget.x[100]^=2;assert(!join(first,first.dialog));}
  current=a->binding;accepted=first.serial;currentCache=sidebar->binding;currentTarget=sidebar->f->scene.preSide.target;currentBytes=sidebar->before;currentPost=first.post;assert(join(first,first.dialog));
  // Both candidate generations independently pass their original full-frame
  // verifier and scene join. Only mismatched immutable text owners decline.
  auto second=capture();assert(second.text!=first.text&&second.proof!=first.proof&&second.serial>first.serial&&second.dialog->current()&&first.dialog->current());
  accepted=second.serial;currentPost=second.post;assert(join(second,second.dialog));assert(!join(second,first.dialog));
  accepted=first.serial;currentPost=first.post;assert(join(first,first.dialog));assert(!join(first,second.dialog));assert(cap::acquireCurrentDialogProof(g)==second.proof);
  auto lease=ac::Lease::create(scene);assert(lease);retainedView=lease->view(190);assert(retainedView.lifetimeOwner&&retainedView.retainOwner&&retainedView.releaseOwner);retainedView.retainOwner(retainedView.lifetimeOwner);weakScene=scene;scene.reset();lease.reset();first={};second={};cache.reset();assert(!weakScene.expired());
  cap::beforeOwnerRetire(g,selected.owner);assert(!cap::acquireCurrentDialogProof(g)&&!cap::lastDialogProof(g));
 }
 assert(!cap::current(g)&&!cap::acquireCurrentDialogProof(g)&&!weakScene.expired());assert(retainedView.layers.dialogWords.count==22&&retainedView.layers.dialogWords.images[0].rgba&&retainedView.layers.text.font.alpha&&retainedView.original565[200*640]==0xffff);unsigned char sha[32];const auto&image=retainedView.layers.dialogWords.images[0];assert(nfs_texture_sha256(image.rgba,image.bytes,sha)&&!std::memcmp(sha,image.sha256,32));retainedView.releaseOwner(retainedView.lifetimeOwner);assert(weakScene.expired());assert(!leaseDepth&&allocationsInLease==0);
}
}
