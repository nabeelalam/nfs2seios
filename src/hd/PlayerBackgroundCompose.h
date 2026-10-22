// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
#include "PlayerPreviewPacket.h"
#include "PlayerSwatchPacket.h"
namespace nfs_player_background {
namespace bg=nfs_background_scene_cpu;namespace pv=nfs_player_preview;namespace sw=nfs_player_swatch;
using nfs_logo_capture::Plane;
// Pure composition only. These must be independently acquired CURRENT source
// witnesses. The caller must already authenticate343 caption/logo/copy replay,
// original copy converter/key and upload generation; numeric tags grant none
// of that authority. No guest reads, arbitrary fullCopyPRE or GPU publication.
// This bounded composer currently accepts full-clip swatches only. It does not
// retain prior partial/no-write overlay history. Missing inherited pixels must
// fail the full-plane comparison; never substitute a submitted Copy.PRE.
struct Authority {
 uint32_t form=0,archive=0,sourceOwner=0;uint64_t epoch=0,lifetime=0,acceptedPreviewSerial=0;
 bg::Event baseEvent{},previewEvent{},sidebarCopyEvent{},uploadEvent{};
 bool hasSwatch=false;sw::Authority currentSwatch;bg::Event swatchEvent{};
};
struct Input {
 const bg::Base&map;std::shared_ptr<const pv::Packet>preview;
 const Plane&actualPreviewPost;const std::vector<uint16_t>&sidebar343;uint32_t copyColorKey=0;
 const Plane&currentUploadPost;std::shared_ptr<const sw::Packet>swatch;const Plane*actualSwatchPost=nullptr;
};
enum class Failure{None,Authority,Map,Preview,Sidebar,Swatch,Target,Pixels,Allocation};
inline bool event(const bg::Event&e,uint64_t epoch)noexcept{return e.ownerEpoch==epoch&&e.sequence;}
inline bool sameEvent(const bg::Event&a,const bg::Event&b)noexcept{return a.ownerEpoch==b.ownerEpoch&&a.sequence==b.sequence&&a.iteration==b.iteration;}
class Packet;
inline std::shared_ptr<const Packet>compose(const Input&,const Authority&,const std::array<unsigned char,32>&independentlyTrustedMapSha,Failure*failure=nullptr)noexcept;
class Packet final {
 std::shared_ptr<const pv::Packet>preview_;std::shared_ptr<const sw::Packet>swatch_;std::vector<uint16_t>original_;Authority authority_;
 Packet()=default;friend std::shared_ptr<const Packet>compose(const Input&,const Authority&,const std::array<unsigned char,32>&,Failure*)noexcept;
public:
 const std::vector<uint16_t>&original()const noexcept{return original_;}
 const std::shared_ptr<const pv::Packet>&preview()const noexcept{return preview_;}
 const std::shared_ptr<const sw::Packet>&swatch()const noexcept{return swatch_;}
 const Authority&authority()const noexcept{return authority_;}
};
inline std::shared_ptr<const Packet>compose(const Input&i,const Authority&a,const std::array<unsigned char,32>&trusted,Failure*failure)noexcept{
 auto reject=[&](Failure f)->std::shared_ptr<const Packet>{if(failure)*failure=f;return {};};if(failure)*failure=Failure::None;
 if((a.form!=pv::Player1Form&&a.form!=pv::Player2Form)||!a.epoch||!a.lifetime||!a.archive||!a.sourceOwner||!a.acceptedPreviewSerial)return reject(Failure::Authority);
 const auto&b=a.baseEvent;const auto&p=a.previewEvent;const auto&c=a.sidebarCopyEvent;const auto&u=a.uploadEvent;
 if(!event(b,a.epoch)||!event(p,a.epoch)||!event(c,a.epoch)||!event(u,a.epoch)||b.iteration||p.iteration||!c.iteration||u.iteration!=c.iteration||!(b.sequence<p.sequence&&p.sequence<c.sequence&&c.sequence<u.sequence)||!sameEvent(i.map.event,b))return reject(Failure::Authority);
 const auto&m=i.map;if(m.form!=a.form||m.mapRole>1||m.sourceBefore.size()!=614416||m.sourceBefore!=m.sourceAfter||!nfs_small_logo_live::hashMatches(m.sourceBefore.data(),614416,trusted)||!pv::planeValid(m.post))return reject(Failure::Map);
 if(bg::le32(m.sourceBefore.data())!=0x78||bg::le32(m.sourceBefore.data()+4)!=(640u|(480u<<16))||bg::le32(m.sourceBefore.data()+8)||bg::le32(m.sourceBefore.data()+12))return reject(Failure::Map);
 for(unsigned k=0;k<pv::Full;++k)if(m.post.pixels[k]!=bg::le16(m.sourceBefore.data()+16+2*k))return reject(Failure::Map);
 if(!i.preview||i.preview->form()!=a.form||!i.preview->baseMatches(m.post)||!pv::planeValid(i.actualPreviewPost)||!i.preview->matches(i.actualPreviewPost.target,a.acceptedPreviewSerial,a.lifetime,a.archive,i.actualPreviewPost.pixels.data(),pv::Full))return reject(Failure::Preview);
 if(i.sidebar343.size()!=343u*480u)return reject(Failure::Sidebar);
 if(!pv::planeValid(i.currentUploadPost)||!bg::retainedBaseTarget(m.post.target,i.currentUploadPost.target))return reject(Failure::Target);
 if(!a.hasSwatch){if(i.swatch||i.actualSwatchPost||a.swatchEvent.ownerEpoch||a.swatchEvent.sequence||a.swatchEvent.iteration)return reject(Failure::Swatch);}
 else{
  const auto&s=a.currentSwatch;const auto&e=a.swatchEvent;
  if(!sw::valid(s)||!event(e,a.epoch)||s.binding.form!=a.form||s.binding.epoch!=a.epoch||s.binding.lifetime!=a.lifetime||s.binding.owner!=a.sourceOwner||s.binding.archive!=a.archive||s.binding.car!=i.preview->selection().selectedCar||s.events.enter<=p.sequence||e.sequence!=s.events.exit||e.sequence>=u.sequence)return reject(Failure::Swatch);
  if((s.binding.origin==sw::Origin::Body&&e.sequence>=c.sequence)||(s.binding.origin==sw::Origin::Refresh&&s.events.enter<=c.sequence))return reject(Failure::Swatch);
  if(!i.swatch||!i.actualSwatchPost||!sw::plane(*i.actualSwatchPost)||!bg::retainedBaseTarget(m.post.target,i.actualSwatchPost->target)||!i.swatch->matches(s,i.actualSwatchPost->target,i.actualSwatchPost->pixels.data(),pv::Full))return reject(Failure::Swatch);
  const auto rect=i.swatch->bounds();if(rect.left!=366||rect.top!=141||rect.width!=85||rect.height!=22||i.swatch->pixels().size()!=85*22||i.swatch->coverage().size()!=85*22)return reject(Failure::Swatch);
 }
 try{
  std::vector<uint16_t>whole=i.preview->post();if(whole.size()!=pv::Full)return reject(Failure::Preview);
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){auto&dst=whole[y*640+x];dst=nfs_small_logo_live::pixel(dst,nfs_small_logo_live::expanded(i.sidebar343[y*343+x],i.copyColorKey),false);}
  // Swatch is outside343; inherited Body and post-copy Refresh are equivalent
  // only after their independent call order and exact coverage were proved.
  if(a.hasSwatch)for(unsigned y=0;y<22;++y)for(unsigned x=0;x<85;++x){size_t k=y*85+x;if(i.swatch->coverage()[k])whole[(y+141)*640+x+366]=i.swatch->pixels()[k];}
  if(!std::equal(whole.begin(),whole.end(),i.currentUploadPost.pixels.begin()))return reject(Failure::Pixels);
  std::shared_ptr<Packet>out(new Packet);out->authority_=a;out->preview_=i.preview;out->swatch_=i.swatch;out->original_=std::move(whole);return out;
 }catch(...){return reject(Failure::Allocation);}
}
} // namespace nfs_player_background
