// SPDX-License-Identifier: MIT
// Synthetic fieldwise image-recipe comparisons only. No forged authoritative
// sidebar packets; actual accepted packet integration remains a live-test gate.
#include "hd/ControlsSidebarRecipeEquivalent.h"
#include <cassert>
#include <limits>
#include <vector>
using namespace nfs_controls_sidebar_recipe;
template<class T,class Equal,class Mutate>static void changed(const T&a,Equal equal,Mutate mutate){T b=a;assert(equal(a,b));mutate(b);assert(!equal(a,b));}
static nfs_soft::SurfaceIdentity validSurface(){nfs_soft::SurfaceIdentity s;s.lifetime=1;auto&g=s.geometry;g[1]=343;g[2]=480;g[5]=343;g[6]=480;g[7]=16;g[8]=0x10000;g[10]=686;g[11]=0x1000;g[12]=0x2000;g[15]=2;g[16]=684;g[17]=16;g[18]=702;g[19]=16+479*686;assert(s.valid());return s;}
int main(){
 assert(!equivalent({},{}));
 NfsBackgroundSceneWordLayer w{};w.sourceRole=1;w.scalar256=256;w.clip[2]=343;w.clip[3]=480;
 changed(w,detail::word,[](auto&v){++v.sourceRole;});changed(w,detail::word,[](auto&v){--v.scalar256;});changed(w,detail::word,[](auto&v){++v.left;});changed(w,detail::word,[](auto&v){++v.top;});
 for(unsigned k=0;k<4;++k)changed(w,detail::word,[k](auto&v){++v.clip[k];});
 NfsSmallLogoLayer l{};l.scalar256=256;l.right=10;l.bottom=10;l.clip[2]=343;l.clip[3]=480;
 changed(l,detail::logo,[](auto&v){++v.role;});changed(l,detail::logo,[](auto&v){--v.scalar256;});
 for(unsigned k=0;k<8;++k)changed(l,detail::logo,[k](auto&v){float*fields[]={&v.left,&v.top,&v.right,&v.bottom,&v.clip[0],&v.clip[1],&v.clip[2],&v.clip[3]};*fields[k]+=1;});
 changed(l,detail::logo,[](auto&v){v.left=std::numeric_limits<float>::infinity();});
 auto nan=l;nan.left=std::numeric_limits<float>::quiet_NaN();assert(!detail::logo(nan,nan));
 nfs_background_scene_cpu::SourceCatalog c{};
 changed(c,detail::catalog,[](auto&v){++v.roles[0];});changed(c,detail::catalog,[](auto&v){++v.mapRoles[0];});changed(c,detail::catalog,[](auto&v){++v.mapSha[1][31];});changed(c,detail::catalog,[](auto&v){++v.logos.roles[4];});
 for(unsigned k=0;k<13;++k){
  for(unsigned f=0;f<4;++f)changed(c,detail::catalog,[k,f](auto&v){auto&i=v.identities[k];uint32_t*fields[]={&i.span,&i.width,&i.height,&i.paletteOffset};++*fields[f];});
  changed(c,detail::catalog,[k](auto&v){++v.identities[k].header[15];});changed(c,detail::catalog,[k](auto&v){++v.identities[k].sha[31];});
 }
 for(unsigned k=0;k<5;++k){
  for(unsigned f=0;f<6;++f)changed(c,detail::catalog,[k,f](auto&v){auto&i=v.logos.identities[k];uint32_t*fields[]={&i.span,&i.width,&i.height,&i.paletteOffset,&i.paletteCount,&i.maxIndex};++*fields[f];});
  changed(c,detail::catalog,[k](auto&v){++v.logos.identities[k].header[3];});changed(c,detail::catalog,[k](auto&v){++v.logos.identities[k].sha[31];});
 }
 nfs_soft::TextCommand t{};t.clip={0,0,343,480};
 changed(t,detail::command,[](auto&v){++v.text[95];});
 for(unsigned f=0;f<4;++f)changed(t,detail::command,[f](auto&v){uint32_t*fields[]={&v.length,&v.role,&v.alignment,&v.sourceSize};++*fields[f];});
 for(unsigned f=0;f<5;++f)changed(t,detail::command,[f](auto&v){int32_t*fields[]={&v.x,&v.y,&v.originX,&v.originY,&v.advance};++*fields[f];});
 changed(t,detail::command,[](auto&v){++v.color;});changed(t,detail::command,[](auto&v){v.clip.right-=1;});
 nfs_soft::Vertex v{};for(unsigned k=0;k<8;++k)changed(v,detail::vertex,[k](auto&x){float*fields[]={&x.x,&x.y,&x.u,&x.v,&x.r,&x.g,&x.b,&x.a};*fields[k]+=1;});
 auto surface=validSurface(),version=surface;version.lifetime=2;version.geometry[8]+=0x100000;version.geometry[11]+=0x10000;version.geometry[12]+=0x10000;assert(detail::surface(surface,version));
 changed(surface,detail::surface,[](auto&x){x.geometry[3]=1;});changed(surface,detail::surface,[](auto&x){++x.geometry[16];});
 nfs_soft::FontDescription desc{};std::vector<uint8_t>alpha(size_t(nfs_soft::FontCatalog::width)*nfs_soft::FontCatalog::height);
 auto font=nfs_soft::FontCatalog::create(desc,alpha.data(),alpha.size(),1);auto same=nfs_soft::FontCatalog::create(desc,alpha.data(),alpha.size(),1);assert(font&&same&&detail::font(*font,*same)&&detail::font(*font,*font));
 auto other=nfs_soft::FontCatalog::create(desc,alpha.data(),alpha.size(),2);assert(other&&!detail::font(*font,*other));
 auto different=desc;different.glyphs[1][95].advance=1;other=nfs_soft::FontCatalog::create(different,alpha.data(),alpha.size(),1);assert(other&&!detail::font(*font,*other));
 different=desc;different.kern[1][95]=1;other=nfs_soft::FontCatalog::create(different,alpha.data(),alpha.size(),1);assert(other&&!detail::font(*font,*other));
 different=desc;different.baseline[1]=1;other=nfs_soft::FontCatalog::create(different,alpha.data(),alpha.size(),1);assert(other&&!detail::font(*font,*other));
 alpha[123]=1;other=nfs_soft::FontCatalog::create(desc,alpha.data(),alpha.size(),1);assert(other&&!detail::font(*font,*other));
 std::vector<unsigned char>rgba(192*128*4,42),rgba2=rgba;NfsCompactCaptionWordView art{};art.schema=NFS_COMPACT_CAPTION_WORD_SCHEMA;art.word=3;art.width=192;art.height=128;art.rgba=rgba.data();art.bytes=rgba.size();art.left=120;art.top=364;art.right=168;art.bottom=396;auto art2=art;art2.rgba=rgba2.data();assert(detail::captionWord(art,art2));
 rgba2.back()^=1;assert(!detail::captionWord(art,art2));rgba2.back()^=1;
 changed(art,detail::captionWord,[](auto&x){++x.sha256[31];});changed(art,detail::captionWord,[](auto&x){--x.bytes;});changed(art,detail::captionWord,[](auto&x){x.right+=1;});changed(art,detail::captionWord,[](auto&x){x.rgba=nullptr;});
}
