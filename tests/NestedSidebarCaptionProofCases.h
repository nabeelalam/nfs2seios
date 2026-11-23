// SPDX-License-Identifier: MIT
#pragma once
#include "HdSceneFixture.h"
#include "hd/AudioSidebarPacket.h"
#include "hd/OptionsSidebarOrigin.h"
#include "hd/OptionsBitmapPrelude.h"
#include <type_traits>
// Included after the real source-hook fixture; private seals are produced only
// by its actual Owner/Tail/Measure/String/Word observers.
static void nestedProofCases(){
 namespace nested=nfs_nested_sidebar_caption;namespace sidebar=nfs_audio_sidebar;
 static_assert(!std::is_default_constructible<nested::Proof>::value,"no public proof constructor");
 static_assert(!std::is_constructible<nested::Proof,std::shared_ptr<const nfs_soft::FramePacket>,const nested::Evidence&>::value,"only lexical tail can seal");
 prepare();auto f=std::unique_ptr<Fixture>(new Fixture);f->scene.form=NFS_NESTED_TEST_FORM;selected=f->copy.sourceOwner;
 std::memcpy(memory.data()+selected.w[8],selected.header.data(),16);std::memcpy(memory.data()+selected.w[8]+16,f->group.post.pixels.data(),cap::Pixels*2);
 Game g;g.eax=g.ebp=NFS_NESTED_TEST_FORM;
 std::shared_ptr<const sidebar::Packet>retained;
 {cap::OwnerScope owner(g);auto*s=cap::current(g);assert(s);{cap::TailScope tail(g);stringBody(g,"normal");}
 auto proof=cap::lastProof(g);auto packet=cap::lastPacket(g);assert(proof&&packet&&proof->packet()==packet);
 const auto&e=proof->evidence();assert(e.policy==NFS_NESTED_TEST_POLICY&&e.form==NFS_NESTED_TEST_FORM&&e.incomingEbp==e.form&&e.game==reinterpret_cast<uintptr_t>(&g)&&e.thread==cap::token());assert(e.measureWidth==48&&e.measureAnchor==168&&e.measureKind==2&&nested::exactCommand(e.command));
 nested::Binding binding{NFS_NESTED_TEST_POLICY,NFS_NESTED_TEST_FORM,0x1234,reinterpret_cast<uintptr_t>(&g),s->serial,s->lifetime};
 auto display=nested::Display::create(proof,packet,binding);assert(display&&display->current()&&display->packet()==packet);auto view=display->view();assert(view.word==3&&view.width==192&&view.height==128&&view.left==120&&view.top==364&&view.right==168&&view.bottom==396&&view.bytes==192*128*4&&!std::memcmp(view.sha256,nfs_compact_caption_word_sha256[2],32));
 for(auto policy:{nested::Policy::None,nested::Policy::Audio,nested::Policy::Graphics,nested::Policy::Controls,static_cast<nested::Policy>(4)})if(policy!=binding.policy){auto bad=binding;bad.policy=policy;assert(!nested::Display::create(proof,packet,bad));}
 for(unsigned k=0;k<6;++k){auto bad=binding;if(k==0)bad.policy=binding.policy==nested::Policy::Audio?nested::Policy::Graphics:nested::Policy::Audio;if(k==1)++bad.form;if(k==2)++bad.archive;if(k==3)++bad.game;if(k==4)++bad.serial;if(k==5)++bad.lifetime;assert(!nested::Display::create(proof,packet,bad));}
 std::vector<uint8_t>raw(sidebar::Bytes);std::memcpy(raw.data(),selected.header.data(),16);std::memcpy(raw.data()+16,packet->original().data(),cap::Pixels*2);auto observed=selected;
 auto acquire=[&](){return cap::acquireRetainedProof(g,proof,packet,s->serial,s->lifetime,observed,raw.data(),raw.size());};assert(acquire()==proof);
 const auto actual=selected;selected.w[1]=640;selected.w[10]=1280;observed.ownerState[0]^=1;assert(acquire()==proof);selected=actual;
 observed.owner+=4;assert(!acquire());observed=actual;observed.x[100]^=2;assert(!acquire());observed=actual;observed.y[250]^=2;assert(!acquire());observed=actual;observed.ownerState[1]^=1;assert(!acquire());observed=actual;
 for(size_t at:{size_t(0),size_t(15),size_t(16),raw.size()-1}){raw[at]^=1;assert(!acquire());raw[at]^=1;}
 put<uint32_t>(dword_4D40B8,0x1235);assert(!acquire());put<uint32_t>(dword_4D40B8,0x1234);
 put<uint32_t>(byte_4D3ECC,7u<<16);assert(!acquire());put<uint32_t>(byte_4D3ECC,6u<<16);
 put<uint32_t>(dword_4D3E9C,0x70003);assert(!acquire());put<uint32_t>(dword_4D3E9C,0x70002);
 put<uint32_t>(off_4D3ED0+24,0x2e0);put<uint32_t>(0x2e0,0x70002);assert(!acquire());put<uint32_t>(off_4D3ED0+24,dword_4D3E9C);
 memory[e.textGuest]^=1;assert(!acquire());memory[e.textGuest]^=1;feature=false;assert(!acquire());feature=true;assert(acquire()==proof);
 Game other=g;assert(!cap::acquireRetainedProof(other,proof,packet,s->serial,s->lifetime,observed,raw.data(),raw.size()));
 std::thread foreign([&]{foreignPath=true;assert(!acquire());foreignPath=false;});foreign.join();assert(foreignAllocations==0);
 assert(!cap::acquireRetainedProof(g,proof,packet,s->serial+1,s->lifetime,observed,raw.data(),raw.size()));assert(!cap::acquireRetainedProof(g,proof,packet,s->serial,s->lifetime,observed,raw.data(),raw.size()-1));
 sidebar::Binding cacheBinding{NFS_NESTED_TEST_FORM,selected.owner,selected.w[8],0x1234,1,1,f->group.thread,1};
 sidebar::Input input{cacheBinding,f->scene,f->group,packet,s->serial,s->lifetime,raw,raw,proof,binding};
 auto join=[&](){return sidebar::Packet::create(input,cacheBinding,actual,std::make_shared<const sidebar::Catalog>(f->catalog));};
 retained=join();assert(retained&&retained->captionWord()&&retained->captionWord()->view().word==3&&retained->caption()==packet&&retained->raw()==raw);
 // The future Options owner may retain immutable source content, but cannot
 // substitute a foreign game/thread/archive/epoch/catalog or changed bytes.
 auto historical=nfs_options_sidebar_origin::Origin::createNested(reinterpret_cast<uintptr_t>(&g),cacheBinding.thread,cacheBinding.archive,cacheBinding.ownerEpoch,retained,retained->catalog());assert(historical&&historical->current()&&historical->frame->nestedCaptionWord&&!historical->frame->captionWord&&historical->frame->wordView().word==3&&historical->raw==raw);
 for(unsigned k=0;k<5;++k){auto game=reinterpret_cast<uintptr_t>(&g),thread=uintptr_t(cacheBinding.thread);auto archive=cacheBinding.archive;auto epoch=cacheBinding.ownerEpoch;auto catalog=retained->catalog();if(k==0)++game;if(k==1)++thread;if(k==2)++archive;if(k==3)++epoch;if(k==4){auto changed=std::make_shared<sidebar::Catalog>(*catalog);changed->identities[0].sha[0]^=1;catalog=changed;}assert(!nfs_options_sidebar_origin::Origin::createNested(game,thread,archive,epoch,retained,catalog));}
 // Independent bitmap source admits only the complete original transparent copy.
 namespace bitmap=nfs_options_bitmap_cpu;auto whole=std::unique_ptr<nfs_background_scene_cpu::Plane>(new nfs_background_scene_cpu::Plane(f->copy.pre));
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){const auto pixel=nfs_background_scene_cpu::le16(raw.data()+16+2*(y*343+x));if(pixel)whole->pixels[y*640+x]=pixel;}
 bitmap::Binding bitmapBinding;bitmapBinding.game=reinterpret_cast<uintptr_t>(&g);bitmapBinding.thread=cacheBinding.thread;bitmapBinding.epoch=11;bitmapBinding.baseSequence=2;bitmapBinding.sequence=3;bitmapBinding.form=0x8000;bitmapBinding.previous=0x9000;bitmapBinding.family=1;bitmapBinding.archive=cacheBinding.archive;bitmapBinding.phase=1001;
 auto bitmapSource=bitmap::Prepared::create(bitmapBinding,f->copy.pre,*whole,historical,raw,raw,1);assert(bitmapSource&&bitmapSource->view(7).schema==1);
 for(unsigned k=0;k<9;++k){auto bad=bitmapBinding;if(k==0)bad.game++;if(k==1)bad.thread++;if(k==2)bad.archive++;if(k==3)bad.epoch=0;if(k==4)bad.baseSequence=0;if(k==5)bad.family=4;if(k==6)bad.phase=1000;if(k==7)bad.phase=1552;if(k==8)bad.phase=1052;assert(!bitmap::Prepared::create(bad,f->copy.pre,*whole,historical,raw,raw,1));}
 assert(!bitmap::Prepared::create(bitmapBinding,f->copy.pre,*whole,historical,raw,raw,0));assert(!bitmap::Prepared::create(bitmapBinding,f->copy.pre,*whole,historical,raw,raw,2));
 whole->pixels[479*640+639]^=1;assert(!bitmap::Prepared::create(bitmapBinding,f->copy.pre,*whole,historical,raw,raw,1));whole->pixels[479*640+639]^=1;
 auto repeat=raw;repeat[16]^=1;assert(!bitmap::Prepared::create(bitmapBinding,f->copy.pre,*whole,historical,raw,repeat,1));
 auto bitmapOwner=std::make_shared<std::atomic<bool>>(true),bitmapFrame=std::make_shared<std::atomic<bool>>(true);auto bitmapLease=bitmap::Lease::create(bitmapSource,bitmapOwner,bitmapFrame);assert(bitmapLease);auto borrowed=bitmapLease->view(8);assert(borrowed.schema&&borrowed.current(borrowed.owner));borrowed.retain(borrowed.owner);bitmapLease.reset();assert(borrowed.current(borrowed.owner));bitmapFrame->store(false);assert(!borrowed.current(borrowed.owner));borrowed.release(borrowed.owner);
 auto copiedCatalog=std::make_shared<const sidebar::Catalog>(*retained->catalog());assert(nfs_options_sidebar_origin::Origin::createNested(reinterpret_cast<uintptr_t>(&g),cacheBinding.thread,cacheBinding.archive,cacheBinding.ownerEpoch,retained,copiedCatalog));nfs_options_sidebar_origin::remember(historical);nfs_options_sidebar_origin::slot().reset();assert(nfs_options_sidebar_origin::acquire(reinterpret_cast<uintptr_t>(&g),cacheBinding.thread,cacheBinding.archive,raw.data(),raw.size())==historical);assert(historical->matches(reinterpret_cast<uintptr_t>(&g),cacheBinding.thread,cacheBinding.archive,raw.data(),raw.size()));raw[16]^=1;assert(!historical->matches(reinterpret_cast<uintptr_t>(&g),cacheBinding.thread,cacheBinding.archive,raw.data(),raw.size()));raw[16]^=1;

 input.captionProof.reset();assert(!join());input.captionProof=proof;
 input.captionBinding.policy=binding.policy==nested::Policy::Audio?nested::Policy::Graphics:nested::Policy::Audio;assert(!join());input.captionBinding=binding;
 ++input.captionBinding.form;assert(!join());input.captionBinding=binding;
 ++input.captionBinding.archive;assert(!join());input.captionBinding=binding;
 ++input.acceptedCaptionSerial;assert(!join());input.acceptedCaptionSerial=s->serial;
 raw[16]^=1;assert(!join());raw[16]^=1;
 input.captionProof.reset();input.captionBinding={};auto legacy=join();assert(legacy&&!legacy->captionWord());input.captionProof=proof;input.captionBinding=binding;assert(join());
 cap::beforeOwnerRetire(g,selected.owner);assert(!acquire()&&!cap::lastProof(g)&&display->current()&&retained->captionWord()->current());assert(historical->current());nfs_main_sidebar_origin::retireGame(reinterpret_cast<uintptr_t>(&g));assert(!historical->current());assert(!nfs_options_sidebar_origin::acquire(reinterpret_cast<uintptr_t>(&g),cacheBinding.thread,cacheBinding.archive,raw.data(),raw.size()));
 }
 assert(retained&&retained->captionWord()->view().word==3&&retained->captionWord()->packet()==retained->caption());
}
