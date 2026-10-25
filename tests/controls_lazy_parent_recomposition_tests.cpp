// SPDX-License-Identifier: MIT
// Synthetic candidate-recomposition checks, without assets or source authority.
#include "hd/ControlsLazyParentRecomposition.h"
#include <array>
#include <cassert>
#include <limits>
#include <type_traits>
using namespace nfs_controls_lazy_parent_recomposition;
static_assert(!std::is_constructible<VerifiedOriginalRecipe,std::vector<uint16_t>>::value,"No raw recipe construction");
static void put16(std::vector<uint8_t>&v,size_t at,uint16_t x){v[at]=uint8_t(x);v[at+1]=uint8_t(x>>8);}
static void put32(std::vector<uint8_t>&v,size_t at,uint32_t x){for(unsigned k=0;k<4;++k)v[at+k]=uint8_t(x>>(8*k));}
struct Fixture {
 std::vector<uint8_t>map=std::vector<uint8_t>(16+Pixels*2,0),sidebar=std::vector<uint8_t>(16+343*480*2,0),actual=std::vector<uint8_t>(Pixels*2,0);
 std::vector<uint16_t>clean=std::vector<uint16_t>(Pixels,0x1111),original;
 std::array<NfsSoftwareTextVertex,696>vertices{};NfsControlsSceneFrameView view{};
 Fixture(){put32(map,0,0x78);put32(map,4,640u|(480u<<16));put32(sidebar,0,0x78);put32(sidebar,4,343u|(480u<<16));for(size_t at=0;at<Pixels;++at)put16(map,16+at*2,0x1111);
  side(10,10,0x2222);clean[10*640+10]=0x2222;original=clean;original[101*640+301]=0xffff;original[205*640+405]=0xeeee;
  view.schema=NFS_CONTROLS_SCENE_RUNTIME_SCHEMA;view.width=640;view.height=480;view.layers.text.logicalWidth=640;view.layers.text.logicalHeight=480;view.layers.text.vertices=vertices.data();view.layers.text.vertexCount=696;view.layers.dialogWords.schema=NFS_CONTROLS_DIALOG_WORD_SCHEMA;view.layers.dialogWords.count=10;
  for(size_t first=0;first<696;first+=6){const float p[6][2]={{300,100},{302,100},{300,102},{300,102},{302,100},{302,102}};for(unsigned k=0;k<6;++k){auto&v=vertices[first+k];v.x=p[k][0];v.y=p[k][1];v.r=v.g=v.b=v.a=1;}}
  for(auto&p:view.layers.dialogWords.placements){p.word=1;p.left=400;p.top=200;p.right=420;p.bottom=232;}refresh();
 }
 void side(size_t x,size_t y,uint16_t p){put16(sidebar,16+(y*343+x)*2,p);}
 void refresh(){for(size_t y=0;y<480;++y)for(size_t x=0;x<640;++x){const size_t at=y*640+x;uint16_t p=0x1111;if(x<343){const size_t sa=16+(y*343+x)*2;const uint16_t s=uint16_t(sidebar[sa])|(uint16_t(sidebar[sa+1])<<8);if(s)p=s;}if((x>=299&&x<303&&y>=99&&y<103)||(x>=399&&x<421&&y>=199&&y<233))p=original[at];put16(actual,at*2,p);}}
 Result result()const{return verify(map,sidebar,clean,original,actual,view);}
 bool ok()const{return result().failure==Failure::None;}
};
int main(){
 {Fixture f;assert(f.ok());auto r=f.result();assert(r.recipe&&r.recipe->finalPlane().size()==Pixels);const auto p=r.recipe->finalPlane()[0];f.actual[0]^=1;f.original[0]^=1;assert(r.recipe->finalPlane()[0]==p);}
 // Full reset removes the old opaque sidebar pixel, then adds a moved one.
 {Fixture f;f.side(10,10,0);f.side(11,10,0x3333);f.refresh();assert(f.ok());}
 // A lazy keyed copy that retains the old opaque pixel is not full recomposition.
 {Fixture f;f.side(10,10,0);f.refresh();put16(f.actual,(10*640+10)*2,0x2222);assert(!f.ok());}
 {Fixture f;f.side(300,100,0x4444);f.refresh();assert(f.result().failure==Failure::ProtectedBackground);}
 {Fixture f;f.side(299,99,0x4444);f.refresh();assert(f.result().failure==Failure::ProtectedBackground);}
 {Fixture f;f.side(298,98,0x4444);f.refresh();assert(f.ok());}
 {Fixture f;put16(f.map,16+(205*640+405)*2,0x5555);assert(f.result().failure==Failure::ProtectedBackground);}
 {Fixture f;f.original[1]^=1;assert(f.result().failure==Failure::UntrackedText);}
 {Fixture f;f.actual.back()^=1;assert(f.result().failure==Failure::ActualPlane);}
 {Fixture f;f.actual[(101*640+301)*2]^=1;assert(f.result().failure==Failure::ActualPlane);}
 {Fixture f;f.map.pop_back();assert(f.result().failure==Failure::Span);}
 {Fixture f;f.sidebar.pop_back();assert(f.result().failure==Failure::Span);}
 {Fixture f;f.clean.pop_back();assert(f.result().failure==Failure::Span);}
 {Fixture f;f.original.pop_back();assert(f.result().failure==Failure::Span);}
 {Fixture f;f.actual.pop_back();assert(f.result().failure==Failure::Span);}
 {Fixture f;f.map[0]=0;assert(f.result().failure==Failure::Header);}
 {Fixture f;f.map[8]=1;assert(f.result().failure==Failure::Header);}
 {Fixture f;f.map[12]=1;assert(f.result().failure==Failure::Header);}
 {Fixture f;f.sidebar[4]^=1;assert(f.result().failure==Failure::Header);}
 {Fixture f;f.sidebar[0]=0;assert(f.result().failure==Failure::Header);}
 {Fixture f;f.view.schema=0;assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.width=639;assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.layers.text.logicalWidth=343;assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.layers.text.vertexCount=690;assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.layers.text.vertexCount=std::numeric_limits<size_t>::max();assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.layers.text.vertices=nullptr;assert(f.result().failure==Failure::View);}
 {Fixture f;f.vertices.back().x=std::numeric_limits<float>::quiet_NaN();assert(f.result().failure==Failure::View);}
 {Fixture f;f.vertices[0].u=std::numeric_limits<float>::infinity();assert(f.result().failure==Failure::View);}
 {Fixture f;f.vertices[0].x=-1;assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.layers.dialogWords.schema=0;assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.layers.dialogWords.count=9;assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.layers.dialogWords.count=11;assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.layers.dialogWords.placements[9].right=641;assert(f.result().failure==Failure::View);}
 {Fixture f;f.view.layers.dialogWords.placements[9].left=std::numeric_limits<float>::quiet_NaN();assert(f.result().failure==Failure::View);}
 // Fractional placement includes floor/ceil plus the one-cell safety margin.
 {Fixture f;auto&p=f.view.layers.dialogWords.placements[9];p.left=310.5f;p.top=80.5f;p.right=311.5f;p.bottom=81.5f;f.side(309,79,0x7777);f.refresh();assert(f.result().failure==Failure::ProtectedBackground);}
}
