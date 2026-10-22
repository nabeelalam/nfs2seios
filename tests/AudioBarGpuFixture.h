// SPDX-License-Identifier: MIT
#pragma once
#include "HdSceneGlesFixture.h"
#include "hd/AudioLayerAbi.h"
#include <array>
#include <thread>
namespace nfs_audio_bar_test {
struct Owner{EGLContext context;uint64_t generation;std::thread::id thread=std::this_thread::get_id();};
inline int current(void*u,uint64_t g,uintptr_t c){auto&o=*static_cast<Owner*>(u);return o.thread==std::this_thread::get_id()&&o.generation==g&&reinterpret_cast<uintptr_t>(o.context)==c&&eglGetCurrentContext()==o.context;}
inline std::array<NfsAudioBarVertex,78>vertices(const std::array<unsigned,3>&values={{140,140,140}},int selected=0,int held=0){
 std::array<NfsAudioBarVertex,78>v{};
 const unsigned region[12][2]={{4,4},{1106,4},{1106,72},{1360,72},{1596,72},{1106,236},{4,388},{1106,388},{1106,456},{1360,456},{1596,456},{1106,620}};
 auto q=[&](unsigned at,unsigned role,bool dim,float l,float t,float r,float b,float ol,float ot){const auto*z=region[role+(dim?6:0)];const float u0=(z[0]+(l-ol)*6)/2048.f,u1=(z[0]+(r-ol)*6)/2048.f,v0=(z[1]+(t-ot)*6)/768.f,v1=(z[1]+(b-ot)*6)/768.f;v[at]={l,t,u0,v0};v[at+1]={r,t,u1,v0};v[at+2]={l,b,u0,v1};v[at+3]=v[at+2];v[at+4]=v[at+1];v[at+5]={r,b,u1,v1};};
 for(unsigned r=0;r<3;++r){const float y=147.f+27*r;const bool dim=selected!=int(r);q(r*6,0,dim,409,y,592,y+31,409,y);const float fy=y+6,l=269.f+values[r];q((3+r)*6,1,dim,l<392?392:l,fy,425.f+values[r],fy+10,l,fy);q((6+r*2)*6,2,false,388,y,429,y+26,388,y);q((7+r*2)*6,3,dim,554,y,592,y+23,554,y);}
 if(held&&selected>=0){const float y=147.f+27*unsigned(selected);if(held==1)q(72,4,false,388,y,426,y+23,388,y);else q(72,5,false,554,y,592,y+24,554,y);}return v;
}
}
