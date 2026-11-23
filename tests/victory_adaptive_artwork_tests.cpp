// SPDX-License-Identifier: MIT
// Explicit private32-image preview; verifies rounded native canvases without pixel edits.
#include "hd/ClassicScreens/VictoryArtProvider.h"
#include "hd/ClassicScreens/VictoryArtworkArchive.h"
#include "hd/ClassicScreens/VictoryCanvasLayout.h"
#include "hd/ClassicScreens/VictoryOriginalArtwork.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
int main(int argc,char**argv){
 assert(argc==4);nfs_victory_art::ArchivePin pin;pin.fileBytes=std::strtoull(argv[2],nullptr,10);assert(std::strlen(argv[3])==64);
 for(unsigned i=0;i<32;++i){char text[3]={argv[3][2*i],argv[3][2*i+1],0};pin.metadataSha[i]=static_cast<unsigned char>(std::strtoul(text,nullptr,16));}
 assert(nfs_authored_wide_canvas(1672,941)&&nfs_authored_wide_canvas(1672,940));
 assert(!nfs_authored_wide_canvas(1672,942)&&!nfs_authored_wide_canvas(1672,939));
 assert(nfs_authored_wide_canvas(32,18)&&!nfs_authored_wide_canvas(33,19));
 assert(!nfs_authored_wide_canvas(641,360)&&!nfs_authored_wide_canvas(0,0)&&!nfs_authored_wide_canvas(4097,2304));
 assert(!nfs_victory_art::Archive::open(argv[1],pin));auto archive=nfs_victory_art::Archive::open(argv[1],pin,true);assert(archive&&archive->count()==32&&archive->draft());
 char error[128];nfs_victory_art_clear();assert(!nfs_victory_art_load_file(argv[1],pin.fileBytes,pin.metadataSha.data(),0,error,sizeof error));assert(nfs_victory_art_load_file(argv[1],pin.fileBytes,pin.metadataSha.data(),1,error,sizeof error));
 unsigned count=0,rounded=0;
 for(unsigned aspect=0;aspect<2;++aspect)for(unsigned image=0;image<16;++image){
  const auto*entry=archive->entry(aspect,image);assert(entry);NfsVictoryArtView view{};
  assert(nfs_victory_art_acquire(aspect,image,nfs_victory_original::sources[image].sha,&view));assert(view.aspect==aspect&&view.image==image&&view.width==entry->width&&view.height==entry->height&&view.bytes==size_t(view.width)*view.height*4);
  unsigned char sha[32];assert(nfs_texture_sha256(view.pixels,view.bytes,sha)&&!std::memcmp(sha,entry->rgbaSha.data(),32)&&!std::memcmp(sha,view.rgbaSha256,32));
  if(aspect&&view.width*9!=view.height*16)++rounded;
  view.release(view.owner);++count;
 }
 assert(count==32&&rounded==16);
 const int sizes[][2]={{708,492},{800,450},{2360,1640},{1920,1080},{3840,2160}};
 for(const auto&size:sizes){int viewport[4]={17,9,size[0],size[1]};NfsVictoryCanvasLayout layout{};assert(nfs_victory_canvas_layout_image(viewport,1672,941,&layout));
  assert(layout.aspect==1&&layout.leftU>=0&&layout.rightU<=1&&layout.topV>=0&&layout.bottomV<=1&&layout.leftU<layout.rightU&&layout.topV<layout.bottomV);
  const double sx=size[0]/(1672.0*(layout.rightU-layout.leftU)),sy=size[1]/(941.0*(layout.bottomV-layout.topV));assert(std::fabs(sx-sy)<.00001);
  const double fraction=(941.0*4/3)/1672.0,left=(1-fraction)/2,right=1-left;
  double px=17+(left-layout.leftU)/(layout.rightU-layout.leftU)*size[0],pr=17+(right-layout.leftU)/(layout.rightU-layout.leftU)*size[0];
  assert(std::fabs(px-layout.textViewport[0])<=1&&std::fabs(pr-layout.textViewport[0]-layout.textViewport[2])<=1);
 }
 int viewport[4]={0,0,800,450};NfsVictoryCanvasLayout sentinel{};sentinel.aspect=99;assert(!nfs_victory_canvas_layout_image(viewport,1672,942,&sentinel)&&sentinel.aspect==99);
 nfs_victory_art_clear();std::cout<<"Verified32real images unchanged,16rounded canvases, strict draft admission and5aspect-preserving crop mappings.\n";
}
