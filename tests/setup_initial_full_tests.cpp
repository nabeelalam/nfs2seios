// SPDX-License-Identifier: MIT
#include "HdSceneFixture.h"
#include "hd/SetupInitialFullProof.h"
int main(){auto f=std::unique_ptr<Fixture>(new Fixture);auto after=std::unique_ptr<nfs_background_scene_cpu::Plane>(new nfs_background_scene_cpu::Plane(f->copy.pre));auto side=std::unique_ptr<nfs_background_scene_cpu::Plane>(new nfs_background_scene_cpu::Plane(f->group.post));const auto&base=f->copy.pre;const uint32_t key=0;
 side->pixels[0]=0;for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x)after->pixels[y*640+x]=nfs_small_logo_live::pixel(base.pixels[y*640+x],nfs_small_logo_live::expanded(side->pixels[y*343+x],key),false);
 CHECK(nfs_setup_initial_full::copied(base,*side,*after,key));CHECK(after->pixels[0]==base.pixels[0]);
 after->pixels[500]^=1;CHECK(!nfs_setup_initial_full::copied(base,*side,*after,key));after->pixels[500]^=1;
 after->pixels[644]^=1;CHECK(!nfs_setup_initial_full::copied(base,*side,*after,key));after->pixels[644]^=1;
 side->pixels[500]^=1;CHECK(!nfs_setup_initial_full::copied(base,*side,*after,key));side->pixels[500]^=1;
 after->target.w[8]^=4;CHECK(!nfs_setup_initial_full::copied(base,*side,*after,key));after->target.w[8]^=4;
 after->target.x[343]^=2;CHECK(!nfs_setup_initial_full::copied(base,*side,*after,key));after->target.x[343]^=2;
 side->width=640;CHECK(!nfs_setup_initial_full::copied(base,*side,*after,key));side->width=343;
 after->valid=0;CHECK(!nfs_setup_initial_full::copied(base,*side,*after,key));std::puts("PASS Setup initial full copy: keyed transparency, exact full-plane replay and7mutations");}
