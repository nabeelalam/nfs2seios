// SPDX-License-Identifier: MIT
#include "hd/PlayerIntroNativePass.h"
#include <array>
#include <cassert>
#include <cstdio>
#include <limits>

static NfsAudioNativeStamp stamp{7,9,11,13,640,480};
static std::array<uint32_t,300> keys(), ids();
static std::array<uint32_t,300> keys(){std::array<uint32_t,300>a{};for(unsigned i=0;i<300;++i)a[i]=i*2048;return a;}
static std::array<uint32_t,300> ids(){std::array<uint32_t,300>a{};for(unsigned i=0;i<300;++i)a[i]=i+1;return a;}
static NfsAudioNativeVertex corner(unsigned tile,unsigned c){return {float((tile/15)*32+(c==1||c==2?32:0))+NFS_AUDIO_NATIVE_VERTEX_SNAP,float((tile%15)*32+(c>=2?32:0))+NFS_AUDIO_NATIVE_VERTEX_SNAP,255,255,255,255,1,float(c==1||c==2?255:0),float(c>=2?255:0)};}
static void begin(NfsPlayerIntroNativePass&p){auto k=keys(),i=ids();assert(nfs_player_intro_pass_begin(&p,&stamp,17,19,k.data(),i.data()));}
static bool select(NfsPlayerIntroNativePass&p,unsigned tile){return nfs_player_intro_pass_select(&p,&stamp,19,0,tile*2048,tile+1,3,11,32,3,3,3);}
static bool triangle(NfsPlayerIntroNativePass&p,unsigned tile,unsigned half){auto a=corner(tile,0),b=corner(tile,half?2:1),c=corner(tile,half?3:2);return nfs_player_intro_pass_triangle(&p,&stamp,19,&a,&b,&c,1,0);}
int main(){
 NfsPlayerIntroNativePass p{};begin(p);
 for(unsigned t=0;t<300;++t){assert(select(p,t));assert(triangle(p,t,0));assert(triangle(p,t,1));}
 assert(nfs_player_intro_pass_finish(&p,&stamp,19,17));assert(!p.active);assert(!nfs_player_intro_pass_finish(&p,&stamp,19,17));
 begin(p);assert(!nfs_player_intro_pass_finish(&p,&stamp,19,17));
 begin(p);assert(!triangle(p,0,0));
 begin(p);assert(select(p,0));assert(!select(p,1));
 begin(p);assert(!select(p,1));
 for(unsigned what=0;what<8;++what){begin(p);assert(select(p,0));auto a=corner(0,0),b=corner(0,1),c=corner(0,2);if(what==0)a.oow=.5f;if(what==1)b.x+=1;if(what==2)c.sow=0;if(what==3)a.a=140;if(what==4)a.r=140;if(what==5)a.x=std::numeric_limits<float>::quiet_NaN();assert(!nfs_player_intro_pass_triangle(&p,&stamp,19,&a,&b,&c,what==6?0:1,what==7?1:0));assert(!p.active);}
 for(unsigned what=0;what<7;++what){begin(p);auto s=stamp;if(what==0)++s.generation;if(what==1)++s.context;if(what==2)++s.thread;if(what==3)++s.framebuffer;if(what==4)++s.width;if(what==5)++s.height;assert(!nfs_player_intro_pass_select(&p,&s,what==6?20:19,0,0,1,3,11,32,3,3,3));}
 for(unsigned what=0;what<5;++what){auto k=keys(),i=ids();if(what==0)k[1]=k[0];if(what==1)k[1]=k[0]+4;if(what==2)i[1]=i[0];if(what==3)i[1]=0;if(what==4)k[299]=0xfffffffcu;assert(!nfs_player_intro_pass_begin(&p,&stamp,17,19,k.data(),i.data()));}
 for(unsigned what=0;what<5;++what){begin(p);assert(!nfs_player_intro_pass_select(&p,&stamp,19,what==0?1:0,0,1,what==1?1:3,what==2?10:11,what==3?64:32,3,3,what==4?2:3));}
 std::puts("PASS independent Player slot0 pass: complete 300 tiles/600 triangles; rejects wrong source/order/geometry/owner/revision; grants no GL authority");
}
