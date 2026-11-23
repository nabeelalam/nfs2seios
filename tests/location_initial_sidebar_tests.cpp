// SPDX-License-Identifier: MIT
// Reuse genuine Tail/String/Word source observers; no fabricated caption seal.
#define main location_caption_existing_test_main
#include "location_caption_runtime_tests.cpp"
#undef main
#include "Main343OriginFixture.h"
#include "hd/LocationInitialSidebarProof.h"
int main(){
 namespace initial=nfs_location_initial_sidebar;
 assert(nfs_soft::installFontCatalog(font()));prepare();
 Game g;g.eax=off_4D2A20;
 auto f=std::unique_ptr<main343_origin_fixture::Fixture>(new main343_origin_fixture::Fixture);
 f->binding.game=reinterpret_cast<uintptr_t>(&g);f->binding.archive=0x1234;
 nfs_main_sidebar_origin::Store origins;auto origin=f->publish(origins);assert(origin);
 selected=origin->post().target;std::memcpy(memory.data()+selected.w[8],selected.header.data(),16);std::memcpy(memory.data()+selected.w[8]+16,origin->post().pixels.data(),bg::Side*2);
 std::shared_ptr<const initial::Proof>escaped;
 {cap::OwnerScope owner(g);{cap::TailScope tail(g);stringBody(g,"proof-normal");}
 auto caption=cap::acquireCurrentProof(g);assert(caption);const auto&e=caption->evidence();
 auto actual=std::unique_ptr<bg::Plane>(new bg::Plane(origin->post()));std::copy(caption->packet()->original().begin(),caption->packet()->original().end(),actual->pixels.begin());
 initial::Binding binding{initial::Form,off_4D2A20,0x1234,origin->formGuest(),reinterpret_cast<uintptr_t>(&g),29,1,e.serial,e.lifetime};
 initial::Store store;auto catalog=f->trusted();auto accept=[&](initial::Binding b){return store.accept(origin,caption,b,*actual,f->workspace,catalog);};
 auto p=accept(binding);assert(p&&p->current());auto v=p->view();assert(v.schema==1&&v.form==initial::Form&&v.ownerEpoch==1&&v.captionSerial==e.serial&&v.captionLifetime==e.lifetime&&v.caption.word==2&&v.caption.left==116&&v.caption.top==162&&v.caption.rgba);
 assert(store.acquire(binding,*actual,catalog)==p);
 // Direct-child retention ignores only the ambient selected-buffer word.
 // Actual pane handles and every source pixel remain authoritative. This
 // exercises the underlying proof store, not full-copy/history/native admission.
 actual->target.ownerState[0]^=0x10000;assert(store.acquire(binding,*actual,catalog)==p);actual->target.ownerState[0]^=0x10000;
 actual->target.ownerState[1]^=4;assert(!store.acquire(binding,*actual,catalog)&&!p->current());actual->target.ownerState[1]^=4;p=accept(binding);assert(p);
 actual->pixels[100]^=1;assert(!store.acquire(binding,*actual,catalog)&&!p->current());actual->pixels[100]^=1;p=accept(binding);assert(p);
 auto zero=binding;zero.incomingEbp=0;p=accept(zero);assert(p&&p->current());assert(store.acquire(zero,*actual,catalog)==p);auto zeroChanged=zero;zeroChanged.incomingEbp=UINT32_MAX;assert(!store.acquire(zeroChanged,*actual,catalog)&&!p->current());p=accept(binding);assert(p);
 std::vector<uint8_t>raw(16+bg::Side*2);std::memcpy(raw.data(),actual->target.header.data(),16);std::memcpy(raw.data()+16,actual->pixels.data(),bg::Side*2);assert(p->sourceMatches(actual->target.w[8],raw.data(),raw.size()));
 assert(!p->sourceMatches(actual->target.w[8]+2,raw.data(),raw.size()));assert(!p->sourceMatches(actual->target.w[8],raw.data(),raw.size()-2));raw[0]^=1;assert(!p->sourceMatches(actual->target.w[8],raw.data(),raw.size()));raw[0]^=1;raw[216]^=1;assert(!p->sourceMatches(actual->target.w[8],raw.data(),raw.size()));raw[216]^=1;

 for(const char*mode:{"form","descriptor","archive","game","thread","epoch","serial","lifetime","incoming"}){auto b=binding;if(!std::strcmp(mode,"form"))++b.form;if(!std::strcmp(mode,"descriptor"))++b.formGuest;if(!std::strcmp(mode,"archive"))++b.archive;if(!std::strcmp(mode,"game"))++b.game;if(!std::strcmp(mode,"thread"))++b.thread;if(!std::strcmp(mode,"epoch"))b.epoch=0;if(!std::strcmp(mode,"serial"))++b.captionSerial;if(!std::strcmp(mode,"lifetime"))++b.captionLifetime;if(!std::strcmp(mode,"incoming"))b.incomingEbp=7;assert(!accept(b));}
 actual->pixels[100]^=1;assert(!accept(binding));actual->pixels[100]^=1;
 actual->target.x[10]^=2;assert(!accept(binding));actual->target.x[10]^=2;
 // A genuine caption seal from a different clean pre-plane is insufficient.
 // Instead of inventing a forged origin seal, test the independently captured
 // pre-plane mutation through a second genuine Tail/String/Word capture.
 std::memcpy(memory.data()+selected.w[8]+16,origin->post().pixels.data(),bg::Side*2);put<uint16_t>(selected.w[8]+16+200,0x1111);
 {cap::TailScope tail(g);stringBody(g,"proof-normal");}auto wrongCaption=cap::acquireCurrentProof(g);assert(wrongCaption);
 auto wrongBinding=binding;wrongBinding.captionSerial=wrongCaption->evidence().serial;wrongBinding.captionLifetime=wrongCaption->evidence().lifetime;auto wrongActual=std::unique_ptr<bg::Plane>(new bg::Plane(*actual));std::copy(wrongCaption->packet()->original().begin(),wrongCaption->packet()->original().end(),wrongActual->pixels.begin());assert(!store.accept(origin,wrongCaption,wrongBinding,*wrongActual,f->workspace,catalog));
 // The original historical caption remains immutable; store owner clear revokes it.
 p=accept(binding);assert(p);escaped=p;store.clear();assert(!escaped->current()&&!escaped->view().schema);
 p=accept(binding);assert(p);auto changed=binding;++changed.epoch;assert(!store.acquire(changed,*actual,catalog));assert(!p->current());
 p=accept(binding);assert(p);origins.clear();assert(!p->current()&&!p->view().schema);
 }
 assert(!leaseDepth&&allocationsInLease==0);std::puts("PASS Location initial: genuine Main origin+caption, clean/post/target/binding/lifetime/owner rejection; no copy/upload/preview authority");
}
