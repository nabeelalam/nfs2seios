// SPDX-License-Identifier: MIT
#pragma once
#include "HdSceneFixture.h"
#include "LocationPreviewFixture.h"
#include "hd/LocationScenePrepare.h"
// Included after the private lexical runtime fixture: proofs are created only
// by the real Tail/String/Word observers, never by metadata construction.
static void genuineSceneCases(){
 namespace loc=nfs_location_scene_cpu;
 prepare();auto f=std::unique_ptr<Fixture>(new Fixture);f->base.form=f->scene.form=loc::LocationForm;f->scene.event.sequence=3;f->scene.tailKind=NFS_BACKGROUND_TAIL_LOCATION_CAPTION;f->scene.tailPredicate=f->scene.tailSymbol=loc::LocationForm;
 auto original=std::unique_ptr<nfs_location_preview_test::Fixture>(new nfs_location_preview_test::Fixture(f->base.post));auto preview=original->packet(f->base.post);assert(preview);f->copy.pre.pixels=original->input.post.pixels;f->copy.post.pixels=f->copy.pre.pixels;
 selected=f->copy.sourceOwner;std::memcpy(memory.data()+selected.w[8],selected.header.data(),16);std::memcpy(memory.data()+selected.w[8]+16,f->scene.tailPre.pixels.data(),cap::Pixels*2);put<uint32_t>(dword_4D40B8,0x20000);
 Game g;g.eax=off_4D2A20;std::shared_ptr<const loc::Prepared>retained;
 {cap::OwnerScope owner(g);auto*s=cap::current(g);assert(s);{cap::TailScope tail(g);stringBody(g,"proof-normal");}auto proof=cap::lastProof(g);auto packet=cap::lastPacket(g);assert(proof&&packet&&cap::acquireCurrentProof(g)==proof);
 std::copy(packet->original().begin(),packet->original().end(),f->scene.tailPost.pixels.begin());
 for(unsigned k=0;k<bg::Side;++k)put16(f->copy.sourceBefore.data()+16+2*k,packet->original()[k]);f->copy.sourceAfter=f->copy.sourceBefore;assert(nfs_texture_sha256(f->copy.sourceBefore.data(),f->copy.span,f->copy.shaBefore.data()));f->copy.shaAfter=f->copy.shaBefore;
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){auto value=packet->original()[y*343+x];put32(f->copy.row[y].source.data()+4*x,rgba(value));f->copy.post.pixels[y*640+x]=value;}
 nfs_location_caption_display::Binding binding{loc::LocationForm,off_4D2A20,0x20000,reinterpret_cast<uintptr_t>(&g),s->serial,s->lifetime};
 loc::JoinedInput input{{f->base,f->scene,f->group,f->copy,{1,4,1},{1,5,1},{1,6,1},{1,7,1},f->copy.post.target,1,1,f->keys},packet,s->serial,s->lifetime,preview,11,1,0x20000,{1,1,0},{1,2,0},proof,binding};
 auto out=std::make_shared<loc::Prepared>();auto verify=[&](){return loc::verifyAndPrepare(input,f->workspace,*out,f->catalog);};assert(verify()==bg::Failure::None&&out->captionWord&&out->captionWord->packet()==packet);auto view=out->view();assert(view.captionWord.word==2&&view.captionWord.rgba&&view.captionWord.left==116&&view.captionWord.right==168&&view.preview.schema==1);assert(out->original==f->copy.post.pixels);
 input.captionProof.reset();assert(verify()!=bg::Failure::None);input.captionProof=proof;
 input.caption.reset();assert(verify()!=bg::Failure::None);input.caption=packet;
 ++input.captionBinding.formGuest;assert(verify()!=bg::Failure::None);input.captionBinding=binding;
 ++input.captionBinding.archive;assert(verify()!=bg::Failure::None);input.captionBinding=binding;
 ++input.captionBinding.serial;assert(verify()!=bg::Failure::None);input.captionBinding=binding;
 input.preview.reset();assert(verify()!=bg::Failure::None);input.preview=preview;
 ++input.acceptedPreviewSerial;assert(verify()!=bg::Failure::None);input.acceptedPreviewSerial=11;
 f->copy.sourceBefore[16]^=1;assert(verify()!=bg::Failure::None);f->copy.sourceBefore[16]^=1;
 assert(verify()==bg::Failure::None);retained=out;
 cap::beforeOwnerRetire(g,selected.owner);assert(!cap::acquireCopiedProof(g,proof,packet,s->serial,s->lifetime,f->copy.sourceOwner,f->copy.sourceBefore.data(),f->copy.sourceBefore.size()));assert(retained->captionWord->current());
 }
 assert(retained&&retained->view().captionWord.word==2&&retained->preview==preview); // Historical ownership survives; live access does not.
}
