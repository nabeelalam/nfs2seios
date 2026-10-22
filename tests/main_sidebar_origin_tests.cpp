// SPDX-License-Identifier: MIT
#include "Main343OriginFixture.h"
#include <type_traits>
#include <thread>
namespace mo=nfs_main_sidebar_origin;using MainFixture=main343_origin_fixture::Fixture;
static_assert(!std::is_default_constructible<mo::Proof>::value,"Origins require full verification");
static_assert(!std::is_copy_constructible<mo::Proof>::value,"Cannot clone revocation authority");
int main(){try{
 auto f=std::unique_ptr<MainFixture>(new MainFixture);mo::Store store;auto proof=f->publish(store);CHECK(proof&&proof->current()&&proof->frame()->valid);CHECK(proof->post().target.owner==0x1800&&proof->post().target.w[8]==0x40000);CHECK(proof->archive()==f->binding.archive&&proof->formGuest()==f->binding.formGuest&&proof->game()==f->binding.game&&proof->publisherThread()==29&&proof->epoch()==1);CHECK(proof->scene().event.sequence==2&&proof->groupEvent().sequence==3&&proof->tailEvent().sequence==4&&proof->copyEvent().sequence==5&&proof->uploadEvent().sequence==6);
 auto actual=std::unique_ptr<bg::Plane>(new bg::Plane(proof->post()));auto catalog=f->trusted();CHECK(store.acquire(*actual,f->binding.game,29,f->binding.archive,catalog)==proof);actual->target.ownerState[0]^=0x10000;CHECK(store.acquire(*actual,f->binding.game,29,f->binding.archive,catalog)==proof);CHECK(proof->post().target.ownerState[0]!=actual->target.ownerState[0]);
 // Every owned record, catalog and prepared frame survives caller mutation.
 const auto oldPixel=proof->post().pixels[0],oldFrame=proof->frame()->original[0];f->scene.tailPost.pixels[0]^=1;f->group.post.pixels[0]^=1;f->copy.sourceBefore[16]^=1;f->catalog.mapSha[0][0]^=1;CHECK(proof->post().pixels[0]==oldPixel&&proof->frame()->original[0]==oldFrame&&proof->group().post.pixels[0]!=f->group.post.pixels[0]&&proof->copy().sourceBefore[16]!=f->copy.sourceBefore[16]&&proof->catalog()->mapSha[0]!=f->catalog.mapSha[0]);
 f.reset(new MainFixture);
 // Full Main verifier is mandatory; no relabeled Player or Prepared.valid path.
 for(unsigned k=0;k<7;++k){mo::Store s;auto bad=std::unique_ptr<MainFixture>(new MainFixture);if(k==0)bad->scene.form=0x4d3050;if(k==1)bad->base.form=0x4d3100;if(k==2)bad->binding.epoch=2;if(k==3)bad->binding.publisherThread=30;if(k==4)bad->copy.post.pixels[639]^=1;if(k==5)bad->scene.tailPost.pixels[13]^=1;if(k==6)bad->scene.tailKind=3;CHECK(!bad->publish(s));}
 // Same storage new publication revokes all retained handles, even when the
 // new candidate is invalid. Address reuse cannot resurrect the old token.
 auto first=f->publish(store);CHECK(first&&first->current()&&!proof->current());auto second=f->publish(store);CHECK(second&&second!=first&&!first->current());f->copy.post.pixels[639]^=1;CHECK(!f->publish(store)&&!second->current());f.reset(new MainFixture);
 for(unsigned k=0;k<8;++k){mo::Store s;auto p=f->publish(s);CHECK(p);auto now=std::unique_ptr<bg::Plane>(new bg::Plane(p->post()));if(k==0)now->pixels[1]^=1;if(k==1)now->target.header[2]^=1;if(k==2)now->target.x[342]^=2;if(k==3)now->target.y[479]^=2;if(k==4)++now->target.owner;if(k==5)now->target.w[8]+=2;if(k==6)now->target.ownerState[1]^=1;if(k==7)now->target.ownerWords[5]--;
  CHECK(!s.acquire(*now,f->binding.game,29,f->binding.archive,f->trusted()));CHECK(!p->current());CHECK(!s.acquire(p->post(),f->binding.game,29,f->binding.archive,f->trusted()));}
 // Replacing archive/catalog/Game/publisher context revokes historical leases.
 for(unsigned k=0;k<4;++k){mo::Store s;auto p=f->publish(s);auto cat=f->trusted();auto game=f->binding.game,thread=f->binding.publisherThread;auto archive=f->binding.archive;if(k==0)++archive;if(k==1){auto changed=std::make_shared<bg::SourceCatalog>(*cat);changed->mapSha[0][0]^=1;cat=changed;}if(k==2)++game;if(k==3)++thread;CHECK(!s.acquire(p->post(),game,thread,archive,cat)&&!p->current());}
 // Bounded two-buffer preservation, deterministic third-source eviction,
 // explicit owner/bitmap retirement, normal-exit retention and reset.
 mo::Store two;auto a=f->publish(two);auto b=std::unique_ptr<MainFixture>(new MainFixture);b->retarget343(0x1900,0x70000,0x5000,0x6000);auto pb=b->publish(two);CHECK(a&&pb&&a->current()&&pb->current());CHECK(two.acquire(a->post(),f->binding.game,29,f->binding.archive,f->trusted())==a);CHECK(two.acquire(pb->post(),f->binding.game,29,f->binding.archive,f->trusted())==pb);
 auto c=std::unique_ptr<MainFixture>(new MainFixture);c->retarget343(0x1a00,0xa0000,0x7000,0x8000);auto pc=c->publish(two);CHECK(pc&&!a->current()&&pb->current());two.retireOwner(0x1900);CHECK(!pb->current()&&pc->current());two.retireBitmap(pc->post().target.w[8]);CHECK(!pc->current());auto reused=b->publish(two);CHECK(reused&&reused->current()&&!pb->current());two.clear();CHECK(!reused->current());auto last=f->publish(two);two.reset();CHECK(last&&!last->current());
 mo::Store gameStore;auto beforeGameExit=f->publish(gameStore);CHECK(beforeGameExit&&beforeGameExit->current());mo::retireGame(f->binding.game);CHECK(!beforeGameExit->current()&&!gameStore.acquire(beforeGameExit->post(),f->binding.game,29,f->binding.archive,f->trusted()));auto afterReuse=f->publish(gameStore);CHECK(afterReuse&&afterReuse->current()&&!beforeGameExit->current());
 mo::Store crossThread;auto cross=f->publish(crossThread);CHECK(cross&&cross->current());std::thread retire([&](){mo::retireGame(f->binding.game);});retire.join();CHECK(!cross->current());auto fresh=f->publish(crossThread);CHECK(fresh&&fresh->current()&&!cross->current());
 std::shared_ptr<const mo::Proof>afterDestruction;{mo::Store temporary;afterDestruction=f->publish(temporary);CHECK(afterDestruction&&afterDestruction->current());}CHECK(!afterDestruction->current());
 std::puts("PASS Main343 origin pure foundation: full Main seals, owned records, exact retained source, ambient-word-only tolerance, revocation/address reuse, two-buffer bound; no actual-runtime claim");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
