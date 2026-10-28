#pragma once
#include "TextureDigest.h"
#include <cstring>
namespace nfs_ls_live {
inline bool exactHash(const void*p,size_t n,const char*hex)noexcept{unsigned char d[32];if(!p||!nfs_texture_sha256(p,n,d))return false;static const char a[]="0123456789abcdef";for(unsigned i=0;i<32;++i)if(hex[2*i]!=a[d[i]>>4]||hex[2*i+1]!=a[d[i]&15])return false;return !hex[64];}
constexpr const char*FontSHA="232674246c06a0cbfb2f27e10e792457dbfbfbef905a666bc4d4114959ed7fbb";
constexpr const char*GlyphSHA="c5153df6a3501ec61765701a89190c6f70841137544b84b628f63f6efe82e490";
inline bool knownPalette(size_t index,const void*p,size_t n)noexcept{static const char*const values[]={"6b183e9a1d7dade26f9b470fd94e425a3ee9cfb3033e5d680668b617fcc795c1","08130afec2a1621500fb072f55c6ab11bc0041baf6807d0f50a361c6124ed26e","6b183e9a1d7dade26f9b470fd94e425a3ee9cfb3033e5d680668b617fcc795c1","915bbe20a938c63e015a29a5b154c6878c25e9d9286fb301b1de7053beae535d","6b183e9a1d7dade26f9b470fd94e425a3ee9cfb3033e5d680668b617fcc795c1","60b02ee0f0d79ba41c92626675163a8025d66d01ba0f7e46767c5ed9bed3c428","6b183e9a1d7dade26f9b470fd94e425a3ee9cfb3033e5d680668b617fcc795c1","3f62c5c0160e4448fb183c4a605c64710516974f0bca2852cf514fabc6aec961"};return index<8&&n==64&&exactHash(p,n,values[index]);}
}
