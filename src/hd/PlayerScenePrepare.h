// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
#include "PlayerSceneRuntimeAbi.h"
#include "SoftwareFramePacket.h"
#include "PlayerPreviewPacket.h"
#include "PlayerSwatchPacket.h"
#include <new>
namespace nfs_player_scene_cpu {
namespace bg=nfs_background_scene_cpu;namespace pv=nfs_player_preview;namespace sw=nfs_player_swatch;
using namespace nfs_background_scene_cpu;
constexpr uint32_t Player1Form=pv::Player1Form,Player2Form=pv::Player2Form;
inline unsigned tail(uint32_t form)noexcept{return form==Player1Form?NFS_BACKGROUND_TAIL_PLAYER1_CAPTION:form==Player2Form?NFS_BACKGROUND_TAIL_PLAYER2_CAPTION:0;}
inline bool sameCatalog(const SourceCatalog&a,const SourceCatalog&b)noexcept{
 if(a.roles!=b.roles||a.mapSha!=b.mapSha||a.mapRoles!=b.mapRoles||a.logos.roles!=b.logos.roles)return false;
 for(unsigned k=0;k<13;++k){const auto&x=a.identities[k];const auto&y=b.identities[k];if(x.span!=y.span||x.width!=y.width||x.height!=y.height||x.paletteOffset!=y.paletteOffset||x.header!=y.header||x.sha!=y.sha)return false;}
 for(unsigned k=0;k<5;++k){const auto&x=a.logos.identities[k];const auto&y=b.logos.identities[k];if(x.span!=y.span||x.width!=y.width||x.height!=y.height||x.paletteOffset!=y.paletteOffset||x.paletteCount!=y.paletteCount||x.maxIndex!=y.maxIndex||x.header!=y.header||x.sha!=y.sha)return false;}return true;
}

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

struct OriginalOverlay {
 int32_t left=0,top=0;uint32_t width=0,height=0;
 std::vector<uint16_t>pixels;std::vector<uint8_t>coverage;
 std::array<unsigned char,32>sha{},coverageSha{};
 NfsOriginalPreviewLayerView view()const noexcept{NfsOriginalPreviewLayerView v{};v.schema=NFS_ORIGINAL_PREVIEW_LAYER_SCHEMA;v.left=left;v.top=top;v.width=width;v.height=height;v.pixels565=pixels.data();v.pixelCount=pixels.size();v.coverage=coverage.data();v.coverageCount=coverage.size();std::copy(sha.begin(),sha.end(),v.sha256);std::copy(coverageSha.begin(),coverageSha.end(),v.coverageSha256);return v;}
};
struct Prepared:bg::Prepared {
 uint32_t form=0;std::shared_ptr<const nfs_soft::FramePacket>caption;uint64_t captionSerial=0,captionLifetime=0;
 std::shared_ptr<const pv::Packet>preview;std::shared_ptr<const sw::Packet>swatch;std::shared_ptr<const OriginalOverlay>overlay;
 uint64_t previewSerial=0,previewLifetime=0,swatchCopySerial=0;uint32_t previewArchive=0;
 Event previewBaseEvent{},previewEvent{},swatchCopyEvent{},swatchEvent{};
 NfsPlayerSceneFrameView view()const noexcept{
  NfsPlayerSceneFrameView v{};if(!valid||!tail(form)||!caption||!caption->font()||!preview||!overlay)return v;
  v.schema=NFS_PLAYER_SCENE_RUNTIME_SCHEMA;v.form=form;v.scene=bg::Prepared::view();v.scene.tailKind=tail(form);v.preview=overlay->view();
  const auto&font=*caption->font();auto&f=v.caption.font;f.schema=1;f.width=font.width;f.height=font.height;f.identity=font.identity();f.alpha=font.alpha().data();f.bytes=font.alpha().size();
  struct FontDigest{std::shared_ptr<const nfs_soft::FontCatalog>owner;std::array<unsigned char,32>sha{};};static thread_local FontDigest cached;const auto&owner=caption->font();
  if(cached.owner.get()!=owner.get()||cached.owner.owner_before(owner)||owner.owner_before(cached.owner)){std::array<unsigned char,32>digest{};if(!nfs_texture_sha256(f.alpha,f.bytes,digest.data()))return {};cached.owner=owner;cached.sha=digest;}std::copy(cached.sha.begin(),cached.sha.end(),f.sha256);
  v.caption.logicalWidth=343;v.caption.logicalHeight=480;v.caption.vertices=caption->vertices().data();v.caption.vertexCount=caption->vertices().size();return v;
 }
};

enum class SourceKind:uint32_t{Initial,Refresh,Lazy};
struct SourceBinding {
 uint32_t form=0,formGuest=0,sourceOwner=0,archive=0;uintptr_t gameOwner=0,thread=0;
 uint64_t epoch=0,lifetime=0,generation=0,contentSerial=0,copySerial=0;
 SourceKind kind=SourceKind::Initial;
};
inline bool sameBinding(const SourceBinding&a,const SourceBinding&b)noexcept{return a.form==b.form&&a.formGuest==b.formGuest&&a.sourceOwner==b.sourceOwner&&a.archive==b.archive&&a.gameOwner==b.gameOwner&&a.thread==b.thread&&a.epoch==b.epoch&&a.lifetime==b.lifetime&&a.generation==b.generation&&a.contentSerial==b.contentSerial&&a.copySerial==b.copySerial&&a.kind==b.kind;}
inline bool validBinding(const SourceBinding&b)noexcept{return tail(b.form)&&b.formGuest&&b.sourceOwner==b.formGuest&&b.archive&&b.gameOwner&&b.thread&&b.epoch&&b.lifetime==b.epoch&&b.generation&&b.contentSerial&&b.copySerial&&(b.kind==SourceKind::Initial||b.kind==SourceKind::Refresh||b.kind==SourceKind::Lazy);}
// formGuest/sourceOwner are independently relocated exact form descriptors,
// distinct from gameOwner (host Game occurrence) and actual target.owner.
inline bool sameEvent(const Event&a,const Event&b)noexcept{return a.ownerEpoch==b.ownerEpoch&&a.sequence==b.sequence&&a.iteration==b.iteration;}
struct SidebarInput {
 const Scene&scene;const nfs_logo_capture::Group&logos;const nfs_logo_capture::Copy&originalCopy;
 std::shared_ptr<const nfs_soft::FramePacket>caption;uint64_t serial=0,captionSerial=0,captionLifetime=0;
 Event groupEvent{},tailEvent{};
};
class SidebarProof;
inline std::shared_ptr<const SidebarProof>verifySidebar(const SidebarInput&,Workspace&,const SourceCatalog&,Failure*failure=nullptr)noexcept;
class SidebarProof final {
 SourceCatalog catalog_;nfs_logo_capture::Group group_;Plane post_;std::shared_ptr<const nfs_soft::FramePacket>caption_;
 uint64_t serial_=0,captionSerial_=0,lifetime_=0;uint32_t form_=0;Event sceneEvent_{},groupEvent_{},tailEvent_{};
 unsigned wordCount_=0;std::array<NfsBackgroundSceneWordLayer,4>words_{};std::array<NfsSmallLogoLayer,3>logos_{};
 SidebarProof()=default;friend std::shared_ptr<const SidebarProof>verifySidebar(const SidebarInput&,Workspace&,const SourceCatalog&,Failure*)noexcept;
 friend class VerifiedSource;
public:
 bool catalogMatches(const SourceCatalog&c)const noexcept{return sameCatalog(catalog_,c);}const nfs_logo_capture::Group&group()const noexcept{return group_;}const Plane&post()const noexcept{return post_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&caption()const noexcept{return caption_;}
 uint64_t serial()const noexcept{return serial_;}uint64_t captionSerial()const noexcept{return captionSerial_;}uint64_t lifetime()const noexcept{return lifetime_;}uint32_t form()const noexcept{return form_;}
 Event sceneEvent()const noexcept{return sceneEvent_;}Event groupEvent()const noexcept{return groupEvent_;}Event tailEvent()const noexcept{return tailEvent_;}
 unsigned wordCount()const noexcept{return wordCount_;}const std::array<NfsBackgroundSceneWordLayer,4>&words()const noexcept{return words_;}const std::array<NfsSmallLogoLayer,3>&logos()const noexcept{return logos_;}
};
struct JoinedInput {
 const Base&base;const nfs_logo_capture::Copy&copy;SourceBinding binding;
 std::shared_ptr<const SidebarProof>sidebar;uint64_t acceptedSidebarSerial=0,sidebarLifetime=0;const Plane*actualSidebarPost=nullptr;Event sidebarValidationEvent{};
 std::shared_ptr<const pv::Packet>preview;uint64_t acceptedPreviewSerial=0,previewLifetime=0;uint32_t previewArchive=0;Event previewBaseEvent{},previewEvent{};const Plane*actualPreviewPost=nullptr;
 Event copyEvent{},uploadEvent{};const Plane*actualUploadPost=nullptr;
 std::shared_ptr<const sw::Packet>swatch;sw::Authority swatchAuthority;uint64_t acceptedSwatchCopySerial=0;Event swatchCopyEvent{},swatchEvent{};const Plane*swatchCopyPost=nullptr;const Plane*actualSwatchPost=nullptr;
 JoinedInput(const Base&b,const nfs_logo_capture::Copy&c):base(b),copy(c){}
};
class VerifiedSource;
inline std::shared_ptr<const VerifiedSource>verifySource(const JoinedInput&,Workspace&,const SourceCatalog&,Failure*failure=nullptr)noexcept;
class VerifiedSource final {
 Prepared data_;SourceBinding binding_;Target target_;
 VerifiedSource()=default;friend std::shared_ptr<const VerifiedSource>verifySource(const JoinedInput&,Workspace&,const SourceCatalog&,Failure*)noexcept;
 friend bool untrustedPreparedLayout(const std::shared_ptr<const VerifiedSource>&,const std::array<uint32_t,300>&,Prepared&)noexcept;
public:
 const SourceBinding&binding()const noexcept{return binding_;}const Target&target()const noexcept{return target_;}
 const std::array<uint16_t,Full>&original()const noexcept{return data_.original;}
 uint32_t form()const noexcept{return binding_.form;}uint32_t player()const noexcept{return binding_.form==Player1Form?1:2;}
};
// Pure layout conversion ONLY. Raw keys confer no native upload/presentation
// authority. Queue admission must consume the separate sealed native proof
// holding this exact shared source and independently CURRENT global event.
inline bool untrustedPreparedLayout(const std::shared_ptr<const VerifiedSource>&s,const std::array<uint32_t,300>&keys,Prepared&o)noexcept{if(!s)return false;for(unsigned k=0;k<300;++k){if(uint64_t(keys[k])+2048>0x100000000ULL)return false;for(unsigned j=0;j<k;++j)if(uint64_t(keys[k])<uint64_t(keys[j])+2048&&uint64_t(keys[j])<uint64_t(keys[k])+2048)return false;}try{o=s->data_;o.keys=keys;return o.valid;}catch(...){o.valid=false;return false;}}
inline std::shared_ptr<const OriginalOverlay>makeOverlay(const pv::Packet&preview,const sw::Packet*swatch,const nfs_logo_capture::Copy&copy){
 std::vector<uint16_t>whole(Full);std::vector<uint8_t>coverage(Full);auto add=[&](unsigned left,unsigned top,unsigned width,unsigned height,const std::vector<uint16_t>&pixels,const std::vector<uint8_t>&mask){if(!width||!height||uint64_t(left)+width>640||uint64_t(top)+height>480||pixels.size()!=size_t(width)*height||mask.size()!=pixels.size())return false;for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x){size_t from=y*width+x,to=(top+y)*640+left+x;if(mask[from]){whole[to]=pixels[from];coverage[to]=255;}}return true;};
 const auto&b=preview.bounds();if(!add(b.left,b.top,b.width,b.height,preview.pixels(),preview.coverage()))return {};
 // Original343 opaque words cover preview. This union layer is drawn after
 // authored sidebar layers, so retain only actually exposed preview coverage.
 const uint32_t key=copy.row[0].colorKey;for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x)if(le16(copy.sourceBefore.data()+16+2*(y*343+x))!=key)coverage[y*640+x]=0;
 if(swatch){const auto s=swatch->bounds();if(!add(s.left,s.top,s.width,s.height,swatch->pixels(),swatch->coverage()))return {};}
 unsigned left=640,top=480,right=0,bottom=0;for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)if(coverage[y*640+x]){left=std::min(left,x);top=std::min(top,y);right=std::max(right,x+1);bottom=std::max(bottom,y+1);}
 if(left>=right||top>=bottom)return {};std::shared_ptr<OriginalOverlay>out(new OriginalOverlay);out->left=int32_t(left);out->top=int32_t(top);out->width=right-left;out->height=bottom-top;for(unsigned y=top;y<bottom;++y)for(unsigned x=left;x<right;++x){out->pixels.push_back(whole[y*640+x]);out->coverage.push_back(coverage[y*640+x]);}if(!pv::wordsSha(out->pixels.data(),out->pixels.size(),out->sha)||!nfs_texture_sha256(out->coverage.data(),out->coverage.size(),out->coverageSha.data()))return {};return out;
}
inline std::shared_ptr<const SidebarProof>verifySidebar(const SidebarInput&i,Workspace&w,const SourceCatalog&catalog,Failure*failure)noexcept{
 auto reject=[&](Failure f)->std::shared_ptr<const SidebarProof>{if(failure)*failure=f;return {};};if(failure)*failure=Failure::None;
 if(!validCatalog(catalog))return reject(Failure::Source);const auto&s=i.scene;const auto&g=i.logos;const auto&c=i.originalCopy;const uint32_t form=s.form;const unsigned expectedTail=tail(form);std::unique_ptr<bg::Prepared>scratch(new(std::nothrow)bg::Prepared);if(!scratch)return reject(Failure::Source);auto&o=*scratch;
 if(!expectedTail||!i.serial||!s.event.ownerEpoch||!s.event.sequence||!s.event.iteration||i.groupEvent.ownerEpoch!=s.event.ownerEpoch||i.tailEvent.ownerEpoch!=s.event.ownerEpoch||i.groupEvent.iteration!=s.event.iteration||i.tailEvent.iteration!=s.event.iteration||s.event.sequence>=i.groupEvent.sequence||i.groupEvent.sequence>=i.tailEvent.sequence)return reject(Failure::Binding);
 if(s.form!=form||!s.complete||s.error||s.draws.empty()||s.draws.size()>5||!plane(s.preSide,343)||!nfs_logo_capture::same(s.preSide.target,s.postSide.target)||!nfs_logo_capture::same(s.preSide.target,s.postWords.target))return reject(Failure::Scene);
 auto componentFailure=side(s.draws[0],s.preSide.target,w,catalog);if(componentFailure!=Failure::None)return reject(componentFailure);if(!plane(s.postSide,343)||!std::equal(w.side.begin(),w.side.end(),s.postSide.pixels.begin()))return reject(Failure::Pixels);
 for(unsigned k=1;k<s.draws.size();++k){componentFailure=word(s.draws[k],s.preSide.target,w,o.words[k-1],catalog);if(componentFailure!=Failure::None)return reject(componentFailure);}if(!plane(s.postWords,343)||!std::equal(w.side.begin(),w.side.end(),s.postWords.pixels.begin())||!samePlane(s.postWords,g.pre,343))return reject(Failure::Pixels);
 if(nfs_small_logo_live::verifyAndPrepare(g,c,w.logo,o.logoScratch,catalog.logos)!=nfs_small_logo_live::Failure::None)return reject(Failure::Logos);
 if(s.tailKind!=expectedTail||s.tailPredicate!=form||s.tailSymbol!=form||!samePlane(s.tailPre,g.post,343))return reject(Failure::Tail);
 if(!plane(s.tailPost,343)||!i.caption||!i.captionSerial||!i.captionLifetime||i.caption->commands()!=1||!i.caption->writes()||i.caption->vertices().empty()||!i.caption->font()||i.caption->pixels()!=Side)return reject(Failure::Tail);
 nfs_soft::SurfaceIdentity identity{};if(!captionIdentity(c.sourceOwner,i.captionLifetime,identity)||!sameCaptionOwner(c.sourceOwner,s.tailPost.target)||!i.caption->matches(identity,i.captionSerial,s.tailPost.pixels.data(),Side)||!std::equal(i.caption->clean().begin(),i.caption->clean().end(),s.tailPre.pixels.begin()))return reject(Failure::Tail);
 // The immutable logo verifier owns its exact union; caption may change only outside it.
 for(unsigned k=0;k<Side;++k)if(w.logo.groupUnion[k]&&s.tailPre.pixels[k]!=s.tailPost.pixels[k])return reject(Failure::Tail);

 if(c.sourceOwner.header[3]||!sameCaptionOwner(c.sourceOwner,s.tailPost.target))return reject(Failure::Target);for(unsigned k=0;k<Side;++k)if(le16(c.sourceBefore.data()+16+2*k)!=s.tailPost.pixels[k])return reject(Failure::Copy);
 try{std::shared_ptr<SidebarProof>p(new SidebarProof);p->catalog_=catalog;p->group_=g;p->post_=s.tailPost;p->caption_=i.caption;p->serial_=i.serial;p->captionSerial_=i.captionSerial;p->lifetime_=i.captionLifetime;p->form_=form;p->sceneEvent_=s.event;p->groupEvent_=i.groupEvent;p->tailEvent_=i.tailEvent;p->wordCount_=unsigned(s.draws.size()-1);p->words_=o.words;p->logos_=o.logoScratch.layers;return p;}catch(...){return reject(Failure::Source);}
}
inline std::shared_ptr<const VerifiedSource>verifySource(const JoinedInput&i,Workspace&w,const SourceCatalog&catalog,Failure*failure)noexcept{
 auto reject=[&](Failure f)->std::shared_ptr<const VerifiedSource>{if(failure)*failure=f;return {};};if(failure)*failure=Failure::None;
 const auto&b=i.base;const auto&c=i.copy;const auto&a=i.binding;const uint32_t form=a.form;
 if(!validBinding(a)||!validCatalog(catalog)||b.form!=form||b.event.ownerEpoch!=a.epoch||b.event.iteration||!b.event.sequence||c.serial!=a.copySerial)return reject(Failure::Binding);
 // Direct Initial/Refresh copies occur outside40BBE0 (iteration0); only
 // genuine Lazy copies carry a nonzero lazy iteration. No synthetic group.
 if((a.kind==SourceKind::Initial&&(i.swatch||i.copyEvent.iteration))||
    (a.kind==SourceKind::Refresh&&(!i.swatch||i.copyEvent.iteration))||
    (a.kind==SourceKind::Lazy&&!i.copyEvent.iteration))return reject(Failure::Binding);
 if(b.mapRole>1||b.sourceBefore.size()!=614416||b.sourceBefore!=b.sourceAfter||!nfs_small_logo_live::hashMatches(b.sourceBefore.data(),614416,catalog.mapSha[b.mapRole])||!plane(b.post,640)||le32(b.sourceBefore.data())!=0x78||le32(b.sourceBefore.data()+4)!=(640u|(480u<<16))||le32(b.sourceBefore.data()+8)||le32(b.sourceBefore.data()+12))return reject(Failure::Base);
 for(unsigned k=0;k<Full;++k)if(b.post.pixels[k]!=le16(b.sourceBefore.data()+16+2*k))return reject(Failure::Base);
 if(!i.preview||!i.acceptedPreviewSerial||i.previewLifetime!=a.lifetime||i.previewArchive!=a.archive||i.preview->form()!=form||!sameEvent(i.previewBaseEvent,b.event)||i.previewEvent.ownerEpoch!=a.epoch||i.previewEvent.iteration||i.previewEvent.sequence<=b.event.sequence||!i.actualPreviewPost||!pv::planeValid(*i.actualPreviewPost)||!i.preview->baseMatches(b.post)||!i.preview->matches(i.actualPreviewPost->target,i.acceptedPreviewSerial,a.lifetime,a.archive,i.actualPreviewPost->pixels.data(),Full))return reject(Failure::Binding);
 const auto&ve=i.sidebarValidationEvent;const auto&ce=i.copyEvent;const auto&ue=i.uploadEvent;
 if(!i.sidebar||!i.sidebar->catalogMatches(catalog)||i.sidebar->form()!=form||i.sidebar->serial()!=i.acceptedSidebarSerial||!i.acceptedSidebarSerial||i.sidebar->lifetime()!=i.sidebarLifetime||!i.sidebarLifetime||i.sidebar->sceneEvent().ownerEpoch!=a.epoch||i.sidebar->tailEvent().sequence>=ve.sequence||ve.ownerEpoch!=a.epoch||ce.ownerEpoch!=a.epoch||ue.ownerEpoch!=a.epoch||ve.iteration!=ce.iteration||ue.iteration!=ce.iteration||ve.sequence<=i.previewEvent.sequence||ve.sequence>=ce.sequence||ce.sequence>=ue.sequence||!i.actualSidebarPost||!plane(*i.actualSidebarPost,343)||!sameCaptionOwner(i.sidebar->post().target,i.actualSidebarPost->target)||i.sidebar->post().pixels!=i.actualSidebarPost->pixels)return reject(Failure::Binding);
 if(!sameCaptionOwner(c.sourceOwner,i.actualSidebarPost->target)||c.sourceOwner.owner==c.pre.target.owner||c.sourceOwner.ownerState[3]!=c.pre.target.owner||!retainedBaseTarget(b.post.target,c.pre.target)||!i.actualUploadPost||!pv::planeValid(*i.actualUploadPost)||!retainedBaseTarget(b.post.target,i.actualUploadPost->target))return reject(Failure::Target);
 for(unsigned k=0;k<Side;++k)if(le16(c.sourceBefore.data()+16+2*k)!=i.actualSidebarPost->pixels[k])return reject(Failure::Copy);
 std::unique_ptr<nfs_small_logo_live::Prepared>copyProof(new(std::nothrow)nfs_small_logo_live::Prepared);if(!copyProof)return reject(Failure::Source);if(nfs_small_logo_live::verifyAndPrepare(i.sidebar->group(),c,w.logo,*copyProof,catalog.logos)!=nfs_small_logo_live::Failure::None)return reject(Failure::Logos);
 if(i.swatch){const auto&sa=i.swatchAuthority;const auto&se=i.swatchEvent;const auto&sc=i.swatchCopyEvent;
  if(!sw::valid(sa)||!i.acceptedSwatchCopySerial||sa.binding.origin!=sw::Origin::Refresh||sa.binding.form!=form||sa.binding.owner!=a.formGuest||sa.binding.epoch!=a.epoch||sa.binding.lifetime!=a.lifetime||sa.binding.archive!=a.archive||sa.binding.car!=i.preview->selection().selectedCar||sc.ownerEpoch!=a.epoch||sc.iteration||se.iteration||sc.sequence<=i.previewEvent.sequence||sc.sequence>ce.sequence||sa.events.enter<=sc.sequence||se.ownerEpoch!=a.epoch||se.sequence!=sa.events.exit||se.sequence>=ue.sequence||!i.swatchCopyPost||!i.actualSwatchPost||!sw::plane(*i.swatchCopyPost)||!sw::plane(*i.actualSwatchPost)||!retainedBaseTarget(b.post.target,i.swatchCopyPost->target)||!retainedBaseTarget(b.post.target,i.actualSwatchPost->target)||!i.swatch->preMatches(*i.swatchCopyPost)||!i.swatch->matches(sa,i.actualSwatchPost->target,i.actualSwatchPost->pixels.data(),Full))return reject(Failure::Binding);
  // A Refresh is bound to this exact original copy. Lazy may retain only
  // an earlier Refresh from a different original copy, completed before it.
  if(a.kind==SourceKind::Refresh){if(!sameEvent(sc,ce)||i.acceptedSwatchCopySerial!=c.serial)return reject(Failure::Binding);}
  else if(a.kind==SourceKind::Lazy){if(sc.sequence>=ce.sequence||se.sequence>=ce.sequence||i.acceptedSwatchCopySerial==c.serial)return reject(Failure::Binding);}
  else return reject(Failure::Binding);
  if(sc.sequence==ce.sequence){if(!sameEvent(sc,ce)||i.acceptedSwatchCopySerial!=c.serial||i.swatchCopyPost->pixels!=c.post.pixels||!nfs_logo_capture::same(i.swatchCopyPost->target,c.post.target))return reject(Failure::Binding);}else if(se.sequence>=ce.sequence)return reject(Failure::Binding);
 }else if(i.actualSwatchPost||i.swatchCopyPost||i.acceptedSwatchCopySerial||i.swatchEvent.ownerEpoch||i.swatchEvent.sequence||i.swatchEvent.iteration||i.swatchCopyEvent.ownerEpoch||i.swatchCopyEvent.sequence||i.swatchCopyEvent.iteration)return reject(Failure::Binding);
 std::copy(i.preview->post().begin(),i.preview->post().end(),w.whole.begin());const uint32_t key=c.row[0].colorKey;for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){unsigned at=y*640+x;w.whole[at]=nfs_small_logo_live::pixel(w.whole[at],nfs_small_logo_live::expanded(le16(c.sourceBefore.data()+16+2*(y*343+x)),key),false);}
 if(i.swatch){const auto r=i.swatch->bounds();if(r.left!=366||r.top!=141||r.width!=85||r.height!=22||i.swatch->pixels().size()!=85*22||i.swatch->coverage().size()!=85*22)return reject(Failure::Pixels);for(unsigned y=0;y<22;++y)for(unsigned x=0;x<85;++x){unsigned k=y*85+x;if(i.swatch->coverage()[k])w.whole[(y+141)*640+x+366]=i.swatch->pixels()[k];}}
 if(!std::equal(w.whole.begin(),w.whole.end(),i.actualUploadPost->pixels.begin()))return reject(Failure::Pixels);
 try{std::shared_ptr<VerifiedSource>source(new VerifiedSource);auto&o=source->data_;o.overlay=makeOverlay(*i.preview,i.swatch.get(),c);if(!o.overlay)return reject(Failure::Pixels);o.form=form;o.caption=i.sidebar->caption();o.captionSerial=i.sidebar->captionSerial();o.captionLifetime=i.sidebarLifetime;o.preview=i.preview;o.previewSerial=i.acceptedPreviewSerial;o.previewLifetime=a.lifetime;o.previewArchive=a.archive;o.previewBaseEvent=b.event;o.previewEvent=i.previewEvent;o.swatch=i.swatch;o.swatchCopySerial=i.acceptedSwatchCopySerial;o.swatchCopyEvent=i.swatchCopyEvent;o.swatchEvent=i.swatchEvent;o.mapRole=b.mapRole;o.wordCount=i.sidebar->wordCount();o.words=i.sidebar->words();o.logos=i.sidebar->logos();o.serial=a.generation;o.contentRevision=a.contentSerial;o.original=i.actualUploadPost->pixels;o.valid=true;source->binding_=a;source->target_=i.actualUploadPost->target;return source;}catch(...){return reject(Failure::Source);}
}
} // namespace nfs_player_scene_cpu
