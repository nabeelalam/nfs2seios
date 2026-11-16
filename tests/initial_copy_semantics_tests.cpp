// SPDX-License-Identifier: MIT
#include "HdSceneFixture.h"
#include "hd/ContinuousLogoCollector.h"
namespace pub=nfs_logo_publisher;
static std::unique_ptr<pub::Collector> collector(Fixture&f,bool transparent){
 auto c=std::unique_ptr<pub::Collector>(new pub::Collector);
 c->catalog=&f.catalog.logos;c->inMenu=true;c->gameOwner=7;c->thread=29;
 c->depth=1;c->groupFinished=true;c->iterationRevision=17;c->group=f.group;c->copy=f.copy;c->copy.serial=17;
 // Genuine independently constructed semantic fixture: the logo overwrites
 // black underneath, and the pre-map destination is also black there.
 if(transparent){c->group.pre.pixels[0]=0;c->copy.pre.pixels[0]=0;}
 return c;
}
static void transparentSeal(pub::Collector&c){
 CHECK(!c.finishCopy());CHECK(c.stop==pub::Collector::Stop::TransparentUnderlay);
 CHECK(c.copySourceVerified&&!c.prepared&&!c.packet.prepared.valid&&!c.bound&&!c.uploadActive);
 CHECK(c.group.complete&&c.copy.complete);c.leaveIteration();
}
int main(){try{
 auto f=std::unique_ptr<Fixture>(new Fixture);
 {auto c=collector(*f,false);CHECK(c->finishCopy());CHECK(c->copySourceVerified&&c->prepared&&c->packet.prepared.valid);}
 {auto c=collector(*f,true);transparentSeal(*c);CHECK(c->beginRetainedCopy(7,29));CHECK(c->depth==1&&c->copyActive&&!c->copySourceVerified&&!c->prepared&&!c->packet.prepared.valid&&!c->bound);}
 {auto c=collector(*f,true);transparentSeal(*c);CHECK(c->beginRetainedCopy(7,29));c->copy=f->copy;c->copy.serial=17;CHECK(c->finishCopy());CHECK(c->prepared&&c->packet.prepared.valid);CHECK(c->beginUpload(c->copy.post,1));CHECK(!c->finishUpload(c->copy.post,true));CHECK(!c->bound);}
 {auto c=collector(*f,true);transparentSeal(*c);CHECK(!c->beginUpload(c->copy.post,1));CHECK(c->copySourceVerified&&!c->bound);CHECK(c->beginRetainedCopy(7,29));}
 {auto c=collector(*f,true);transparentSeal(*c);c->bound=true;c->uploadActive=true;c->uploaded=300;c->slot1Overwrite();CHECK(c->copySourceVerified&&!c->prepared&&!c->packet.prepared.valid&&!c->bound&&!c->uploadActive&&!c->uploaded);CHECK(c->beginRetainedCopy(7,29));}
 {auto c=collector(*f,true);transparentSeal(*c);c->slot1Overwrite();c->copy.post.pixels[0]^=1;CHECK(!c->beginRetainedCopy(7,29));CHECK(!c->copySourceVerified);}
 {auto c=collector(*f,true);transparentSeal(*c);CHECK(!c->beginRetainedCopy(8,29));CHECK(!c->copySourceVerified);}
 {auto c=collector(*f,true);transparentSeal(*c);CHECK(!c->beginRetainedCopy(7,30));CHECK(!c->copySourceVerified);}
 {auto c=collector(*f,true);transparentSeal(*c);c->group.post.pixels[0]^=1;CHECK(!c->beginRetainedCopy(7,29));CHECK(!c->copySourceVerified&&c->stop==pub::Collector::Stop::Verifier);}
 {auto c=collector(*f,true);transparentSeal(*c);c->copy.error=1;CHECK(!c->beginRetainedCopy(7,29));CHECK(!c->copySourceVerified);}
 for(unsigned k=0;k<=unsigned(pub::Collector::Stop::Overflow);++k){auto c=collector(*f,true);transparentSeal(*c);c->invalidate(pub::Collector::Stop(k));CHECK(!c->copySourceVerified&&!c->prepared&&!c->packet.prepared.valid);}
 {auto c=collector(*f,true);transparentSeal(*c);c->retire(c->copy.sourceOwner.owner);CHECK(!c->copySourceVerified);}
 {auto c=collector(*f,true);transparentSeal(*c);c->reset();CHECK(!c->copySourceVerified);}
 {auto c=collector(*f,true);transparentSeal(*c);c->endMenu(7,true);CHECK(!c->copySourceVerified);CHECK(c->beginMenu(8,30));CHECK(!c->copySourceVerified);}
 {auto c=collector(*f,true);transparentSeal(*c);CHECK(c->enterIteration(1,0,29));CHECK(!c->copySourceVerified);}
 {auto c=collector(*f,true);CHECK(!c->finishCopy());c->leaveIteration(true);CHECK(!c->copySourceVerified);}
 {auto c=collector(*f,true);c->copy.post.pixels[0]^=1;CHECK(!c->finishCopy());CHECK(c->stop==pub::Collector::Stop::Verifier&&!c->copySourceVerified);}
 std::puts("initial CPU copy semantic and native separation checks passed");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
