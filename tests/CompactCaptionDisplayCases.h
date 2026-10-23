// SPDX-License-Identifier: MIT
#pragma once
#include "hd/CompactCaptionDisplay.h"
#include "hd/SetupScenePrepare.h"
#include "hd/OptionsScenePrepare.h"
// Included only by the genuine lexical capture fixture, after trackedString.
static void compactCaptionDisplayAndJoinCases(){
 if(testSelector!=0&&testSelector!=6)return;
 namespace display=nfs_compact_caption_display;
#if defined(NFS_TEST_SETUP_CAPTION)
 namespace cached=nfs_setup_scene_cpu;
 const uint32_t form=cached::SetupForm;
#else
 namespace cached=nfs_options_scene_cpu;
 const uint32_t form=cached::OptionsForm;
#endif
 static_assert(!std::is_default_constructible<display::Prepared>::value,"word authority requires a genuine sealed source proof");
 prepare();auto f=std::unique_ptr<main343_origin_fixture::Fixture>(new main343_origin_fixture::Fixture);
 f->base.form=f->scene.form=form;
 f->scene.tailKind=form==0x4d2790?NFS_BACKGROUND_TAIL_SETUP_CAPTION:NFS_BACKGROUND_TAIL_OPTIONS_CAPTION;
 f->scene.tailPredicate=f->scene.tailSymbol=form;
 selected=f->group.post.target;
 for(unsigned k=0;k<4;++k)put<uint32_t>(selected.w[8]+4*k,selected.header[k]);
 std::memcpy(memory.data()+selected.w[8]+16,f->group.post.pixels.data(),bg::Side*2);
 Game g;g.eax=testOwner;g.ebp=testOwner;
 std::shared_ptr<const display::Prepared>escaped;
 std::shared_ptr<cached::Prepared>queued;
 {cap::OwnerScope owner(g);auto*current=cap::current(g);assert(current);
  {cap::TailScope tail(g);trackedString(g,"normal");}
  auto proof=cap::acquireCurrentProof(g);assert(proof);
  auto packet=proof->packet();const auto original=packet->original(),clean=packet->clean();
  const auto font=packet->font();const auto vertices=packet->vertices().data();
  display::Binding binding{form,testOwner,0x1234,reinterpret_cast<uintptr_t>(&g),current->serial,current->lifetime};
  auto prepared=display::Prepared::create(proof,packet,binding);assert(prepared&&prepared->current());
  const auto view=prepared->view();const bool setup=testSelector==0;
  assert(view.schema==NFS_COMPACT_CAPTION_WORD_SCHEMA&&view.word==(setup?1u:3u)&&view.width==(setup?300u:192u)&&view.height==128&&view.bytes==size_t(view.width)*128*4&&view.rgba);
  assert(view.left==(setup?93:120)&&view.top==(setup?126:364)&&view.right==168&&view.bottom==view.top+32);
  unsigned char digest[32];assert(nfs_texture_sha256(view.rgba,view.bytes,digest)&&!std::memcmp(digest,view.sha256,32)&&!std::memcmp(digest,nfs_compact_caption_word_sha256[view.word-1],32));
  for(const char*mode:{"form","formGuest","game","archive","serial","lifetime","packet","proof"}){
   auto b=binding;auto p=packet;auto q=proof;
   if(!std::strcmp(mode,"form"))b.form=setup?0x4d3a88:0x4d2790;
   if(!std::strcmp(mode,"formGuest"))++b.formGuest;
   if(!std::strcmp(mode,"game"))++b.game;
   if(!std::strcmp(mode,"archive"))++b.archive;
   if(!std::strcmp(mode,"serial"))++b.serial;
   if(!std::strcmp(mode,"lifetime"))++b.lifetime;
   if(!std::strcmp(mode,"packet"))p.reset();
   if(!std::strcmp(mode,"proof"))q.reset();
   assert(!display::Prepared::create(q,p,b));
  }
  // A complete independent original side/group/copy reconstruction is still
  // mandatory. Display authority alone supplies no full scene authority.
  std::copy(packet->original().begin(),packet->original().end(),f->scene.tailPost.pixels.begin());
  f->copy.pre.pixels=f->base.post.pixels;f->copy.post.pixels=f->copy.pre.pixels;
  for(unsigned k=0;k<bg::Side;++k)put16(f->copy.sourceBefore.data()+16+2*k,f->scene.tailPost.pixels[k]);
  f->copy.sourceAfter=f->copy.sourceBefore;
  assert(nfs_texture_sha256(f->copy.sourceBefore.data(),f->copy.span,f->copy.shaBefore.data()));f->copy.shaAfter=f->copy.shaBefore;
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){const uint16_t value=f->scene.tailPost.pixels[y*343+x];put32(f->copy.row[y].source.data()+4*x,value?rgba(value):0);f->copy.post.pixels[y*640+x]=value?value:f->base.post.pixels[y*640+x];}
  cached::JoinedInput input{f->input(),packet,current->serial,current->lifetime,proof,binding};
  queued=std::make_shared<cached::Prepared>();
  auto verify=[&](){return cached::verifyAndPrepare(input,f->workspace,*queued,f->catalog);};
  assert(verify()==bg::Failure::None&&queued->valid&&queued->captionWord&&queued->captionWord->current());
  const auto sceneView=queued->view();assert(sceneView.captionWord.word==view.word&&sceneView.captionWord.rgba==view.rgba&&sceneView.caption.font.alpha==font->alpha().data());
  for(const char*mode:{"form","formGuest","archive","game","serial","lifetime","proof"}){
   input.captionBinding=binding;input.captionProof=proof;
   if(!std::strcmp(mode,"form"))++input.captionBinding.form;
   if(!std::strcmp(mode,"formGuest"))++input.captionBinding.formGuest;
   if(!std::strcmp(mode,"archive"))++input.captionBinding.archive;
   if(!std::strcmp(mode,"game"))++input.captionBinding.game;
   if(!std::strcmp(mode,"serial"))++input.captionBinding.serial;
   if(!std::strcmp(mode,"lifetime"))++input.captionBinding.lifetime;
   if(!std::strcmp(mode,"proof"))input.captionProof.reset();
   assert(verify()!=bg::Failure::None&&!queued->valid&&queued->view().schema==0);
  }
  input.captionBinding={};input.captionProof.reset();
  assert(verify()==bg::Failure::None&&!queued->captionWord&&queued->view().captionWord.schema==0);
  input.captionBinding=binding;input.captionProof=proof;
  assert(verify()==bg::Failure::None&&queued->captionWord);
  f->copy.post.pixels[639]^=1;assert(verify()!=bg::Failure::None&&!queued->valid);f->copy.post.pixels[639]^=1;
  assert(verify()==bg::Failure::None&&queued->original==f->copy.post.pixels);
  assert(packet->original()==original&&packet->clean()==clean&&packet->font()==font&&packet->vertices().data()==vertices);
  escaped=prepared;prepared.reset();assert(escaped->view().rgba==view.rgba);
 }
 f.reset();assert(escaped&&escaped->current()&&escaped->view().rgba&&queued&&queued->captionWord&&queued->view().captionWord.schema==NFS_COMPACT_CAPTION_WORD_SCHEMA);
}
