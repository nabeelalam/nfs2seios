// SPDX-License-Identifier: MIT
#include "PlayerSceneFixture.h"
#include <functional>
namespace fx=player_scene_fixture;namespace ps=nfs_player_scene_cpu;
int main(){try{unsigned negatives=0;
 for(unsigned player:{1u,2u})for(unsigned mode:{0u,1u,2u}){auto f=std::unique_ptr<fx::Fixture>(new fx::Fixture(player,mode));CHECK(f->source&&f->source->form()==f->binding.form&&f->source->player()==player);CHECK(f->source->original()==f->actualUpload.pixels);ps::Prepared out;CHECK(ps::untrustedPreparedLayout(f->source,f->keys,out));auto v=out.view();CHECK(v.schema==1&&v.form==f->binding.form&&v.scene.tailKind==ps::tail(f->binding.form)&&v.scene.planeWords==bg::Full);CHECK(out.caption==f->caption&&out.preview==f->preview&&out.swatch==f->swatch&&out.overlay);CHECK(v.preview.pixelCount==v.preview.coverageCount&&v.preview.pixelCount==size_t(v.preview.width)*v.preview.height);
 // The last union overlay may not cover original opaque343 source words.
 for(unsigned y=0;y<v.preview.height;++y)for(unsigned x=0;x<v.preview.width;++x){unsigned xx=v.preview.left+x,yy=v.preview.top+y,k=y*v.preview.width+x;if(xx<343&&f->scene.tailPost.pixels[yy*343+xx])CHECK(!v.preview.coverage[k]);}
 const auto retained=out.original;f->actualUpload.pixels.fill(0);f->caption.reset();f->preview.reset();f->swatch.reset();CHECK(out.original==retained&&out.caption&&out.preview);
 }
 // A genuine later Lazy copy may have no retained Refresh (e.g. initial
 // menu state); independent currentcopy/upload iterations must both be nonzero.
 for(unsigned player:{1u,2u}){auto f=std::unique_ptr<fx::Fixture>(new fx::Fixture(player,0));auto i=f->input();i.binding.kind=ps::SourceKind::Lazy;i.copyEvent.iteration=i.uploadEvent.iteration=i.sidebarValidationEvent.iteration=1;CHECK(ps::verifySource(i,f->workspace,f->catalog));}
 auto bad=[&](unsigned mode,const std::function<void(fx::Fixture&,ps::JoinedInput&)>&mutate){auto f=std::unique_ptr<fx::Fixture>(new fx::Fixture(1,mode));auto i=f->input();mutate(*f,i);ps::Failure failure;CHECK(!ps::verifySource(i,f->workspace,f->catalog,&failure));CHECK(failure!=ps::Failure::None);++negatives;};
 bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.binding.form=ps::Player2Form;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.binding.sourceOwner++;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.binding.copySerial++;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.binding.epoch++;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.acceptedSidebarSerial++;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.sidebarLifetime++;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.sidebarValidationEvent.sequence=51;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.sidebarValidationEvent.sequence=53;});bad(0,[](fx::Fixture&f,ps::JoinedInput&){f.scene.tailPost.pixels[10]^=1;});bad(0,[](fx::Fixture&f,ps::JoinedInput&){f.actualUpload.pixels[639]^=1;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.acceptedPreviewSerial++;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.previewBaseEvent.sequence++;});bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.previewEvent.sequence=50;});bad(0,[](fx::Fixture&f,ps::JoinedInput&){f.base.post.pixels[0]^=1;});bad(0,[](fx::Fixture&f,ps::JoinedInput&){f.copy.sourceAfter[16]^=1;});bad(0,[](fx::Fixture&f,ps::JoinedInput&){f.copy.post.pixels[0]^=1;});bad(0,[](fx::Fixture&f,ps::JoinedInput&){f.copy.sourceOwner.owner=f.copy.pre.target.owner;});
 bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.swatchAuthority.binding.origin=nfs_player_swatch::Origin::Body;});bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.swatchAuthority.binding.owner++;});bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.swatchAuthority.binding.serial++;});bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.swatchCopyEvent.sequence=54;});bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.acceptedSwatchCopySerial++;});bad(1,[](fx::Fixture&f,ps::JoinedInput&){f.oldCopyPost.pixels[0]^=1;});bad(1,[](fx::Fixture&f,ps::JoinedInput&){f.s->current.pixels[0]^=1;});bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.swatchEvent.sequence++;});bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.uploadEvent.sequence=59;});bad(2,[](fx::Fixture&,ps::JoinedInput&i){i.previewEvent.sequence=61;});bad(2,[](fx::Fixture&,ps::JoinedInput&i){i.swatchEvent.ownerEpoch++;});
 bad(0,[](fx::Fixture&f,ps::JoinedInput&){f.catalog.identities[1].sha[0]^=1;});
 // Every enum below is individually valid; chronology must still bind it.
 bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.binding.kind=ps::SourceKind::Refresh;});
 bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.binding.kind=ps::SourceKind::Lazy;});
 bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.binding.kind=ps::SourceKind::Initial;});
 bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.binding.kind=ps::SourceKind::Lazy;});
 bad(2,[](fx::Fixture&,ps::JoinedInput&i){i.binding.kind=ps::SourceKind::Initial;});
 bad(2,[](fx::Fixture&,ps::JoinedInput&i){i.binding.kind=ps::SourceKind::Refresh;});
 // Coherent Refresh->Lazy relabelling/event movement must not reuse the
 // SAME original copy identity as both refresh-copy and new-lazy-copy.
 bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.binding.kind=ps::SourceKind::Lazy;i.sidebarValidationEvent={1,72,1};i.copyEvent={1,73,1};i.uploadEvent={1,90,1};});
 // Coherent Lazy->Refresh relabelling cannot rewrite the sealed historical
 // SwatchPacket's PRE, original events and exact currentcopy binding.
 bad(2,[](fx::Fixture&f,ps::JoinedInput&i){i.binding.kind=ps::SourceKind::Refresh;i.sidebarValidationEvent.iteration=i.copyEvent.iteration=i.uploadEvent.iteration=0;i.swatchCopyEvent=i.copyEvent;i.acceptedSwatchCopySerial=f.copy.serial;i.swatchCopyPost=&f.copy.post;i.swatchAuthority.events={74,75,76,77,78,79,80};i.swatchEvent={1,80,0};i.uploadEvent.sequence=90;});
 bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.copyEvent.iteration=i.uploadEvent.iteration=i.sidebarValidationEvent.iteration=1;});
 bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.copyEvent.iteration=i.uploadEvent.iteration=i.sidebarValidationEvent.iteration=1;});
 bad(2,[](fx::Fixture&,ps::JoinedInput&i){i.copyEvent.iteration=i.uploadEvent.iteration=i.sidebarValidationEvent.iteration=0;});
 bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.swatchCopyEvent.iteration=1;});
 bad(1,[](fx::Fixture&,ps::JoinedInput&i){i.swatchEvent.iteration=1;});
 bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.sidebarValidationEvent.iteration=1;});
 bad(0,[](fx::Fixture&,ps::JoinedInput&i){i.uploadEvent.iteration=1;});
 bad(2,[](fx::Fixture&,ps::JoinedInput&i){i.swatchEvent.sequence=i.copyEvent.sequence;});
 CHECK(negatives==45);std::printf("PASS Player scene pure negatives=%u no-live-activation=1\n",negatives);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
