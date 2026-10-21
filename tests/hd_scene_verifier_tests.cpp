// SPDX-License-Identifier: MIT
#include "HdSceneFixture.h"
int main(){try{
 auto f=std::unique_ptr<Fixture>(new Fixture{});CHECK(bg::validCatalog(f->catalog));CHECK(f->verify()==bg::Failure::None);CHECK(f->output.valid&&f->output.wordCount==1);
 // Independent complete-frame oracle: opaque synthetic panel and single frame pixel.
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){const uint16_t expected=x>=343?0x2a55:(x==0&&y==0?f->frameColor:f->sideColor);CHECK(f->output.original[y*640+x]==expected);}
 CHECK(f->output.words[0].scalar256==256&&f->output.logos[0].role==1&&f->output.logos[1].scalar256==0&&f->output.logos[2].role==0);
 unsigned negatives=0;
 auto reject=[&](){CHECK(f->verify()!=bg::Failure::None&&!f->output.valid);++negatives;};
 f->catalog.roles[12]=11;reject();f->catalog.roles[12]=12;
 auto good=f->catalog.identities[0];f->catalog.identities[0].span=16;reject();f->catalog.identities[0]=good;
 f->catalog.identities[0].paletteOffset=UINT32_MAX;reject();f->catalog.identities[0]=good;
 f->catalog.logos.roles[4]=0;reject();f->catalog.logos.roles[4]=4;
 auto logoGood=f->catalog.logos.identities[0];f->catalog.logos.identities[0].maxIndex=256;reject();f->catalog.logos.identities[0]=logoGood;
 auto mapGood=f->catalog.mapSha[1];f->catalog.mapSha[1]=f->catalog.mapSha[0];reject();f->catalog.mapSha[1]=mapGood;
 f->catalog.identities[0].sha={};reject();f->catalog.identities[0]=good;
 f->base.event.ownerEpoch=2;reject();f->base.event.ownerEpoch=1;
 f->scene.event.iteration=2;reject();f->scene.event.iteration=1;
 f->scene.draws[0].paletteAfter[1]^=1;reject();f->scene.draws[0].paletteAfter[1]^=1;
 f->scene.draws[0].rows[0].args[2]--;reject();f->scene.draws[0].rows[0].args[2]++;
 f->scene.draws[1].sourceAfter[16]^=1;reject();f->scene.draws[1].sourceAfter[16]^=1;
 f->copy.post.pixels[639]^=1;reject();f->copy.post.pixels[639]^=1;
 f->keys[1]=f->keys[0];reject();f->keys[1]=2048;
 f->scene.tailPost.pixels[42]^=1;reject();f->scene.tailPost.pixels[42]^=1;
 CHECK(f->verify()==bg::Failure::None);auto oldView=f->output.view();CHECK(oldView.serial==1&&oldView.original565==f->output.original.data());
 // Every interior LUT word participates in exact ownership equality.
 auto changed=f->copy.post.target;changed.x[313]^=1;CHECK(!rec::same(changed,f->copy.post.target));changed=f->copy.post.target;changed.y[279]^=1;CHECK(!rec::same(changed,f->copy.post.target));
 std::printf("PASS synthetic full-frame oracle307200 pixels, %u rejection cases, trusted catalog and original-owned records\n",negatives);
 return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
