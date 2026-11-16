// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
// Exact original405B60 mode0x2800 composition, before native widgets.
namespace nfs_setup_initial_full {
inline bool copied(const nfs_background_scene_cpu::Plane&base,const nfs_background_scene_cpu::Plane&side,const nfs_background_scene_cpu::Plane&after,uint32_t key)noexcept{
 namespace bg=nfs_background_scene_cpu;
 if(!bg::plane(base,640)||!bg::plane(side,343)||!bg::plane(after,640)||!nfs_logo_capture::same(base.target,after.target))return false;
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)if(after.pixels[y*640+x]!=(x<343?nfs_small_logo_live::pixel(base.pixels[y*640+x],nfs_small_logo_live::expanded(side.pixels[y*343+x],key),false):base.pixels[y*640+x]))return false;
 return true;
}
}
