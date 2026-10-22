// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
namespace nfs_soft {
struct FontGlyph {int16_t x=0,y=0,w=0,h=0,left=0,top=0;int32_t advance=0;};
struct FontDescription {
 static constexpr unsigned width=1024,height=512,roles=2,glyphCount=96;
 std::array<std::array<FontGlyph,glyphCount>,roles> glyphs{};
 std::array<std::array<int32_t,glyphCount*glyphCount>,roles> kern{};
 std::array<int32_t,roles> baseline{};
};
// Validation establishes shape and bounds, not license or provenance. The startup
// owner must authenticate the supplied font. All metadata and pixels are copied;
// mutable caller aliases cannot change a catalog after construction.
class FontCatalog final {
 FontDescription data_;std::vector<uint8_t> alpha_;uint64_t identity_;
 FontCatalog(const FontDescription&d,std::vector<uint8_t>&&a,uint64_t id):data_(d),alpha_(std::move(a)),identity_(id){}
public:
 static constexpr unsigned width=FontDescription::width,height=FontDescription::height;
 FontCatalog(const FontCatalog&)=delete;FontCatalog&operator=(const FontCatalog&)=delete;
 static std::shared_ptr<const FontCatalog> create(const FontDescription&d,const uint8_t*alpha,size_t bytes,uint64_t identity)noexcept{
  if(!identity||!alpha||bytes!=size_t(width)*height)return {};
  for(unsigned role=0;role<2;++role){
   if(d.baseline[role]<-512||d.baseline[role]>512)return {};
   for(const auto&g:d.glyphs[role]){
    if(g.w<0||g.h<0||g.advance<0||g.advance>4096*65536||g.left < -4096||g.left>4096||g.top < -4096||g.top>4096)return {};
    if((g.w==0)!=(g.h==0))return {};
    if(!g.w)continue;
    if(g.x<1||g.y<1||int(g.x)+g.w>=int(width)||int(g.y)+g.h>=int(height))return {};
    // The layout samples one transparent texel around every ink box.
    for(int x=g.x-1;x<=g.x+g.w;++x)if(alpha[size_t(g.y-1)*width+x]||alpha[size_t(g.y+g.h)*width+x])return {};
    for(int y=g.y-1;y<=g.y+g.h;++y)if(alpha[size_t(y)*width+g.x-1]||alpha[size_t(y)*width+g.x+g.w])return {};
   }
   for(int32_t k:d.kern[role])if(k < -4096*65536||k>4096*65536)return {};
  }
  try{return std::shared_ptr<const FontCatalog>(new FontCatalog(d,std::vector<uint8_t>(alpha,alpha+bytes),identity));}catch(...){return {};}
 }
 const FontDescription&description()const noexcept{return data_;}
 const std::vector<uint8_t>&alpha()const noexcept{return alpha_;}
 uint64_t identity()const noexcept{return identity_;}
};
} // namespace nfs_soft
