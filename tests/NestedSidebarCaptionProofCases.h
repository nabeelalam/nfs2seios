// SPDX-License-Identifier: MIT
#pragma once
#include "HdSceneFixture.h"
#include "hd/AudioSidebarPacket.h"
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
 input.captionProof.reset();assert(!join());input.captionProof=proof;
 input.captionBinding.policy=binding.policy==nested::Policy::Audio?nested::Policy::Graphics:nested::Policy::Audio;assert(!join());input.captionBinding=binding;
 ++input.captionBinding.form;assert(!join());input.captionBinding=binding;
 ++input.captionBinding.archive;assert(!join());input.captionBinding=binding;
 ++input.acceptedCaptionSerial;assert(!join());input.acceptedCaptionSerial=s->serial;
 raw[16]^=1;assert(!join());raw[16]^=1;
 input.captionProof.reset();input.captionBinding={};auto legacy=join();assert(legacy&&!legacy->captionWord());input.captionProof=proof;input.captionBinding=binding;assert(join());
 cap::beforeOwnerRetire(g,selected.owner);assert(!acquire()&&!cap::lastProof(g)&&display->current()&&retained->captionWord()->current());
 }
 assert(retained&&retained->captionWord()->view().word==3&&retained->captionWord()->packet()==retained->caption());
}
