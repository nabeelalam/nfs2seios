// SPDX-License-Identifier: MIT
// Pure integer geometry only; no original data, game or physical-device claims.
#include "hd/FeedbackModalGeometry.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <initializer_list>
namespace modal=nfs_feedback_modal_geometry;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"FAIL feedback geometry line%d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
static bool same(const modal::Geometry&a,const modal::Geometry&b){return a.width==b.width&&a.height==b.height&&a.left==b.left&&a.top==b.top&&a.titleAnchor==b.titleAnchor&&a.titleY==b.titleY&&a.messageAnchor==b.messageAnchor&&a.messageY==b.messageY;}
// Independent expanded original register sequence: MOV EAX,v; MOV EDX,EAX;
// SAR EDX,31; SUB EAX,EDX; SAR EAX,1. Unsigned bits avoid relying on a host's
// negative signed right-shift behavior or implementation-defined signed casts.
static int32_t originalHalf(int32_t value){uint32_t eax=uint32_t(value),edx=(eax&0x80000000u)?0xffffffffu:0;eax-=edx;eax=(eax>>1)|(eax&0x80000000u);return int32_t((eax&0x80000000u)?int64_t(eax)-INT64_C(4294967296):int64_t(eax));}
static modal::Geometry original(int32_t sw,int32_t sh,int32_t title,int32_t message){int32_t edi=title;edi+=0x3c;if(message>edi)edi=message;edi+=0x1e;int32_t esi=0x1e;esi+=0x1e;int32_t eax=sw;eax-=edi;const int32_t left=originalHalf(eax);eax=sh;eax-=esi;const int32_t top=originalHalf(eax);int32_t titleAnchor=edi;titleAnchor-=0x3c;titleAnchor=originalHalf(titleAnchor);titleAnchor+=0x3c;titleAnchor+=left;int32_t messageAnchor=originalHalf(edi);messageAnchor+=left;return {edi,esi,left,top,titleAnchor,top-0x19,messageAnchor,top+0xf};}
static void concrete(){
 modal::Geometry result;
 // Genuine observed Feedback301: title96, message108 on original640x480.
 CHECK(modal::make(640,480,96,108,result));CHECK(same(result,{186,60,227,210,350,185,320,225}));
 CHECK(modal::make(640,480,100,120,result));CHECK(same(result,{190,60,225,210,350,185,320,225}));
 CHECK(modal::make(641,481,101,122,result));CHECK(same(result,{191,60,225,210,350,185,320,225}));
 CHECK(modal::make(640,480,30,201,result));CHECK(same(result,{231,60,204,210,349,185,319,225}));
 // Negative odd deltas truncate toward zero, not toward negative infinity.
 CHECK(modal::make(10,9,20,100,result));CHECK(same(result,{130,60,-60,-25,35,-50,5,-10}));
 CHECK(modal::make(9,9,20,100,result));CHECK(same(result,{130,60,-60,-25,35,-50,5,-10}));
 CHECK(modal::make(1,1,0,0,result));CHECK(same(result,{90,60,-44,-29,31,-54,1,-14}));
 // The comparison includes title's extra60 before adding the shared30 padding.
 CHECK(modal::make(640,480,40,100,result));CHECK(same(result,{130,60,255,210,350,185,320,225}));
 for(int32_t value:{-101,-100,-3,-2,-1,0,1,2,3,100,101})CHECK(originalHalf(value)==value/2);
}
static void boundaries(){
 const int32_t hi=std::numeric_limits<int32_t>::max(),lo=std::numeric_limits<int32_t>::min();
 const modal::Geometry marker{11,12,13,14,15,16,17,18};modal::Geometry g=marker;
 for(int32_t bad:{lo,-1,0}){CHECK(!modal::make(bad,480,1,1,g)&&same(g,marker));CHECK(!modal::make(640,bad,1,1,g)&&same(g,marker));}
 for(int32_t bad:{lo,-1}){CHECK(!modal::make(640,480,bad,1,g)&&same(g,marker));CHECK(!modal::make(640,480,1,bad,g)&&same(g,marker));}
 for(int32_t bad:{hi-89,hi-60,hi})CHECK(!modal::make(640,480,bad,0,g)&&same(g,marker));
 for(int32_t bad:{hi-29,hi})CHECK(!modal::make(640,480,0,bad,g)&&same(g,marker));
 CHECK(modal::make(1,1,hi-90,0,g));CHECK(g.width==hi&&g.left==-1073741823&&g.top==-29&&g.titleAnchor==30&&g.messageAnchor==0);
 CHECK(modal::make(hi,hi,0,hi-30,g));CHECK(g.width==hi&&g.left==0&&g.top==1073741793&&g.titleAnchor==1073741853&&g.messageAnchor==1073741823);
 CHECK(modal::make(hi,hi,0,0,g));CHECK(g.left==1073741778&&g.top==1073741793&&g.titleY==1073741768&&g.messageY==1073741808);
 CHECK(originalHalf(lo)==-1073741824&&originalHalf(hi)==1073741823);
}
static void exhaustive(){
 for(int32_t sw=1;sw<=64;++sw)for(int32_t sh=1;sh<=16;++sh)for(int32_t title=0;title<=32;++title)for(int32_t message=0;message<=64;++message){modal::Geometry g;CHECK(modal::make(sw,sh,title,message,g));CHECK(same(g,original(sw,sh,title,message)));}
}
int main(){concrete();boundaries();exhaustive();std::printf("PASS feedback modal geometry checks=%u pure-arithmetic=1 source-native-authority=none\n",checks);}
