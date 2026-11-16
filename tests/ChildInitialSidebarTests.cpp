// SPDX-License-Identifier: MIT
// Reuse the existing synthetic guest/caption/Main-verifier fixture unchanged.
#define main existing_caption_fixture_main
#include "options_caption_runtime_tests.cpp"
#undef main
#include "hd/ChildInitialSidebarProof.h"
int main(int argc,char**argv){const uint32_t previous=argc>=2?uint32_t(std::strtoul(argv[1],nullptr,0)):off_4D3EC4;assert(nfs_soft::installFontCatalog(font()));prepare();Game g;g.eax=testOwner;g.esi=testOwner;g.ebp=previous;
 auto f=std::unique_ptr<main343_origin_fixture::Fixture>(new main343_origin_fixture::Fixture);f->binding.game=reinterpret_cast<uintptr_t>(&g);f->binding.publisherThread=29;f->group.thread=f->copy.thread=29;f->binding.formGuest=off_4D3EC4;f->binding.archive=0x1234;
 nfs_main_sidebar_origin::Store origins;auto origin=f->publish(origins);assert(origin&&origin->current());selected=origin->post().target;selected.ownerState[0]=0;std::memcpy(memory.data()+selected.w[8],selected.header.data(),16);std::memcpy(memory.data()+selected.w[8]+16,origin->post().pixels.data(),343*480*2);std::memcpy(memory.data()+selected.owner,selected.ownerWords.data(),56);put<uint32_t>(0x1c00+32,0xa0000);put<uint32_t>(dword_4D2314+2,0);put<uint32_t>(dword_4D231C,selected.ownerState[1]);put<uint32_t>(dword_4D2320,selected.ownerState[2]);put<uint32_t>(dword_4D243C,previous);
 cap::OwnerScope owner(g);
 {cap::ChildInitialCallerScope caller(g,origin);{cap::TailScope tail(g);trackedString(g,"normal");}g.esi=testOwner;if(argc==3){g.ebp=-1;put<uint32_t>(dword_4D243C,UINT32_MAX);}}
 auto caption=cap::acquireCurrentProof(g);if(argc==3||(previous!=0&&previous!=off_4D3EC4&&previous!=UINT32_MAX&&previous!=UINT32_MAX-1)){assert(!caption);std::puts("PASS child initial source rejects changed/unsupported previous form");return 0;}assert(caption&&caption->evidence().route==nfs_compact_caption::Route::InitialMain);
 auto actual=std::unique_ptr<bg::Plane>(new bg::Plane(origin->post()));actual->target=selected;std::memcpy(actual->pixels.data(),memory.data()+selected.w[8]+16,bg::Side*2);
 namespace child=nfs_child_initial_sidebar;
 #if defined(NFS_TEST_SETUP_CAPTION)
 using Policy=child::SetupPolicy;using Wrong=child::OptionsPolicy;
 #else
 using Policy=child::OptionsPolicy;using Wrong=child::SetupPolicy;
 #endif
 child::Binding binding;binding.form=Policy::form;binding.formGuest=testOwner;binding.archive=0x1234;binding.game=reinterpret_cast<uintptr_t>(&g);binding.thread=29;binding.epoch=1;binding.captionEvent={1,1,0};
 auto workspace=std::unique_ptr<bg::Workspace>(new bg::Workspace);child::Store<Policy>store;auto source=store.accept(origin,caption,binding,*actual,*workspace,origin->catalog());assert(source&&source->current()&&source->wordView().schema);
 child::Store<Wrong>wrong;assert(!wrong.accept(origin,caption,binding,*actual,*workspace,origin->catalog()));
 for(unsigned bad=0;bad<8;++bad){auto bnd=binding;auto plane=std::unique_ptr<bg::Plane>(new bg::Plane(*actual));if(bad==0)++bnd.form;if(bad==1)++bnd.formGuest;if(bad==2)++bnd.archive;if(bad==3)++bnd.game;if(bad==4)++bnd.thread;if(bad==5)++bnd.epoch;if(bad==6)plane->pixels[100]^=1;if(bad==7)plane->target.x[342]^=2;child::Store<Policy>reject;assert(!reject.accept(origin,caption,bnd,*plane,*workspace,origin->catalog()));}
 store.clear();assert(!source->current()&&!source->wordView().schema);std::puts("PASS child initial source: actual caption scopes/Main verifier, fixedpolicy/8negatives/revocation, noGPUclaim");
}
