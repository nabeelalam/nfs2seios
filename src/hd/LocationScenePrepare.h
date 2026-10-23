// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
#include "LocationSceneRuntimeAbi.h"
#include "SoftwareFramePacket.h"
#include "PreviewPacket.h"
#include "LocationCaptionDisplay.h"
namespace nfs_location_scene_cpu {
namespace bg=nfs_background_scene_cpu;
using namespace nfs_background_scene_cpu;
constexpr uint32_t LocationForm=0x404abd;
// Constructed from an independently captured actual Copy owner, never a selected640 target.
inline bool captionIdentity(const Target&t,uint64_t lifetime,nfs_soft::SurfaceIdentity&out)noexcept{
 if(!fullClip(t,343)||t.w[7]!=0x7810||t.w[10]!=686||t.w[13]||!t.owner||t.header[3]||(t.header[0]&127)!=0x78||t.header[1]!=(343u|(480u<<16)))return false;
 for(unsigned k=0;k<343;++k)if(t.x[k]!=2*k)return false;
 for(unsigned k=0;k<480;++k)if(t.y[k]!=16+686*k)return false;
 for(unsigned k:{0u,1u,2u,7u,8u,10u,11u,12u,13u})if(t.ownerWords[k]!=t.w[k])return false;
 out={};out.lifetime=lifetime;std::copy(t.w.begin(),t.w.end(),out.geometry.begin());auto&a=out.geometry;a[14]=t.x[0];a[15]=t.x[1];a[16]=t.x[342];a[17]=t.y[0];a[18]=t.y[1];a[19]=t.y[479];return out.valid();
}
// The selected-double-buffer word is ambient; actual owner/header/full LUT and handles remain exact.
inline bool sameCaptionOwner(const Target&a,const Target&b)noexcept{
 if(a.w!=b.w||a.header!=b.header||a.x!=b.x||a.y!=b.y||a.owner!=b.owner||a.ownerWords!=b.ownerWords)return false;
 for(unsigned k=1;k<5;++k)if(a.ownerState[k]!=b.ownerState[k])return false;return true;
}
struct Prepared: bg::Prepared {
 std::shared_ptr<const nfs_location_caption_display::Prepared>captionWord;
 std::shared_ptr<const nfs_soft::FramePacket>caption;uint64_t captionSerial=0,captionLifetime=0;
 std::shared_ptr<const nfs_location_preview::Packet>preview;uint64_t previewSerial=0,previewLifetime=0;uint32_t previewArchive=0;Event previewBaseEvent{},previewEvent{};
 NfsLocationSceneFrameView view()const noexcept{
  NfsLocationSceneFrameView v{};if(!valid||!caption||!caption->font()||!preview)return v;
  if(captionWord){if(!captionWord->current()||captionWord->packet()!=caption)return v;v.captionWord=captionWord->view();if(!v.captionWord.schema)return {};}
  v.schema=NFS_LOCATION_SCENE_RUNTIME_SCHEMA;v.preview=preview->view();v.scene=bg::Prepared::view();v.scene.tailKind=NFS_BACKGROUND_TAIL_LOCATION_CAPTION;
  const auto&font=*caption->font();auto&f=v.caption.font;f.schema=1;f.width=font.width;f.height=font.height;f.identity=font.identity();f.alpha=font.alpha().data();f.bytes=font.alpha().size();
  // One strongly retained immutable owner per renderer thread prevents address reuse.
  // Cache is bounded and never authorizes by numeric identity or digest alone.
  struct FontDigest {std::shared_ptr<const nfs_soft::FontCatalog>owner;std::array<unsigned char,32>sha{};};
  static thread_local FontDigest cached;
  const auto&owner=caption->font();
  if(cached.owner.get()!=owner.get()||cached.owner.owner_before(owner)||owner.owner_before(cached.owner)){
   std::array<unsigned char,32>digest{};if(!nfs_texture_sha256(f.alpha,f.bytes,digest.data()))return {};cached.owner=owner;cached.sha=digest;
  }
  std::copy(cached.sha.begin(),cached.sha.end(),f.sha256);
  v.caption.logicalWidth=343;v.caption.logicalHeight=480;v.caption.vertices=caption->vertices().data();v.caption.vertexCount=caption->vertices().size();return v;
 }
};
struct JoinedInput: bg::JoinedInput {
 std::shared_ptr<const nfs_location_caption_source::Proof>captionProof;nfs_location_caption_display::Binding captionBinding{};
 std::shared_ptr<const nfs_soft::FramePacket>caption;uint64_t acceptedCaptionSerial=0,captionLifetime=0;
 std::shared_ptr<const nfs_location_preview::Packet>preview;uint64_t acceptedPreviewSerial=0,previewLifetime=0;uint32_t previewArchive=0;Event previewBaseEvent{},previewEvent{};
 JoinedInput(bg::JoinedInput input,std::shared_ptr<const nfs_soft::FramePacket>p,uint64_t serial,uint64_t lifetime,std::shared_ptr<const nfs_location_preview::Packet>originalPreview,uint64_t previewSerial,uint64_t originalLifetime,uint32_t archive,Event originalBaseEvent,Event originalPreviewEvent,std::shared_ptr<const nfs_location_caption_source::Proof>proof={},nfs_location_caption_display::Binding binding={}):bg::JoinedInput(input),caption(std::move(p)),acceptedCaptionSerial(serial),captionLifetime(lifetime),preview(std::move(originalPreview)),acceptedPreviewSerial(previewSerial),previewLifetime(originalLifetime),previewArchive(archive),previewBaseEvent(originalBaseEvent),previewEvent(originalPreviewEvent){captionProof=std::move(proof);captionBinding=binding;}
};
inline Failure verifyAndPrepare(const JoinedInput&i,Workspace&w,Prepared&o,const SourceCatalog&catalog)noexcept{o.valid=false;o.captionWord.reset();o.caption.reset();o.captionSerial=o.captionLifetime=0;o.preview.reset();o.previewSerial=o.previewLifetime=0;o.previewArchive=0;o.previewBaseEvent=o.previewEvent={};o.wordCount=0;o.words={};if(!validCatalog(catalog))return Failure::Source;const auto&b=i.base;const auto&s=i.scene;const auto&g=i.logos;const auto&c=i.copy;
 const Event ev[6]={b.event,s.event,i.groupEvent,i.tailEvent,i.copyEvent,i.uploadEvent};if(!i.serial||!i.contentRevision||!ev[0].ownerEpoch||!ev[0].sequence||ev[0].iteration)return Failure::Binding;for(unsigned k=1;k<6;++k)if(ev[k].ownerEpoch!=ev[0].ownerEpoch||ev[k].sequence<=ev[k-1].sequence||!ev[k].iteration||ev[k].iteration!=ev[1].iteration)return Failure::Binding;

 if(b.mapRole>1||b.form!=LocationForm||b.sourceBefore.size()!=614416||b.sourceBefore!=b.sourceAfter||!nfs_small_logo_live::hashMatches(b.sourceBefore.data(),614416,catalog.mapSha[b.mapRole])||!plane(b.post,640))return Failure::Base;for(unsigned k=0;k<Full;++k)if(b.post.pixels[k]!=le16(b.sourceBefore.data()+16+2*k))return Failure::Base;
 // Preview ownership is supplied independently by the current live owner, not
 // copied from the candidate. Its event bridges this exact base to this scene.
 if(!i.preview||!i.acceptedPreviewSerial||!i.previewLifetime||!i.previewArchive||i.previewLifetime!=b.event.ownerEpoch||i.preview->serial()!=i.acceptedPreviewSerial||i.preview->lifetime()!=i.previewLifetime||i.preview->archive()!=i.previewArchive||i.previewBaseEvent.ownerEpoch!=b.event.ownerEpoch||i.previewBaseEvent.sequence!=b.event.sequence||i.previewBaseEvent.iteration!=b.event.iteration||i.previewEvent.ownerEpoch!=b.event.ownerEpoch||i.previewEvent.iteration||i.previewEvent.sequence<=b.event.sequence||i.previewEvent.sequence>=s.event.sequence)return Failure::Binding;
 if(!i.preview->baseMatches(b.post)||i.preview->post().size()!=Full)return Failure::Base;
 if(s.form!=LocationForm||!s.complete||s.error||s.draws.empty()||s.draws.size()>5||!plane(s.preSide,343)||!nfs_logo_capture::same(s.preSide.target,s.postSide.target)||!nfs_logo_capture::same(s.preSide.target,s.postWords.target))return Failure::Scene;
 auto failure=side(s.draws[0],s.preSide.target,w,catalog);if(failure!=Failure::None)return failure;if(!plane(s.postSide,343)||!std::equal(w.side.begin(),w.side.end(),s.postSide.pixels.begin()))return Failure::Pixels;
 for(unsigned k=1;k<s.draws.size();++k){failure=word(s.draws[k],s.preSide.target,w,o.words[k-1],catalog);if(failure!=Failure::None)return failure;}if(!plane(s.postWords,343)||!std::equal(w.side.begin(),w.side.end(),s.postWords.pixels.begin())||!samePlane(s.postWords,g.pre,343))return Failure::Pixels;
 if(nfs_small_logo_live::verifyAndPrepare(g,c,w.logo,o.logoScratch,catalog.logos)!=nfs_small_logo_live::Failure::None)return Failure::Logos;
 if(s.tailKind!=NFS_BACKGROUND_TAIL_LOCATION_CAPTION||s.tailPredicate!=LocationForm||s.tailSymbol!=LocationForm||!samePlane(s.tailPre,g.post,343))return Failure::Tail;
 if(!plane(s.tailPost,343)||!i.caption||!i.acceptedCaptionSerial||!i.captionLifetime||i.caption->commands()!=1||!i.caption->writes()||i.caption->vertices().empty()||!i.caption->font()||i.caption->pixels()!=Side)return Failure::Tail;
 nfs_soft::SurfaceIdentity identity{};if(!captionIdentity(c.sourceOwner,i.captionLifetime,identity)||!sameCaptionOwner(c.sourceOwner,s.tailPost.target)||!i.caption->matches(identity,i.acceptedCaptionSerial,s.tailPost.pixels.data(),Side)||!std::equal(i.caption->clean().begin(),i.caption->clean().end(),s.tailPre.pixels.begin()))return Failure::Tail;
 // The immutable logo verifier owns its exact union; caption may change only outside it.
 for(unsigned k=0;k<Side;++k)if(w.logo.groupUnion[k]&&s.tailPre.pixels[k]!=s.tailPost.pixels[k])return Failure::Tail;
 if(c.sourceOwner.header[3]||!retainedBaseTarget(b.post.target,c.pre.target)||!nfs_logo_capture::same(i.currentTarget,c.post.target))return Failure::Target;for(unsigned k=0;k<Side;++k)if(le16(c.sourceBefore.data()+16+2*k)!=s.tailPost.pixels[k])return Failure::Copy;
 // Reconstruct complete CURRENT native source from authenticated map plus
 // independently replayed preview, never from old Copy.PRE.
 std::copy(i.preview->post().begin(),i.preview->post().end(),w.whole.begin());const uint32_t key=c.row[0].colorKey;for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){unsigned at=y*640+x;uint16_t sourceWord=le16(c.sourceBefore.data()+16+2*(y*343+x));w.whole[at]=nfs_small_logo_live::pixel(w.whole[at],nfs_small_logo_live::expanded(sourceWord,key),false);}if(!std::equal(w.whole.begin(),w.whole.end(),c.post.pixels.begin()))return Failure::Pixels;
 for(unsigned k=0;k<300;++k){uint32_t keyAddress=i.rendererAddress[k];if(uint64_t(keyAddress)+2048>0x100000000ULL)return Failure::NativeKeys;for(unsigned j=0;j<k;++j)if(uint64_t(keyAddress)<uint64_t(i.rendererAddress[j])+2048&&uint64_t(i.rendererAddress[j])<uint64_t(keyAddress)+2048)return Failure::NativeKeys;}
 if(i.captionProof||!i.captionBinding.empty()){
  if(!i.captionProof)return Failure::Tail;const auto&e=i.captionProof->evidence();
  if(i.captionBinding.form!=s.form||i.captionBinding.archive!=i.previewArchive||i.captionBinding.serial!=i.acceptedCaptionSerial||i.captionBinding.lifetime!=i.captionLifetime||!sameCaptionOwner(e.post.target,c.sourceOwner))return Failure::Tail;
  o.captionWord=nfs_location_caption_display::Prepared::create(i.captionProof,i.caption,i.captionBinding);if(!o.captionWord)return Failure::Tail;
 }
 o.preview=i.preview;o.previewSerial=i.acceptedPreviewSerial;o.previewLifetime=i.previewLifetime;o.previewArchive=i.previewArchive;o.previewBaseEvent=i.previewBaseEvent;o.previewEvent=i.previewEvent;o.caption=i.caption;o.captionSerial=i.acceptedCaptionSerial;o.captionLifetime=i.captionLifetime;o.mapRole=b.mapRole;o.wordCount=unsigned(s.draws.size()-1);o.serial=i.serial;o.contentRevision=i.contentRevision;o.original=c.post.pixels;o.keys=i.rendererAddress;o.logos=o.logoScratch.layers;o.valid=true;return Failure::None;
}
} // namespace nfs_location_scene_cpu
