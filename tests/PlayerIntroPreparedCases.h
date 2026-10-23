// SPDX-License-Identifier: MIT
#pragma once
#include "hd/PlayerIntroPrepare.h"
// Genuine source is supplied by the actual Initial caption fixture. Native
// proofs below are sealed only by the complete real CPU upload Witness.
struct PlayerIntroFixture {
 std::shared_ptr<const nfs_player_initial_sidebar::Proof>source;
 std::shared_ptr<const bg::SourceCatalog>catalog;
 nfs_player_initial_native::Binding binding{};
 std::unique_ptr<bg::Plane>actual,stage;
 explicit PlayerIntroFixture(std::shared_ptr<const nfs_player_initial_sidebar::Proof>s):source(std::move(s)),catalog(source->origin()->catalog()),actual(new bg::Plane(source->post())),stage(new bg::Plane(source->origin()->base().post)){
  binding.sidebar=source->binding();binding.sourceOwner=source->post().target.owner;binding.sourceBitmap=source->post().target.w[8];auto&t=stage->target;t.owner=0xb000;t.w[8]=0x100000;t.w[12]=0x8000;t.w[11]=0x9000;t.ownerWords=t.w;t.ownerState=actual->target.ownerState;stage->pixels.fill(0);for(unsigned y=0;y<480;++y)std::copy_n(actual->pixels.begin()+y*343,343,stage->pixels.begin()+y*640);assert(nfs_player_initial_native::padded(*source,*stage));
 }
 nfs_player_initial_native::TileObservation tile(unsigned n)const{
  nfs_player_initial_native::TileObservation t;t.descriptorGuest=t.expectedSlotDescriptor=0x1a0000+64*n;t.payloadGuest=0x1b0000;t.before={{3,3,3,11,0,0x01000000+2048*n,3}};t.after=t.before;t.after[4]=t.payloadGuest;t.originalUploadReturned=true;for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx){uint16_t p=stage->pixels[((n%15)*32+yy)*640+(n/15)*32+xx];const uint16_t rgb=uint16_t((p/2048)*1024+((p/64)%32)*32+p%32),v=rgb?uint16_t(rgb+32768):0;const unsigned at=2*(yy*32+xx);t.pixels[at]=uint8_t(v);t.pixels[at+1]=uint8_t(v>>8);}return t;
 }
 std::shared_ptr<const nfs_player_initial_native::Proof>seal(nfs_player_initial_native::Witness&w)const{
  const auto event=nfs_player_initial_native::overwriteEvent();assert(w.begin(source,binding,*actual,*stage,event,catalog));for(unsigned n=0;n<300;++n)assert(w.tile(tile(n),binding,catalog));assert(w.finish(*actual,*stage,true,binding,catalog));auto proof=w.proofForUse(*actual,binding,catalog);assert(proof&&proof->current(binding,catalog));return proof;
 }
};
static void playerIntroPreparedCases(const std::shared_ptr<const nfs_player_initial_sidebar::Proof>&source){
 namespace native=nfs_player_initial_native;namespace intro=nfs_player_intro_cpu;static_assert(!std::is_default_constructible<intro::Prepared>::value,"layout requires a genuine native proof");
 PlayerIntroFixture f(source);native::Witness witness;auto proof=f.seal(witness);auto prepared=intro::Prepared::create(proof,f.binding,*f.actual,f.catalog,701);assert(prepared&&prepared->current());auto v=prepared->view();const auto main=source->origin()->frame()->view();assert(v.schema==1&&v.form==source->binding().form&&v.requestSerial==701&&v.uploadEvent==proof->uploadEvent(f.binding,f.catalog));assert(v.inheritedMain.serial==main.serial&&v.inheritedMain.contentRevision==main.contentRevision&&v.inheritedMain.tailKind==NFS_BACKGROUND_TAIL_MAIN_SKIP);assert(v.inheritedMain.original565==main.original565&&v.inheritedMain.wordCount==main.wordCount);assert(v.sourceOwner==f.binding.sourceOwner&&v.sourceBitmap==f.binding.sourceBitmap&&v.wordsCount==bg::Full);assert(v.caption.logicalWidth==343&&v.caption.logicalHeight==480&&v.caption.vertices==source->caption()->vertices().data()&&v.caption.vertexCount==source->caption()->vertices().size());assert(v.caption.font.alpha==source->caption()->font()->alpha().data());for(unsigned n=0;n<300;++n)assert(v.keys[n]==0x01000000+2048*n);for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)assert(v.paddedOriginal565[y*640+x]==(x<343?source->post().pixels[y*343+x]:0));
 // Prepared borrows only owned immutable witness/source memory. Reuse of the
 // caller's temporary stage cannot affect a later renderer callback.
 f.stage->pixels[343]^=1;assert(prepared->view().paddedOriginal565[343]==0);f.stage->pixels[343]^=1;assert(!intro::Prepared::create({},f.binding,*f.actual,f.catalog,702));assert(!intro::Prepared::create(proof,f.binding,*f.actual,f.catalog,0));auto wrong=std::unique_ptr<bg::Plane>(new bg::Plane(*f.actual));wrong->pixels[bg::Side-1]^=1;assert(!intro::Prepared::create(proof,f.binding,*wrong,f.catalog,703));assert(prepared->current());
 native::overwriteEvent();assert(!prepared->current()&&!prepared->view().schema);proof=f.seal(witness);prepared=intro::Prepared::create(proof,f.binding,*f.actual,f.catalog,704);assert(prepared);auto bad=f.binding;++bad.sidebar.epoch;assert(!intro::Prepared::create(proof,bad,*f.actual,f.catalog,705)&&!prepared->current());proof=f.seal(witness);prepared=intro::Prepared::create(proof,f.binding,*f.actual,f.catalog,706);assert(prepared);witness.ownerExit();assert(!prepared->current()&&!prepared->view().schema);assert(source->origin()->frame()->view().serial==main.serial);assert(!leaseDepth&&allocationsInLease==0);std::puts("PASS Player Intro owned preparation: genuine source/native seal, historical Main identity unchanged, full stage/key/font ownership, current-event/lifetime rejection; no GPU claim");
}
