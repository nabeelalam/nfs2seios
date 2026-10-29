// SPDX-License-Identifier: MIT
#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include <array>
#include <cstring>
namespace nfs_player_stat_material {
inline float limit(float v,float a=0,float b=1){return std::max(a,std::min(b,v));}
inline void pixel(std::vector<unsigned char>&v,size_t at,float r,float g,float b,float a){unsigned A=unsigned(limit(a)*255+.5f);v[at]=unsigned(limit(r,0,255)*A/255+.5f);v[at+1]=unsigned(limit(g,0,255)*A/255+.5f);v[at+2]=unsigned(limit(b,0,255)*A/255+.5f);v[at+3]=A;}
inline std::vector<unsigned char>plate(){
 const unsigned W=307*4,H=147*4;std::vector<unsigned char>v(W*H*4);
 for(unsigned y=0;y<H;++y)for(unsigned x=0;x<W;++x){float X=(x+.5f)/4,Y=(y+.5f)/4,right=289-Y*.065f+2*std::sin(Y*3.14159265f/130),fade=limit((X-2)/36),edge=std::min(std::min(Y,130-Y),right-X),a=limit(edge*4+.5f)*fade;
  float shine=.72f+.23f*std::cos((Y-33)*.022f)+.08f*std::sin(X*.009f);float r=195*shine,g=104*shine,b=18*shine;
  // Fine drafting grid and broad circular construction lines, not raster grain.
  float gx=std::fmod(X+4,25.f),gy=std::fmod(Y+7,25.f);if(gx<.28f||gy<.28f){r*=.77f;g*=.77f;b*=.77f;}
  if(std::fabs(X-Y*.9f-185)<.28f||std::fabs(X+Y*.65f-264)<.28f){r*=.72f;g*=.72f;b*=.72f;}
  float radial=std::sqrt((X-160)*(X-160)+(Y-63)*(Y-63));if(std::fabs(radial-93)<.3f||std::fabs(radial-123)<.3f){r*=.67f;g*=.67f;b*=.67f;}
  for(unsigned row=0;row<4;++row){float top=11+30*row,dx=std::min(X-26,273-X),dy=std::min(Y-top,top+15-Y);if(dx>=0&&dy>=0){float bevel=std::min(dx,dy);if(bevel<1.2f){r=47;g=22;b=4;}else if(Y<top+3){r=9;g=5;b=1;}else if(Y>top+13){r=170;g=84;b=10;}else{r=63;g=31;b=4;}}}
  if(edge<1.0f&&edge>=0){r*=.55f;g*=.55f;b*=.55f;}if(Y>.7f&&Y<1.15f){r=233;g=160;b=65;}
  size_t at=4*(y*W+x);if(a>0)pixel(v,at,r,g,b,a);else{float sx=right+6-X,sy=137-Y;float sh=limit(std::min(sx,sy)/6)*limit((X-3)/40)*limit(Y/9)*.52f;if(X<right+6&&Y<137)pixel(v,at,0,0,0,sh);}
 }return v;
}
inline std::vector<unsigned char>bar(unsigned role){
 static const float colors[4][3]={{58,145,9},{20,28,204},{207,25,21},{220,214,10}};std::vector<unsigned char>v(984*60*4);
 for(unsigned y=0;y<60;++y)for(unsigned x=0;x<984;++x){float X=(x+.5f)/4,Y=(y+.5f)/4,e=std::min(std::min(X,246-X),std::min(Y,15-Y)),a=limit(e*4+.5f),light=.67f+.27f*std::cos((Y-4)*.18f);if(Y<1.2f)light=1.3f;else if(Y>13.4f)light=.4f;if(X>244.6f)light=1.15f;pixel(v,4*(y*984+x),colors[role][0]*light,colors[role][1]*light,colors[role][2]*light,a);}return v;
}
struct Assets{std::vector<unsigned char>paper;std::array<std::vector<unsigned char>,4>bars;Assets():paper(plate()){for(unsigned k=0;k<4;++k)bars[k]=bar(k);}};
inline const Assets&assets(){static const Assets a;return a;}
template<class P,class S,class O>inline void decorate(const P&p,const S*swatch,O&o){
 static const char names[5][5]={"grph","grpg","grpb","grpr","grpy"};
 static const unsigned char hashes[5][32]={
 {0x14,0x54,0xfb,0x9f,0x5d,0xb5,0x06,0x2c,0x7c,0x0e,0xa8,0x41,0xdf,0x2a,0xd4,0x54,0x80,0xab,0x95,0xb5,0x46,0x34,0x14,0xc5,0x34,0xba,0xa0,0xb0,0xff,0x53,0x15,0xbd},
 {0x01,0xd3,0x92,0xb5,0xfb,0x1f,0xfe,0x3e,0x18,0xc9,0xa2,0x7e,0xf2,0x41,0xda,0x5d,0xe6,0x4a,0xb7,0xd3,0xe8,0x45,0x1d,0xe8,0xee,0x6f,0x97,0xd2,0x36,0xa0,0xed,0x73},
 {0x2f,0x5b,0x05,0xaa,0x4a,0xf3,0xd2,0xfc,0xaa,0xa7,0x21,0x55,0x2b,0x43,0x27,0x9b,0x93,0x04,0xe1,0x48,0x44,0x20,0x0f,0xf7,0xec,0x7a,0x7b,0x3d,0x46,0xdc,0x5e,0xd7},
 {0x9a,0x3a,0xbf,0xcc,0xc0,0xb2,0x58,0xf3,0xf4,0x84,0x2a,0xb9,0x01,0x48,0x73,0xe7,0x84,0x01,0xfe,0x93,0xe6,0x52,0xfe,0x64,0x5a,0xe1,0xb7,0x4f,0xbf,0x8d,0xe6,0x16},
 {0xb0,0xb9,0x7b,0x16,0xb6,0x23,0xd8,0x41,0x15,0xf7,0xbe,0xa9,0x6a,0xfd,0x04,0x1f,0x3e,0xa5,0xf9,0xc5,0x33,0x6a,0x6f,0xb6,0x88,0xed,0x68,0x79,0xbb,0xd3,0xfc,0x6b}};
 if(p.sources().size()!=5||!p.selection().graphEnabled)return;for(unsigned k=0;k<5;++k)if(std::memcmp(p.sources()[k].name.data(),names[k],4)||std::memcmp(p.sources()[k].sha.data(),hashes[k],32))return;
 const unsigned W=o.width*4,H=o.height*4;if(!W||!H||W>2560||H>1920)return;auto&a=o.authored;a.resize(size_t(W)*H*4);
 auto put=[&](int x,int y,const unsigned char*s,bool blend){if(x<0||y<0||x>=int(W)||y>=int(H))return;auto*d=a.data()+4*(y*W+x);for(unsigned k=0;k<4;++k)d[k]=blend?static_cast<unsigned char>(s[k]+(d[k]*(255-s[3])+127)/255):s[k];};
 for(unsigned y=0;y<H;++y)for(unsigned x=0;x<W;++x){size_t q=(y/4)*o.width+x/4;unsigned w=o.pixels[q],A=o.coverage[q];unsigned char c[4]={static_cast<unsigned char>(A?((w>>11)*255/31):0),static_cast<unsigned char>(A?(((w>>5)&63)*255/63):0),static_cast<unsigned char>(A?((w&31)*255/31):0),static_cast<unsigned char>(A)};put(x,y,c,false);}
 // Clear ONLY the exact old plate rectangle before new alpha; the original
 // black-fill rectangle is opaque, external plate/shadow pixels transparent.
 const auto&pb=p.bounds();for(unsigned y=0;y<pb.height*4;++y)for(unsigned x=0;x<pb.width*4;++x){int X=int(pb.left*4+x),Y=int(pb.top*4+y);unsigned char c[4]={0,0,0,static_cast<unsigned char>(X>=211*4&&X<491*4&&Y>=316*4&&Y<444*4?255:0)};put(X-o.left*4,Y-o.top*4,c,false);}
 const auto&t=assets();auto draw=[&](const std::vector<unsigned char>&v,unsigned w,unsigned h,int left,int top){for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)put(left*4+int(x)-o.left*4,top*4+int(y)-o.top*4,v.data()+4*(y*w+x),true);};draw(t.paper,1228,588,211,316);
 for(unsigned k=0;k<4;++k)draw(t.bars[k],984,60,237-9*(12-int(p.selection().graphValues[k])),327+30*k);
 // Existing source swatch remains the last preview operation, exact original
 // geometry/coverage; no dynamic data, source operation or guest pixel changes.
 if(swatch){const auto&b=swatch->bounds();for(unsigned y=0;y<b.height;++y)for(unsigned x=0;x<b.width;++x){size_t q=y*b.width+x;if(!swatch->coverage()[q])continue;unsigned v=swatch->pixels()[q];unsigned char c[4]={static_cast<unsigned char>((v>>11)*255/31),static_cast<unsigned char>(((v>>5)&63)*255/63),static_cast<unsigned char>((v&31)*255/31),255};for(unsigned dy=0;dy<4;++dy)for(unsigned dx=0;dx<4;++dx)put((int(b.left+x)-o.left)*4+dx,(int(b.top+y)-o.top)*4+dy,c,false);}}
 if(!nfs_texture_sha256(a.data(),a.size(),o.authoredSha.data()))o.authored.clear();
}
}
