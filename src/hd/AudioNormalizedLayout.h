// SPDX-License-Identifier: MIT
#pragma once
#include "AudioLayerAbi.h"
#include "TextureDigest.h"
#include <array>
#include <memory>
#include <vector>
#include <algorithm>
#include <cstring>
namespace nfs_audio_frame {
class ArtBinding final {
 std::vector<unsigned char>pixels_;uint64_t identity_=0;std::array<unsigned char,32>sha_{};ArtBinding()=default;
public:
 // expected is authenticated by the startup provider/recipe, independently of
 // input.sha256. Shape or a caller numeric identity cannot supply provenance.
 static std::shared_ptr<const ArtBinding>create(const NfsAudioArt&input,const std::array<unsigned char,32>&expected)noexcept{
  if(!input.pixels||input.bytes!=NFS_AUDIO_ART_BYTES||input.width!=2048||input.height!=768||input.schema!=2||input.premultiplied!=1||input.opacityBaked!=1||!input.identity||std::memcmp(input.sha256,expected.data(),32))return {};
  try{auto p=std::shared_ptr<ArtBinding>(new ArtBinding);p->pixels_.assign(input.pixels,input.pixels+input.bytes);if(!nfs_texture_sha256(p->pixels_.data(),p->pixels_.size(),p->sha_.data())||p->sha_!=expected)return {};for(size_t k=0;k<p->pixels_.size();k+=4)if(p->pixels_[k]>p->pixels_[k+3]||p->pixels_[k+1]>p->pixels_[k+3]||p->pixels_[k+2]>p->pixels_[k+3])return {};p->identity_=input.identity;return p;}catch(...){return {};}
 }
 NfsAudioArt view()const noexcept{NfsAudioArt a{};a.pixels=pixels_.data();a.bytes=pixels_.size();a.width=2048;a.height=768;a.schema=2;a.premultiplied=a.opacityBaked=1;a.identity=identity_;std::copy(sha_.begin(),sha_.end(),a.sha256);return a;}
};
struct Rect {int32_t left=0,top=0,right=0,bottom=0;};
struct BarDrawLayout {uint32_t asset=0,row=0,sequence=0,mode=0;int32_t x=0,y=0;Rect clip;};
struct BarState {std::array<uint8_t,3>values{};int32_t selected=0,held=0;uint32_t count=0;std::array<BarDrawLayout,13>draws{};};
struct OwnedBarLayout {uint32_t vertexCount=0;std::array<NfsAudioBarVertex,78>vertices{};BarState state;};
constexpr int32_t HotX[6]={-432,-432,-411,-577,-411,-577},HotY[6]={-141,-147,-141,-141,-141,-141};
constexpr uint32_t Width[6]={183,156,41,38,38,38},Height[6]={31,10,26,23,23,24};
inline bool normalize(const BarState&s,OwnedBarLayout&out)noexcept{
 if(s.selected<0||(s.selected>=14&&s.selected!=100)||s.held<0||s.held>2)return false;for(auto v:s.values)if(v>140)return false;const bool held=s.selected<3&&s.held; if(s.count!=(held?13u:12u))return false;
 static constexpr unsigned region[12][2]={{4,4},{1106,4},{1106,72},{1360,72},{1596,72},{1106,236},{4,388},{1106,388},{1106,456},{1360,456},{1596,456},{1106,620}};OwnedBarLayout result{};result.state=s;
 for(unsigned k=0;k<s.count;++k){unsigned row=k<3?k:k<6?k-3:k<12?(k-6)/2:unsigned(s.selected),asset=k<3?0:k<6?1:k<12?((k-6)&1?3:2):(s.held==1?4:5);unsigned opacity=asset==2||int(row)==s.selected?256:140;int x=asset==1?int(s.values[row])-163:-23,y=6+27*int(row);Rect clip=asset==1?Rect{392,0,640,480}:Rect{0,0,640,480};const auto&d=s.draws[k];if(d.asset!=asset||d.row!=row||d.sequence!=k||d.mode!=(0x2000|opacity)||d.x!=x||d.y!=y||d.clip.left!=clip.left||d.clip.top!=clip.top||d.clip.right!=clip.right||d.clip.bottom!=clip.bottom)return false;
  const int left=x-HotX[asset],top=y-HotY[asset],right=left+int(Width[asset]),bottom=top+int(Height[asset]);const int l=std::max(left,clip.left),t=std::max(top,clip.top),r=std::min(right,clip.right),b=std::min(bottom,clip.bottom);if(left<0||top<0||right>640||bottom>480||l>=r||t>=b)return false;const auto&at=region[asset+(opacity==140?6:0)];float u0=float(at[0]+(l-left)*6)/2048,u1=float(at[0]+(r-left)*6)/2048,v0=float(at[1]+(t-top)*6)/768,v1=float(at[1]+(b-top)*6)/768;NfsAudioBarVertex batch[6]={{float(l),float(t),u0,v0},{float(r),float(t),u1,v0},{float(l),float(b),u0,v1},{float(l),float(b),u0,v1},{float(r),float(t),u1,v0},{float(r),float(b),u1,v1}};std::copy_n(batch,6,result.vertices.begin()+result.vertexCount);result.vertexCount+=6;
 }out=result;return true;
}
}
