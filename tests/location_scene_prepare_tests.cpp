// SPDX-License-Identifier: MIT
#include "HdSceneFixture.h"
#include "hd/LocationScenePrepare.h"
namespace loc=nfs_location_scene_cpu;
int main(){try{
 auto f=std::unique_ptr<Fixture>(new Fixture);f->base.form=f->scene.form=loc::LocationForm;
 f->scene.tailKind=NFS_BACKGROUND_TAIL_LOCATION_CAPTION;f->scene.tailPredicate=f->scene.tailSymbol=loc::LocationForm;
 nfs_soft::FontDescription fd{};std::vector<uint8_t> alpha(1024*512);alpha[1025]=255;
 for(auto&role:fd.glyphs)for(auto&g:role){g.x=g.y=g.w=g.h=1;g.advance=65536;}
 auto font=nfs_soft::FontCatalog::create(fd,alpha.data(),alpha.size(),77);CHECK(font);
 nfs_soft::SurfaceIdentity identity;CHECK(loc::captionIdentity(f->copy.sourceOwner,7,identity));
 nfs_soft::FramePacketBuilder builder(font);CHECK(builder.begin(identity,5,f->scene.tailPre.pixels.data(),bg::Side));
 nfs_soft::TextCommand command{};command.length=8;std::memcpy(command.text.data(),"location",8);command.role=1;command.x=116;command.y=162;command.sourceSize=18;command.advance=8;command.color=65535;command.clip={0,0,343,480};CHECK(builder.beginString(command));
 const unsigned at=170*343+120;CHECK(builder.word(at,f->scene.tailPre.pixels[at],65535,65535));f->scene.tailPost.pixels[at]=65535;CHECK(builder.endString(command));
 auto caption=builder.finish(identity,f->scene.tailPost.pixels.data(),bg::Side);CHECK(caption);
 // Update the independently observed actual Copy source, converter rows and full640 post.
 for(unsigned k=0;k<bg::Side;++k)put16(f->copy.sourceBefore.data()+16+2*k,f->scene.tailPost.pixels[k]);f->copy.sourceAfter=f->copy.sourceBefore;CHECK(nfs_texture_sha256(f->copy.sourceBefore.data(),f->copy.span,f->copy.shaBefore.data()));f->copy.shaAfter=f->copy.shaBefore;
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){auto value=f->scene.tailPost.pixels[y*343+x];put32(f->copy.row[y].source.data()+4*x,rgba(value));f->copy.post.pixels[y*640+x]=value;}
 auto out=std::unique_ptr<loc::Prepared>(new loc::Prepared);
 loc::JoinedInput input{{f->base,f->scene,f->group,f->copy,{1,3,1},{1,4,1},{1,5,1},{1,6,1},f->copy.post.target,1,1,f->keys},caption,5,7};
 auto verify=[&](){return loc::verifyAndPrepare(input,f->workspace,*out,f->catalog);};CHECK(verify()==bg::Failure::None);CHECK(out->valid&&out->caption==caption);
 // Independent full original oracle, including the changed caption pixel.
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){uint16_t expected=x>=343?0x2a55:(x==0&&y==0?f->frameColor:f->sideColor);if(y==170&&x==120)expected=65535;CHECK(out->original[y*640+x]==expected);}
 auto v=out->view();CHECK(v.schema==1&&v.scene.tailKind==NFS_BACKGROUND_TAIL_LOCATION_CAPTION&&v.caption.logicalWidth==343&&v.caption.vertexCount&&v.caption.font.identity==77);CHECK(v.caption.font.alpha==font->alpha().data());
 auto firstView=out->view();CHECK(!std::memcmp(firstView.caption.font.sha256,v.caption.font.sha256,32));
 // A different immutable owner (even same numeric identity) must rehash, then returning to first owner restores its digest.
 auto otherAlpha=alpha;otherAlpha[4096]=17;auto otherFont=nfs_soft::FontCatalog::create(fd,otherAlpha.data(),otherAlpha.size(),77);CHECK(otherFont);
 nfs_soft::FramePacketBuilder otherBuilder(otherFont);CHECK(otherBuilder.begin(identity,1,f->scene.tailPre.pixels.data(),bg::Side));CHECK(otherBuilder.beginString(command));CHECK(otherBuilder.word(at,f->scene.tailPre.pixels[at],65535,65535));CHECK(otherBuilder.endString(command));auto otherPacket=otherBuilder.finish(identity,f->scene.tailPost.pixels.data(),bg::Side);CHECK(otherPacket);
 out->caption=otherPacket;auto otherView=out->view();CHECK(otherView.caption.font.alpha==otherFont->alpha().data()&&std::memcmp(otherView.caption.font.sha256,v.caption.font.sha256,32));out->caption=caption;auto returnedView=out->view();CHECK(!std::memcmp(returnedView.caption.font.sha256,v.caption.font.sha256,32));
 // Ambient selected-buffer changes do not fabricate a new source lifetime.
 f->copy.sourceOwner.ownerState[0]^=0x10000;CHECK(verify()==bg::Failure::None);f->copy.sourceOwner.ownerState[0]^=0x10000;
 unsigned negatives=0;auto reject=[&](){CHECK(verify()!=bg::Failure::None&&!out->valid);++negatives;};
 f->scene.tailPost.valid=0;reject();f->scene.tailPost.valid=1;
 input.acceptedCaptionSerial++;reject();input.acceptedCaptionSerial--;
 input.captionLifetime++;reject();input.captionLifetime--;
 f->copy.sourceOwner.owner^=4;reject();f->copy.sourceOwner.owner^=4;
 input.caption.reset();reject();input.caption=caption;
 f->scene.tailPre.pixels[at]^=1;reject();f->scene.tailPre.pixels[at]^=1;
 f->scene.tailPost.pixels[at]^=1;reject();f->scene.tailPost.pixels[at]^=1;
 f->copy.sourceOwner.x[150]^=2;reject();f->copy.sourceOwner.x[150]^=2;
 f->copy.sourceOwner.y[150]^=2;reject();f->copy.sourceOwner.y[150]^=2;
 f->copy.sourceBefore[16+2*at]^=1;reject();f->copy.sourceBefore[16+2*at]^=1;
 f->copy.post.pixels[639]^=1;reject();f->copy.post.pixels[639]^=1;
 input.tailEvent.sequence=input.copyEvent.sequence;reject();input.tailEvent.sequence=4;
 input.rendererAddress[1]=0;reject();input.rendererAddress[1]=2048;
 f->scene.form=bg::MainForm;reject();f->scene.form=loc::LocationForm;
 f->scene.tailKind=NFS_BACKGROUND_TAIL_MAIN_SKIP;reject();f->scene.tailKind=NFS_BACKGROUND_TAIL_LOCATION_CAPTION;
 f->scene.draws[0].sourceAfter[16]^=1;reject();f->scene.draws[0].sourceAfter[16]^=1;
 f->group.post.pixels[0]^=1;reject();f->group.post.pixels[0]^=1;
 CHECK(verify()==bg::Failure::None);std::printf("PASS Location full307200 original oracle + %u stale/tail/source/LUT/union/order negatives\n",negatives);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
