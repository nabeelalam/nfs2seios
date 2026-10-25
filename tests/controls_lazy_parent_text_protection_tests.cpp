// SPDX-License-Identifier: MIT
// Synthetic geometry/key checks only, without original assets or authority.
#include "hd/ControlsLazyParentTextProtection.h"
#include <array>
#include <cassert>
#include <limits>
using namespace nfs_controls_lazy_parent_text_protection;
using nfs_controls_lazy_parent_copy::Geometry;
struct Fixture {
 Geometry g{0,0,0,0,343,480};std::vector<uint8_t>sidebar=std::vector<uint8_t>(nfs_controls_lazy_parent_copy::SourceBytes,0);
 std::array<NfsSoftwareTextVertex,696>vertices{};NfsControlsSceneFrameView view{};uint32_t key=0;
 Fixture(){view.schema=NFS_CONTROLS_SCENE_RUNTIME_SCHEMA;view.width=640;view.height=480;view.layers.text.logicalWidth=640;view.layers.text.logicalHeight=480;view.layers.text.vertices=vertices.data();view.layers.text.vertexCount=vertices.size();view.layers.dialogWords.schema=NFS_CONTROLS_DIALOG_WORD_SCHEMA;view.layers.dialogWords.count=NFS_CONTROLS_DIALOG_WORD_PLACEMENTS;
  for(size_t i=0;i<vertices.size();i+=6)quad(i,300,100,302,102);
  for(auto&p:view.layers.dialogWords.placements){p.word=1;p.left=400;p.top=200;p.right=420;p.bottom=232;}
 }
 void quad(size_t first,float l,float t,float r,float b){const float xy[6][2]={{l,t},{r,t},{l,b},{l,b},{r,t},{r,b}};for(unsigned j=0;j<6;++j){auto&v=vertices[first+j];v={};v.x=xy[j][0];v.y=xy[j][1];v.r=v.g=v.b=v.a=1;}}
 void pixel(int x,int y,uint16_t value){const size_t at=16+(size_t(y-g.y)*343+size_t(x-g.x))*2;sidebar[at]=uint8_t(value);sidebar[at+1]=uint8_t(value>>8);}
 bool ok()const{return preserved(g,sidebar,key,view);}
};
int main(){
 {Fixture f;assert(f.ok());}
 {Fixture f;f.pixel(300,100,1);assert(!f.ok());}
 {Fixture f;f.pixel(299,99,1);assert(!f.ok());} // One-cell margin.
 {Fixture f;f.pixel(298,98,1);assert(f.ok());}
 {Fixture f;f.pixel(1,1,1);assert(f.ok());} // Opaque outside protected text.
 {Fixture f;f.key=0x10000;assert(!f.ok());}
 {Fixture f;f.key=0x1234;for(size_t at=16;at<f.sidebar.size();at+=2){f.sidebar[at]=0x34;f.sidebar[at+1]=0x12;}assert(f.ok());f.pixel(301,101,0);assert(!f.ok());}
 {Fixture f;f.g={-167,0,0,0,176,480};f.quad(0,100,100,101,101);assert(f.ok());f.pixel(100,100,1);assert(!f.ok());}
 {Fixture f;f.g={-167,-20,13,7,176,460};f.quad(0,13,7,14,8);assert(f.ok());f.pixel(13,7,1);assert(!f.ok());}
 {Fixture f;f.g={0,0,0,0,343,480};f.quad(0,0,0,1,1);assert(f.ok());f.pixel(0,0,1);assert(!f.ok());}
 {Fixture f;f.g={297,0,297,0,640,480};f.quad(0,639,479,640,480);assert(f.ok());f.pixel(639,479,1);assert(!f.ok());}
 {Fixture f;auto&p=f.view.layers.dialogWords.placements[9];p.left=310.5f;p.top=80.5f;p.right=311.5f;p.bottom=81.5f;assert(f.ok());f.pixel(309,79,1);assert(!f.ok());}
 {Fixture f;f.view.schema=0;assert(!f.ok());}
 {Fixture f;f.view.width=639;assert(!f.ok());}
 {Fixture f;f.view.height=479;assert(!f.ok());}
 {Fixture f;f.view.layers.text.logicalWidth=343;assert(!f.ok());}
 {Fixture f;f.view.layers.text.logicalHeight=479;assert(!f.ok());}
 {Fixture f;f.view.layers.text.vertices=nullptr;assert(!f.ok());}
 for(size_t n:{size_t(0),size_t(690),size_t(695),size_t(697),std::numeric_limits<size_t>::max()}){Fixture f;f.view.layers.text.vertexCount=n;assert(!f.ok());}
 {Fixture f;f.vertices.back().x=std::numeric_limits<float>::quiet_NaN();assert(!f.ok());}
 {Fixture f;f.vertices[0].u=std::numeric_limits<float>::infinity();assert(!f.ok());}
 {Fixture f;f.vertices[0].a=-1;assert(!f.ok());}
 {Fixture f;f.vertices[0].x=-0.1f;assert(!f.ok());}
 {Fixture f;f.quad(0,10,10,10,11);assert(!f.ok());}
 {Fixture f;f.view.layers.dialogWords.schema=0;assert(!f.ok());}
 {Fixture f;f.view.layers.dialogWords.count=9;assert(!f.ok());}
 {Fixture f;f.view.layers.dialogWords.count=11;assert(!f.ok());}
 {Fixture f;f.view.layers.dialogWords.placements[9].left=std::numeric_limits<float>::quiet_NaN();assert(!f.ok());}
 {Fixture f;f.view.layers.dialogWords.placements[0].right=641;assert(!f.ok());}
 {Fixture f;f.view.layers.dialogWords.placements[0].top=233;assert(!f.ok());}
 {Fixture f;f.sidebar.pop_back();assert(!f.ok());}
 {Fixture f;f.sidebar.push_back(0);assert(!f.ok());}
 {Fixture f;f.g.x=-2049;assert(!f.ok());}
 {Fixture f;f.g.y=2048;assert(!f.ok());}
 {Fixture f;f.g.left=-1;assert(!f.ok());}
 {Fixture f;f.g.right=344;assert(!f.ok());}
 {Fixture f;f.g.bottom=481;assert(!f.ok());}
 {Fixture f;f.g.top=f.g.bottom;assert(!f.ok());}
 {Fixture f;f.g.x=1;assert(!f.ok());}
 {Fixture f;f.g.y=1;assert(!f.ok());}
 {Fixture f;f.g.right=641;assert(!f.ok());}
}
