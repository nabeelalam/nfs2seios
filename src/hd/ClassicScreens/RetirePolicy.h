// PRIVATE pure source-derived retirement policy. No guest reads or authority issuer.
#pragma once
#include <array>
#include <cstdint>
namespace nfs_ls_retire {
constexpr uint32_t DescriptorDestroy=0x482da0,OwnerReplace=0x4ac934,DescriptorClear=0x4ac9c8;
struct Resources {std::array<uint32_t,3>owners{},bitmaps{};uint32_t font=0;std::array<uint32_t,6>luts{};};
struct Input {uint32_t site=0,handle=0,newPitch=0;std::array<uint32_t,14>descriptor{};std::array<uint32_t,2>refs{};};
struct Plan {bool valid=false;std::array<uint32_t,2>ordered{},before{},after{};unsigned count=0;};
inline Plan plan(const Input&i,const Resources&r)noexcept{
 Plan p;if(!i.handle||i.descriptor[0]!=0x57494e44u||i.descriptor[13]||!r.owners[0]||!r.owners[1]||!r.owners[2]||!r.font)return p;
 for(auto owner:r.owners)if(i.handle==owner)return p;
 if(i.handle==r.font)return p;
 for(auto bitmap:r.bitmaps)if(!bitmap||i.descriptor[8]==bitmap)return p; // Conservative borrowed-source alias rejection, NOT a proven bitmap free.
 if(i.descriptor[8]==r.font)return p;
 if(!i.descriptor[11]||!i.descriptor[12]||!i.refs[0]||!i.refs[1])return p;
 if(i.site!=DescriptorDestroy&&i.site!=DescriptorClear&&i.site!=OwnerReplace)return p;
 unsigned fields[2]={11,12},count=i.site==OwnerReplace?(i.newPitch!=i.descriptor[10]?1:0):2;
 for(unsigned k=0;k<count;++k){unsigned field=fields[k];uint32_t ptr=i.descriptor[field],ref=i.refs[field-11];bool protectedLut=false;for(auto lut:r.luts)if(lut&&ptr==lut)protectedLut=true;
  if(!ptr||!protectedLut||!ref)return Plan{};unsigned prior=0,total=0;for(unsigned j=0;j<count;++j)if(i.descriptor[fields[j]]==ptr){++total;if(j<k)++prior;}
  if(ref<=total)return Plan{}; // Includes duplicate-field last-reference and zero/underflow cases.
  p.ordered[k]=ptr;p.before[k]=ref-prior;p.after[k]=ref-prior-1;
 }
 p.count=count;p.valid=true;return p;
}
inline bool released(const Plan&p,unsigned index,uint32_t ptr,uint32_t before,uint32_t after)noexcept{return p.valid&&index<p.count&&ptr==p.ordered[index]&&before==p.before[index]&&after==p.after[index]&&after>0;}
struct FrameFlow {unsigned frames=0;bool redrawn=false,finished=false,lazy=false;void iteration()noexcept{redrawn=false;}void tail()noexcept{redrawn=true;}void completed()noexcept{if(frames<2)++frames;if(frames==2)finished=true;}bool genuineLazyEdge()noexcept{if(frames==1&&!redrawn&&!finished){lazy=finished=true;return true;}return false;}};
}
