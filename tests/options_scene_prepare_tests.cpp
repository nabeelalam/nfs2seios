// SPDX-License-Identifier: MIT
#include "HdSceneFixture.h"
#ifdef NFS_TEST_SETUP_SCENE
#include "hd/SetupScenePrepare.h"
namespace opt=nfs_setup_scene_cpu;
static constexpr uint32_t testForm=opt::SetupForm,testTail=NFS_BACKGROUND_TAIL_SETUP_CAPTION;
static constexpr unsigned textLength=10,textLeft=93,textY=126,textAdvance=75;
static constexpr char testText[]="game setup";
#else
#include "hd/OptionsScenePrepare.h"
namespace opt=nfs_options_scene_cpu;
static constexpr uint32_t testForm=opt::OptionsForm,testTail=NFS_BACKGROUND_TAIL_OPTIONS_CAPTION;
static constexpr unsigned textLength=7,textLeft=120,textY=364,textAdvance=48;
static constexpr char testText[]="options";
#endif
struct OptionsFixture:Fixture {
 static constexpr unsigned CaptionAt=369*343+123,TransparentAt=5*343+20;
 nfs_soft::FontDescription description{};std::vector<uint8_t>alpha;
 std::shared_ptr<const nfs_soft::FontCatalog>font;
 std::shared_ptr<const nfs_soft::FramePacket>caption;nfs_soft::TextCommand command{};
 std::unique_ptr<opt::Prepared>prepared{new opt::Prepared};
 OptionsFixture():alpha(1024*512){
  base.form=scene.form=testForm;scene.tailKind=testTail;scene.tailPredicate=scene.tailSymbol=testForm;
  // A genuinely transparent source word must retain the authenticated map,
  // rather than a stale full-plane Copy.PRE supplied by a candidate.
  auto&side=scene.draws[0];side.sourceBefore[16+TransparentAt]=0;side.sourceAfter=side.sourceBefore;side.rows[0].payload[TransparentAt]=0;catalog.identities[0].sha=digest(side.sourceBefore);
  scene.postSide.pixels[TransparentAt]=scene.postWords.pixels[TransparentAt]=0;group.pre.pixels[TransparentAt]=group.post.pixels[TransparentAt]=0;scene.tailPre.pixels[TransparentAt]=scene.tailPost.pixels[TransparentAt]=0;
  alpha[1025]=255;for(auto&role:description.glyphs)for(auto&g:role){g.x=g.y=g.w=g.h=1;g.advance=65536;}
  font=nfs_soft::FontCatalog::create(description,alpha.data(),alpha.size(),77);CHECK(font);
  command.length=textLength;std::memcpy(command.text.data(),testText,textLength);command.role=1;command.alignment=2;command.x=textLeft;command.y=textY;command.sourceSize=18;command.advance=textAdvance;command.color=65535;command.clip={0,0,343,480};
  caption=packet(font,5);CHECK(caption);syncCopy();
 }
 std::shared_ptr<const nfs_soft::FramePacket>packet(const std::shared_ptr<const nfs_soft::FontCatalog>&f,uint64_t serial){
  nfs_soft::SurfaceIdentity identity{};CHECK(opt::captionIdentity(copy.sourceOwner,7,identity));nfs_soft::FramePacketBuilder builder(f);CHECK(builder.begin(identity,serial,scene.tailPre.pixels.data(),bg::Side));CHECK(builder.beginString(command));CHECK(builder.word(CaptionAt,scene.tailPre.pixels[CaptionAt],65535,65535));scene.tailPost.pixels[CaptionAt]=65535;CHECK(builder.endString(command));return builder.finish(identity,scene.tailPost.pixels.data(),bg::Side);
 }
 void syncCopy(){
  copy.pre.pixels=base.post.pixels;copy.post.pixels=copy.pre.pixels;
  for(unsigned k=0;k<bg::Side;++k)put16(copy.sourceBefore.data()+16+2*k,scene.tailPost.pixels[k]);copy.sourceAfter=copy.sourceBefore;CHECK(nfs_texture_sha256(copy.sourceBefore.data(),copy.span,copy.shaBefore.data()));copy.shaAfter=copy.shaBefore;
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){uint16_t value=scene.tailPost.pixels[y*343+x];put32(copy.row[y].source.data()+4*x,value?rgba(value):0);copy.post.pixels[y*640+x]=value?value:base.post.pixels[y*640+x];}
 }
};
int main(){try{
 #ifdef NFS_TEST_SETUP_SCENE
 static_assert(testForm==0x4D2790&&testTail==4,"Exact Setup descriptor/tail");
#else
 static_assert(testForm==0x4D3A88&&testTail==3,"Exact Options descriptor/tail");
#endif
 static_assert(testTail==3||testTail==4,"Distinct cached-menu tail");
 auto f=std::unique_ptr<OptionsFixture>(new OptionsFixture);
 opt::JoinedInput input{{f->base,f->scene,f->group,f->copy,{1,3,1},{1,4,1},{1,5,1},{1,6,1},f->copy.post.target,19,23,f->keys},f->caption,5,7};
 auto verify=[&](){return opt::verifyAndPrepare(input,f->workspace,*f->prepared,f->catalog);};
 CHECK(verify()==bg::Failure::None&&f->prepared->valid&&f->prepared->caption==f->caption);
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){const unsigned at=y*640+x,sideAt=y*343+x;uint16_t expected=x>=343?uint16_t(0x2a55):sideAt==OptionsFixture::TransparentAt?uint16_t(0x2a55):sideAt==0?f->frameColor:sideAt==OptionsFixture::CaptionAt?uint16_t(65535):f->sideColor;CHECK(f->prepared->original[at]==expected);}
 auto view=f->prepared->view();CHECK(view.schema==1&&view.scene.tailKind==testTail&&view.scene.serial==19&&view.scene.contentRevision==23&&view.scene.planeWords==307200&&view.caption.logicalWidth==343&&view.caption.logicalHeight==480&&view.caption.vertexCount==textLength*6&&view.caption.font.identity==77&&view.caption.font.alpha==f->font->alpha().data());
 auto repeated=f->prepared->view();CHECK(!std::memcmp(view.caption.font.sha256,repeated.caption.font.sha256,32));
 // Strong font ownership, rather than numeric identity/address, keys the digest.
 auto otherAlpha=f->alpha;otherAlpha[4096]=17;auto otherFont=nfs_soft::FontCatalog::create(f->description,otherAlpha.data(),otherAlpha.size(),77);CHECK(otherFont);auto other=f->packet(otherFont,5);CHECK(other);input.caption=other;CHECK(verify()==bg::Failure::None);auto otherView=f->prepared->view();CHECK(otherView.caption.font.alpha==otherFont->alpha().data()&&std::memcmp(view.caption.font.sha256,otherView.caption.font.sha256,32));input.caption=f->caption;CHECK(verify()==bg::Failure::None);
 unsigned negatives=0;auto reject=[&](){CHECK(verify()!=bg::Failure::None&&!f->prepared->valid&&f->prepared->view().schema==0);++negatives;};
 input.caption.reset();reject();input.caption=f->caption;input.acceptedCaptionSerial++;reject();input.acceptedCaptionSerial--;input.captionLifetime++;reject();input.captionLifetime--;
 input.serial=0;reject();input.serial=19;input.contentRevision=0;reject();input.contentRevision=23;
 f->base.form=bg::MainForm;reject();f->base.form=testForm;f->scene.form=bg::MainForm;reject();f->scene.form=testForm;
 f->scene.tailKind=NFS_BACKGROUND_TAIL_MAIN_SKIP;reject();f->scene.tailKind=testTail;f->scene.tailPredicate++;reject();f->scene.tailPredicate--;f->scene.tailSymbol++;reject();f->scene.tailSymbol--;
 f->scene.tailPost.valid=0;reject();f->scene.tailPost.valid=1;f->scene.tailPre.pixels[OptionsFixture::CaptionAt]^=1;reject();f->scene.tailPre.pixels[OptionsFixture::CaptionAt]^=1;f->scene.tailPost.pixels[OptionsFixture::CaptionAt]^=1;reject();f->scene.tailPost.pixels[OptionsFixture::CaptionAt]^=1;
 f->copy.sourceOwner.owner^=4;reject();f->copy.sourceOwner.owner^=4;f->copy.sourceOwner.x[150]^=2;reject();f->copy.sourceOwner.x[150]^=2;f->copy.sourceOwner.y[150]^=2;reject();f->copy.sourceOwner.y[150]^=2;f->copy.sourceOwner.header[3]=1;reject();f->copy.sourceOwner.header[3]=0;
 f->copy.sourceOwner.ownerWords[8]^=4;reject();f->copy.sourceOwner.ownerWords[8]^=4;f->copy.sourceOwner.ownerState[1]^=4;reject();f->copy.sourceOwner.ownerState[1]^=4;
 // The ambient doublebuffer word may flip; it cannot fabricate a source owner.
 f->copy.sourceOwner.ownerState[0]^=0x10000;CHECK(verify()==bg::Failure::None);f->copy.sourceOwner.ownerState[0]^=0x10000;
 f->copy.sourceBefore[16+2*OptionsFixture::CaptionAt]^=1;reject();f->copy.sourceBefore[16+2*OptionsFixture::CaptionAt]^=1;f->copy.sourceAfter[200]^=1;reject();f->copy.sourceAfter[200]^=1;
 f->copy.args[0]=f->base.post.target.w[8];reject();f->copy.args[0]=f->copy.sourceOwner.w[8];f->copy.row[200].row.args[1]^=2;reject();f->copy.row[200].row.args[1]^=2;f->copy.row[200].source[0]^=1;reject();f->copy.row[200].source[0]^=1;
 f->scene.draws[0].sourceAfter[16]^=1;reject();f->scene.draws[0].sourceAfter[16]^=1;f->scene.draws[0].paletteAfter[1]^=1;reject();f->scene.draws[0].paletteAfter[1]^=1;f->scene.draws[0].rows[0].args[2]--;reject();f->scene.draws[0].rows[0].args[2]++;
 f->scene.draws[1].rows[0].payload[0]^=1;reject();f->scene.draws[1].rows[0].payload[0]^=1;f->scene.draws[1].tableAfter[6]^=4;reject();f->scene.draws[1].tableAfter[6]^=4;
 f->group.post.pixels[0]^=1;reject();f->group.post.pixels[0]^=1;f->group.draw[0].paletteLast[1]^=1;reject();f->group.draw[0].paletteLast[1]^=1;
 f->copy.post.pixels[639]^=1;reject();f->copy.post.pixels[639]^=1;
 // Self-consistent stale full-plane Copy.PRE/POST outside the sidebar still
 // cannot substitute for this current authenticated map.
 const unsigned outside=320*640+500;f->copy.pre.pixels[outside]^=1;f->copy.post.pixels[outside]^=1;reject();f->copy.pre.pixels[outside]^=1;f->copy.post.pixels[outside]^=1;
 // A lazy redraw legitimately starts with old opaque sidebar words on the
 // full target. Independent copy replay overwrites them with current343, and
 // complete map+current343 reconstruction still proves every published word.
 f->copy.pre.pixels[100]^=1;CHECK(verify()==bg::Failure::None&&f->prepared->original==f->copy.post.pixels);f->copy.pre.pixels[100]^=1;
 const unsigned transparent=5*640+20;f->copy.pre.pixels[transparent]^=1;f->copy.post.pixels[transparent]^=1;reject();f->copy.pre.pixels[transparent]^=1;f->copy.post.pixels[transparent]^=1;
 // A coherent343 descriptor cannot also be the simultaneously live full640
 // owner, even if all caption-side records repeat the same forged owner tag.
 {const uint32_t old=f->copy.sourceOwner.owner;const uint32_t full=f->copy.pre.target.owner;f->copy.sourceOwner.owner=full;
 auto setOwner=[&](uint32_t v){f->scene.preSide.target.owner=f->scene.postSide.target.owner=f->scene.postWords.target.owner=f->scene.tailPre.target.owner=f->scene.tailPost.target.owner=f->group.pre.target.owner=f->group.post.target.owner=v;for(auto&d:f->scene.draws)d.before.owner=d.after.owner=v;for(auto&d:f->group.draw)d.before.owner=d.after.owner=v;};
 setOwner(full);reject();f->copy.sourceOwner.owner=old;setOwner(old);}
 input.currentTarget.x[639]^=2;reject();input.currentTarget.x[639]^=2;input.currentTarget.ownerState[3]^=4;reject();input.currentTarget.ownerState[3]^=4;f->base.post.target.ownerWords[8]^=4;reject();f->base.post.target.ownerWords[8]^=4;
 input.tailEvent.sequence=input.copyEvent.sequence;reject();input.tailEvent.sequence=4;input.groupEvent.ownerEpoch++;reject();input.groupEvent.ownerEpoch--;input.copyEvent.iteration++;reject();input.copyEvent.iteration--;input.uploadEvent.sequence=input.copyEvent.sequence;reject();input.uploadEvent.sequence=6;
 f->base.event.iteration=1;reject();f->base.event.iteration=0;f->scene.event.iteration=0;reject();f->scene.event.iteration=1;
 input.rendererAddress[1]=input.rendererAddress[0];reject();input.rendererAddress[1]=2048;input.rendererAddress[1]=1024;reject();input.rendererAddress[1]=2048;input.rendererAddress[1]=0xfffffc00;reject();input.rendererAddress[1]=2048;
 f->base.sourceAfter[16]^=1;reject();f->base.sourceAfter[16]^=1;f->base.post.pixels[0]^=1;reject();f->base.post.pixels[0]^=1;
 // Only this synthetic fixture can replace the trusted catalog. An independently
 // reauthenticated map change hidden by opaque343 leaves the correct published
 // POST unchanged and is allowed; the real runtime catalog is immutable.
 // Even a self-consistently rehashed malformed map header cannot stand in
 // for the fixed original RGB565640x480 map format.
 for(unsigned word=0;word<4;++word){const auto saved=f->catalog.mapSha[0];f->base.sourceBefore[4*word]^=1;f->base.sourceAfter=f->base.sourceBefore;f->catalog.mapSha[0]=digest(f->base.sourceBefore);reject();f->base.sourceBefore[4*word]^=1;f->base.sourceAfter=f->base.sourceBefore;f->catalog.mapSha[0]=saved;}
 // A real one-command caption packet which also alters a logo-union word
 // cannot smuggle that write through its valid full343 replay.
 {nfs_soft::SurfaceIdentity id{};CHECK(opt::captionIdentity(f->copy.sourceOwner,7,id));nfs_soft::FramePacketBuilder b(f->font);CHECK(b.begin(id,5,f->scene.tailPre.pixels.data(),bg::Side));CHECK(b.beginString(f->command));CHECK(b.word(OptionsFixture::CaptionAt,f->scene.tailPre.pixels[OptionsFixture::CaptionAt],65535,65535));CHECK(b.word(0,f->scene.tailPre.pixels[0],123,123));f->scene.tailPost.pixels[0]=123;CHECK(b.endString(f->command));auto overlapping=b.finish(id,f->scene.tailPost.pixels.data(),bg::Side);CHECK(overlapping);input.caption=overlapping;f->syncCopy();reject();f->scene.tailPost.pixels[0]=f->scene.tailPre.pixels[0];input.caption=f->caption;f->syncCopy();}
 const auto oldSha=f->catalog.mapSha[0];f->base.sourceBefore[16]^=2;f->base.sourceAfter=f->base.sourceBefore;f->base.post.pixels[0]^=2;f->catalog.mapSha[0]=digest(f->base.sourceBefore);CHECK(verify()==bg::Failure::None&&f->prepared->original==f->copy.post.pixels);f->base.sourceBefore[16]^=2;f->base.sourceAfter=f->base.sourceBefore;f->base.post.pixels[0]^=2;f->catalog.mapSha[0]=oldSha;
 // A reauthenticated change outside343 is observable, so a self-consistent old
 // Copy.PRE/POST cannot hide it from full307200 current-map reconstruction.
 {const unsigned at=640+500;const auto saved=f->catalog.mapSha[0];f->base.sourceBefore[16+2*at]^=1;f->base.sourceAfter=f->base.sourceBefore;f->base.post.pixels[at]^=1;f->catalog.mapSha[0]=digest(f->base.sourceBefore);reject();f->base.sourceBefore[16+2*at]^=1;f->base.sourceAfter=f->base.sourceBefore;f->base.post.pixels[at]^=1;f->catalog.mapSha[0]=saved;}
 // A separately authenticated second map is allowed with a verified actual
 // copy and matching transparent/outside pixels, not by role tag alone.
 f->base.mapRole=1;f->base.sourceBefore[16]^=1;f->base.sourceAfter=f->base.sourceBefore;f->base.post.pixels[0]^=1;f->syncCopy();CHECK(verify()==bg::Failure::None&&f->prepared->mapRole==1);f->base.mapRole=0;f->base.sourceBefore[16]^=1;f->base.sourceAfter=f->base.sourceBefore;f->base.post.pixels[0]^=1;f->syncCopy();CHECK(verify()==bg::Failure::None);
 auto retained=f->prepared->caption;f->prepared->caption.reset();CHECK(f->prepared->view().schema==0);f->prepared->caption=retained;CHECK(f->prepared->view().schema==1);
 CHECK(f->prepared->captionSerial==5&&f->prepared->captionLifetime==7&&retained->original()[OptionsFixture::CaptionAt]==65535);auto owner=std::shared_ptr<opt::Prepared>(f->prepared.release());const auto savedPixel=owner->original[OptionsFixture::TransparentAt];f.reset();auto lifetimeView=owner->view();CHECK(lifetimeView.schema==1&&lifetimeView.caption.font.alpha&&lifetimeView.scene.original565[OptionsFixture::TransparentAt]==savedPixel);std::printf("PASS cached-menu current full307200 oracle, transparent343 + caption, %u stale/source/order/alias/LUT/current-owner negatives\n",negatives);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
