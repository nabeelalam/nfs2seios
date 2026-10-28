// PRIVATE original-computed graph geometry; no statistics are recalculated.
#pragma once
#include "StatsRuntimeAbi.h"
#include "hd/SoftwareFramePacket.h"
#include "hd/OriginalUiRecords.h"
#include <array>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
namespace nfs_stats_ui {
constexpr uint32_t FormSymbol=0x4d2824,HandlerSymbol=0x426050;
constexpr size_t Pixels=640*480,MaxPrimitives=1024,MaxCommands=256;
enum class Kind:unsigned {line=0,rectangle=1,dot=2};
struct Primitive {Kind kind=Kind::line;float x0=0,y0=0,x1=0,y1=0,radius=.55f;uint32_t argb=0;std::array<float,4>clip{{0,0,640,480}};};
struct Event {bool text=false;unsigned index=0;};
struct Frame {
 uintptr_t game=0,thread=0;uint32_t form=0,archive=0;uint64_t serial=0,epoch=0;uint32_t handlerStack=0;NfsRecordsBackdropView backdrop{};
 nfs_logo_capture::Target target{};std::vector<uint16_t>clean,original;
 std::vector<Event>events;std::vector<Primitive>primitives;std::vector<nfs_soft::TextCommand>commands;
};
inline uint32_t rgb565(uint16_t p)noexcept{unsigned r=(p>>11)&31,g=(p>>5)&63,b=p&31;return 0xff000000u|(((r<<3)|(r>>2))<<16)|(((g<<2)|(g>>4))<<8)|((b<<3)|(b>>2));}
inline bool append(const Primitive&p,std::vector<NfsStatsGraphVertex>&out)noexcept{
 try{if(out.size()>=MaxPrimitives*6||unsigned(p.kind)>2||!std::isfinite(p.radius)||p.radius<0||p.radius>64)return false;
  for(float f:{p.x0,p.y0,p.x1,p.y1})if(!std::isfinite(f)||f<-8192||f>8192)return false;
  if(p.clip[0]<0||p.clip[1]<0||p.clip[2]>640||p.clip[3]>480||p.clip[0]>=p.clip[2]||p.clip[1]>=p.clip[3])return false;
  const float pad=p.kind==Kind::rectangle?.75f:p.radius+1.f;
  float l=std::max(p.clip[0],std::min(p.x0,p.x1)-pad),t=std::max(p.clip[1],std::min(p.y0,p.y1)-pad),r=std::min(p.clip[2],std::max(p.x0,p.x1)+pad),b=std::min(p.clip[3],std::max(p.y0,p.y1)+pad);
  if(l>=r||t>=b)return true;
  const float xy[6][2]={{l,t},{l,b},{r,t},{r,t},{l,b},{r,b}};
  for(auto&v:xy){NfsStatsGraphVertex q{};q.vertex[0]=v[0];q.vertex[1]=v[1];q.vertex[2]=p.x0;q.vertex[3]=p.y0;q.shape[0]=p.x1;q.shape[1]=p.y1;q.shape[2]=p.radius;q.shape[3]=float(unsigned(p.kind));q.color[0]=float((p.argb>>16)&255)/255;q.color[1]=float((p.argb>>8)&255)/255;q.color[2]=float(p.argb&255)/255;q.color[3]=float(p.argb>>24)/255;std::copy(p.clip.begin(),p.clip.end(),q.clip);out.push_back(q);}return true;
 }catch(...){return false;}
}
}
