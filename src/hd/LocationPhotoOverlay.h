// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>
// Pure source-command replay. No guest reads, source ownership or asset authority.
namespace nfs_location_photo_overlay {
inline uint32_t le32(const unsigned char*p){return uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16|uint32_t(p[3])<<24;}
inline uint16_t pack(uint32_t p){return uint16_t(((p>>8)&0xf800)|((p>>5)&0x7e0)|((p>>3)&31));}
inline uint16_t blend16(uint16_t old,uint32_t raw){uint32_t c=(raw&0xff000000u)|pack(raw);if(c<0x10000000u)return old;if(c>=0xfc000000u)return uint16_t(c);uint32_t fields=(old&31u)|((old&0x7e0u)<<8)|((old&0xf800u)<<16);uint64_t p=uint64_t(fields)*(255-(c>>24));return uint16_t(c+(((p>>32)<<8)&0xf800)+((p>>16)&0x7e0)+((p>>8)&31));}
// Original4831A8 byte arithmetic, including transparent-destination shortcut,
// truncation and wrapping stores. Source colours are already premultiplied.
inline uint32_t blend32(uint32_t old,uint32_t c){if(!(c>>24))return old;if(!(old>>24))return c;uint32_t out=0,remain=255-(c>>24);for(unsigned k=0;k<3;++k)out|=uint32_t(uint8_t((c>>(8*k))+(((old>>(8*k))&255)*remain>>8)))<<(8*k);out|=(255-((remain*(255-(old>>24)))>>8))<<24;return out;}
// Original4A1090 indexed8->ARGB32 route. Numeric consistency only: the
// live caller must separately own the source/palette and original row target.
constexpr uint32_t Indexed32Wrapper=0xf000211cu,Indexed32Converter=0xf0002218u,Argb32Forward=0xf000184cu;
inline bool indexed32Contract(uint32_t wrapper,uint32_t converter,uint32_t forward,uint32_t palette,uint32_t expectedPalette)noexcept{return wrapper==Indexed32Wrapper&&converter==Indexed32Converter&&forward==Argb32Forward&&palette&&palette==expectedPalette;}
struct Command{unsigned width=0,height=0,count=0;int x=0,y=0;std::vector<unsigned char>indices;std::array<uint32_t,256>palette{};};
inline bool decode(const std::vector<unsigned char>&s,int x,int y,Command&c){if(s.size()<32)return false;uint32_t first=le32(s.data()),wh=le32(s.data()+4),off=first>>8;c.width=wh&65535;c.height=wh>>16;if((first&127)!=0x7b||!c.width||c.width>384||!c.height||c.height>160||16ull+uint64_t(c.width)*c.height>off||off-(16ull+uint64_t(c.width)*c.height)>3||uint64_t(off)+16>s.size()||(le32(s.data()+off)&255)!=0x2a)return false;c.count=le32(s.data()+off+4)&65535;if(!c.count||c.count>256||uint64_t(off)+16+4*c.count!=s.size())return false;c.indices.assign(s.begin()+16,s.begin()+16+c.width*c.height);c.palette.fill(0);for(unsigned k=0;k<c.count;++k)c.palette[k]=le32(s.data()+off+16+4*k);for(auto i:c.indices)if(i>=c.count&&i!=255)return false;c.x=x;c.y=y;return true;}
inline uint32_t at(const Command&c,int x,int y){x-=c.x;y-=c.y;if(x<0||y<0||x>=int(c.width)||y>=int(c.height))return 0;return c.palette[c.indices[size_t(y)*c.width+x]];}
inline void overlay(unsigned char*d,int X,int Y,unsigned bits,const std::vector<Command>&commands){uint32_t old=uint32_t(d[2])|uint32_t(d[1])<<8|uint32_t(d[0])<<16|uint32_t(d[3])<<24;for(size_t j=1;j<commands.size();++j){uint32_t c=at(commands[j],X/4,Y/4);if(bits==32)old=blend32(old,c);else{uint16_t w=blend16(pack(old),c);if(c>=0x10000000u)old=0xff000000u|((w&0xf800u)<<8)|((w&0x7e0u)<<5)|((w&31u)<<3);}}d[0]=uint8_t(old>>16);d[1]=uint8_t(old>>8);d[2]=uint8_t(old);d[3]=uint8_t(old>>24);}
// False leaves destination untouched: caller retains the already authenticated
// native converted tile, including localized overlays outside base coverage.
inline bool prepare(unsigned char*d,int X,int Y,const unsigned char*art,unsigned width,unsigned height,unsigned bits,const std::vector<Command>&commands){if(!d||!art||X<0||Y<0||X>=int(width)||Y>=int(height)||commands.empty()||(bits!=16&&bits!=32)||at(commands[0],X/4,Y/4)<0x10000000u)return false;const auto&base=commands[0];const int sx=X/4-base.x,sy=Y/4-base.y;if(sx<0||sy<0||sx>=int(base.width)||sy>=int(base.height)||base.indices[size_t(sy)*base.width+sx]==255)return false;const auto*p=art+4*(size_t(Y)*width+X);for(unsigned k=0;k<4;++k)d[k]=p[k];overlay(d,X,Y,bits,commands);if(d[3]&&d[3]!=255)for(unsigned k=0;k<3;++k)d[k]=uint8_t(std::min(255u,(unsigned(d[k])*255u+d[3]/2u)/d[3]));return true;}

}
