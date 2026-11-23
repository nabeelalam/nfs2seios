// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerGraphCanvasReplay.h"
#include "PlayerGraphSourceCatalog.h"
#include "../TextureDigest.h"
#include <memory>

// Authenticated CPU source/replay only. Live original-owner, menu token,
// archive lifetime, crop/conversion, 100 native uploads and original animation
// passes remain independently bound by the maintained Player photo integration.
namespace nfs_player_graph_source {
struct Domain {
 uintptr_t game=0,thread=0;uint64_t epoch=0;
 uint32_t form=0,formGuest=0,archive=0,car=0;std::array<uint32_t,4>statistics{};
};
struct Canvas {
 unsigned bits=0;std::array<uint32_t,14>words{};std::array<uint32_t,4>header{};
 std::array<uint32_t,384>x{};std::array<uint32_t,160>y{};
 std::vector<uint8_t>pixels;std::array<unsigned char,32>sha{};
};
enum class Failure {None,Replay,Domain,Clear,Source,Palette,Table,Fill,Allocation};
class Packet;
template<class Capture>std::shared_ptr<const Packet>verify(const Capture&,Failure* =nullptr)noexcept;
class Packet final {
 Domain domain_;std::array<std::vector<uint8_t>,5>sources_;std::array<Canvas,2>canvases_;
 Packet()=default;
 template<class Capture>friend std::shared_ptr<const Packet>verify(const Capture&,Failure*)noexcept;
public:
 Packet(const Packet&)=delete;Packet&operator=(const Packet&)=delete;
 const Domain&domain()const noexcept{return domain_;}
 const std::array<std::vector<uint8_t>,5>&sources()const noexcept{return sources_;}
 const std::array<Canvas,2>&canvases()const noexcept{return canvases_;}
};
template<class Capture>std::shared_ptr<const Packet>verify(const Capture&c,Failure*failure)noexcept{
 namespace rr=nfs_player_graph_replay;
 auto reject=[&](Failure f)->std::shared_ptr<const Packet>{if(failure)*failure=f;return {};};
 if(failure)*failure=Failure::None;
 if(!rr::replay(c).matched)return reject(Failure::Replay);
 if(!c.game||!c.thread||!c.epoch||!c.formGuest||!c.archive||c.car>=12||(c.form!=0x4d3050&&c.form!=0x4d3100))return reject(Failure::Domain);
 try{
  for(unsigned phase=0;phase<2;++phase){const auto&p=c.photos[phase];
   if(std::any_of(p.before.pixels.begin(),p.before.pixels.end(),[](uint8_t v){return v!=0;}))return reject(Failure::Clear);
   for(const auto&r:p.fillRows)if(r.pattern!=p.fillColor)return reject(Failure::Fill);
   for(unsigned k=0;k<5;++k){const auto&d=p.draws[k];const auto&id=catalog[k];const auto&raw=d.sourceBefore;
    if(std::memcmp(d.name.data(),id.name,4)||raw.size()!=id.span||d.sourceAfter!=raw||uint64_t(c.archive)+id.offset+id.span>UINT64_C(0x100000000)||uint64_t(d.args[0])!=uint64_t(c.archive)+id.offset)return reject(Failure::Source);
    for(unsigned h=0;h<4;++h)if(rr::le(raw.data()+h*4,4)!=id.header[h])return reject(Failure::Source);
    if((id.header[1]&65535)!=id.width||(id.header[1]>>16)!=id.height||(id.header[0]>>8)!=id.paletteOffset||rr::le(raw.data()+id.paletteOffset+4,2)!=id.paletteCount)return reject(Failure::Source);
    std::array<unsigned char,32>sha{};if(!nfs_texture_sha256(raw.data(),raw.size(),sha.data())||sha!=id.sha)return reject(Failure::Source);
    // 48330C overwrites only tail[6]/[8] before actual draw consumption.
    // Prior values are not authority; policy[7] remains a semantic input.
    if(!rr::tablePrefix(d.tableBefore)||d.tableBefore[7]!=3)return reject(Failure::Table);
    std::array<uint32_t,9>expected{{0xf000006c,0xf000183c,0xf0001840,0xf0001844,0xf0001848,0xf000184c,phase?0xf0001850u:0u,3,phase?0x7bu:0x7du}};
    if(d.tableAfter!=expected)return reject(Failure::Table);
    for(const auto&r:d.rows){
     if(r.callback!=(phase?0xf0001850u:0xf000211cu)||r.converter!=(phase?0u:0xf0002218u)||r.format!=(phase?16u:32u)||(!phase&&(!r.wrappedIndexed32||r.forward!=0xf000184c)))return reject(Failure::Palette);
     // All 256 canonical palette entries are authenticated, not merely the
     // subset that happened to be consumed in this clipped destination row.
     for(unsigned index=0;index<id.paletteCount;++index){const uint32_t embedded=rr::le(raw.data()+id.paletteOffset+16+index*4,4);const uint32_t want=phase?((embedded&0xff000000u)|rr::pack565(embedded)):embedded;if(r.paletteBefore[index]!=want||r.paletteAfter[index]!=want)return reject(Failure::Palette);}
    }
   }
  }
  auto p=std::shared_ptr<Packet>(new Packet);
  p->domain_={c.game,c.thread,c.epoch,c.form,c.formGuest,c.archive,c.car,c.photos[0].consumed.graphValues};
  for(unsigned k=0;k<5;++k)p->sources_[k]=c.photos[0].draws[k].sourceBefore;
  for(unsigned phase=0;phase<2;++phase){const auto&input=c.photos[phase].after;auto&out=p->canvases_[phase];out.bits=input.bits;out.words=input.words;out.header=input.header;out.x=input.x;out.y=input.y;out.pixels=input.pixels;if(!nfs_texture_sha256(out.pixels.data(),out.pixels.size(),out.sha.data()))return reject(Failure::Allocation);}
  return p;
 }catch(...){return reject(Failure::Allocation);}
}
} // namespace nfs_player_graph_source
