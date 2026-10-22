// SPDX-License-Identifier: MIT
#pragma once
#include "hd/PlayerInitialNativeUploadWitness.h"
#include <functional>
// Called with a source sealed by the actual resource/measure/String scopes and
// the full Main verifier above, never a fabricated proof constructor.
static void initialNativeUploadCases(const std::shared_ptr<const nfs_player_initial_sidebar::Proof>&source){
 namespace native=nfs_player_initial_native;
 static_assert(!std::is_default_constructible<native::Proof>::value,"upload proof must be sealed");
 static_assert(!std::is_copy_constructible<native::Proof>::value,"cannot copy revocation authority");
 assert(source&&source->current());native::Binding binding;binding.sidebar=source->binding();binding.sourceOwner=source->post().target.owner;binding.sourceBitmap=source->post().target.w[8];auto catalog=source->origin()->catalog();
 auto actual=std::unique_ptr<bg::Plane>(new bg::Plane(source->post()));auto stage=std::unique_ptr<bg::Plane>(new bg::Plane(source->origin()->base().post));
 auto&t=stage->target;t.owner=0x1000;t.w[8]=0x100000;t.w[12]=0x8000;t.w[11]=0x9000;t.ownerWords=t.w;stage->pixels.fill(0);
 for(unsigned y=0;y<480;++y)std::copy_n(source->post().pixels.begin()+y*343,343,stage->pixels.begin()+y*640);
 assert(native::padded(*source,*stage));
 auto tile=[&](unsigned n){native::TileObservation v;v.descriptorGuest=v.expectedSlotDescriptor=0x20000000+64*n;v.payloadGuest=0xc0000000;v.before={{3,3,3,11,0,0x01000000+2048*n,3}};v.after=v.before;v.after[4]=v.payloadGuest;v.originalUploadReturned=true;
  const unsigned x=n/15*32,y=n%15*32;for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx){uint16_t p=stage->pixels[(y+yy)*640+x+xx];uint16_t rgb=uint16_t((p/2048)*1024+((p/64)%32)*32+p%32);uint16_t expected=rgb?uint16_t(rgb+32768):0;const unsigned at=2*(yy*32+xx);v.pixels[at]=uint8_t(expected);v.pixels[at+1]=uint8_t(expected/256);}return v;};
 native::Witness witness;
 auto begin=[&](){auto event=native::overwriteEvent();assert(event);assert(witness.begin(source,binding,*actual,*stage,event,catalog));return event;};
 auto complete=[&](){auto event=begin();for(unsigned n=0;n<300;++n)assert(witness.tile(tile(n),binding,catalog));assert(witness.uploaded()==300);assert(witness.finish(*actual,*stage,true,binding,catalog));auto p=witness.proofForUse(*actual,binding,catalog);assert(p&&p->current(binding,catalog)&&p->uploadEvent(binding,catalog)==event&&p->source(binding,catalog)==source);return p;};
 auto proof=complete();std::array<uint32_t,300>keys{};assert(proof->keys(binding,catalog,keys));for(unsigned n=0;n<300;++n)assert(keys[n]==0x01000000+2048*n);
 // Actual upload staging is retired before its caller returns. Completed proof
 // owns its pixels and must survive expected cleanup and caller buffer reuse.
 auto ownedStage=proof->stage(binding,catalog);assert(ownedStage&&ownedStage->pixels==stage->pixels);witness.retire(stage->target.owner);witness.retire(stage->target.w[8]);assert(proof->current(binding,catalog));stage->pixels.back()^=1;assert(ownedStage->pixels.back()!=stage->pixels.back());stage->pixels.back()^=1;
 native::overwriteEvent();assert(!proof->current(binding,catalog)&&!proof->keys(binding,catalog,keys));for(auto key:keys)assert(!key);assert(!proof->stage(binding,catalog)&&!proof->source(binding,catalog));
 // Every current domain is independently bound, including the Player epoch
 // rather than the retained Main or policy-local caption lifetime.
 for(unsigned k=0;k<10;++k){proof=complete();auto b=binding;if(k==0)++b.sidebar.form;if(k==1)++b.sidebar.formGuest;if(k==2)++b.sidebar.archive;if(k==3)++b.sidebar.game;if(k==4)++b.sidebar.thread;if(k==5)++b.sidebar.epoch;if(k==6)++b.sidebar.captionEvent.sequence;if(k==7)++b.sourceOwner;if(k==8)++b.sourceBitmap;if(k==9)++b.sidebar.captionEvent.iteration;assert(!witness.proofForUse(*actual,b,catalog)&&!proof->current(binding,catalog));}
 for(unsigned k=0;k<9;++k){auto bad=std::unique_ptr<bg::Plane>(new bg::Plane(*stage));if(k==0)bad->pixels[343]^=1;if(k==1)bad->pixels[bg::Full-1]^=1;if(k==2)bad->pixels[479*640+342]^=1;if(k==3)bad->target.header[3]=1;if(k==4)bad->target.w[3]=1;if(k==5)bad->target.x[639]^=2;if(k==6)bad->target.y[479]^=2;if(k==7)bad->target.w[8]=binding.sourceBitmap;if(k==8)bad->target.w[8]=UINT32_MAX-1024;assert(!witness.begin(source,binding,*actual,*bad,native::overwriteEvent(),catalog));}
 for(unsigned k=0;k<5;++k){auto bad=std::unique_ptr<bg::Plane>(new bg::Plane(*actual));if(k==0)bad->pixels[bg::Side-1]^=1;if(k==1)bad->target.header[3]=1;if(k==2)bad->target.x[342]^=2;if(k==3)bad->target.y[479]^=2;if(k==4)++bad->target.owner;assert(!witness.begin(source,binding,*bad,*stage,native::overwriteEvent(),catalog));}
 std::vector<std::function<void(native::TileObservation&)>>mutations={
  [](native::TileObservation&v){v.originalUploadReturned=false;},[](native::TileObservation&v){++v.expectedSlotDescriptor;},[](native::TileObservation&v){v.tableGuest=1;},[](native::TileObservation&v){v.before[3]=v.after[3]=12;},[](native::TileObservation&v){++v.after[5];},[](native::TileObservation&v){v.after[4]=0;},[](native::TileObservation&v){v.payloadGuest=UINT32_MAX-1024;v.after[4]=v.payloadGuest;},[](native::TileObservation&v){v.descriptorGuest=v.expectedSlotDescriptor=UINT32_MAX-16;},[](native::TileObservation&v){v.before[5]=v.after[5]=UINT32_MAX-1024;},[&](native::TileObservation&v){v.descriptorGuest=v.expectedSlotDescriptor=binding.sourceBitmap;},[&](native::TileObservation&v){v.payloadGuest=stage->target.w[8];v.after[4]=v.payloadGuest;}};
 for(auto mutate:mutations){begin();auto v=tile(0);mutate(v);assert(!witness.tile(v,binding,catalog));}
 for(unsigned at:{0u,14u,15u,149u,299u}){begin();for(unsigned n=0;n<at;++n)assert(witness.tile(tile(n),binding,catalog));auto v=tile(at);v.pixels.back()^=1;assert(!witness.tile(v,binding,catalog));}
 for(unsigned k=0;k<3;++k){begin();assert(witness.tile(tile(0),binding,catalog));auto v=tile(1);if(k==0)v.descriptorGuest=v.expectedSlotDescriptor=tile(0).descriptorGuest;if(k==1)v.before[5]=v.after[5]=tile(0).after[5]+1024;if(k==2){v.descriptorGuest=v.expectedSlotDescriptor=tile(0).payloadGuest;}assert(!witness.tile(v,binding,catalog));}
 begin();assert(!witness.finish(*actual,*stage,true,binding,catalog));begin();native::overwriteEvent();assert(!witness.tile(tile(0),binding,catalog));begin();witness.retire(stage->target.owner);assert(!witness.tile(tile(0),binding,catalog));
 for(unsigned k=0;k<4;++k){begin();for(unsigned n=0;n<300;++n)assert(witness.tile(tile(n),binding,catalog));auto after=std::unique_ptr<bg::Plane>(new bg::Plane(*stage));auto src=std::unique_ptr<bg::Plane>(new bg::Plane(*actual));if(k==0)after->pixels.back()^=1;if(k==1)src->pixels[bg::Side-1]^=1;assert(!witness.finish(*src,*after,k!=2,binding,catalog,k==3));}
 for(unsigned k=0;k<4;++k){proof=complete();if(k==0)witness.reset();if(k==1)witness.ownerExit();if(k==2)witness.slot0Overwrite();if(k==3)witness.retire(binding.sourceOwner);assert(!proof->current(binding,catalog));}
 proof=complete();auto changed=std::make_shared<bg::SourceCatalog>(*catalog);changed->mapSha[0][0]^=1;assert(!witness.proofForUse(*actual,binding,changed)&&!proof->current(binding,catalog));
 proof=complete();auto changedSource=std::unique_ptr<bg::Plane>(new bg::Plane(*actual));changedSource->pixels[bg::Side-1]^=1;assert(!witness.proofForUse(*changedSource,binding,catalog)&&!proof->current(binding,catalog));
 std::puts("PASS Initial sidebar independent slot0 upload: genuine source, full padded stage, 300 tiles, corruption/alias/overwrite/lifetime guards; no GPU or live-hook claim");
}
