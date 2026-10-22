// SPDX-License-Identifier: MIT
#pragma once
#include "AudioBackgroundProof.h"

// Pure Graphics complete-frame evidence. A future source-bound adapter must prove
// exact original call scopes, live owners and event order before supplying these
// observations. This verifier proves full-plane transformations, not row-callback
// authority. It grants no guest access, GPU substitution or HD activation.
namespace nfs_graphics_frame {
namespace af=nfs_audio_frame;
constexpr size_t Pixels=640*480,SidebarBytes=16+343*480*2;

struct SidebarBinding {
 af::Target target{};
 uint64_t lifetime=0;
 uint32_t handle=0,tableEntry=0,word=0;
 std::vector<uint8_t>source;
 bool valid()const noexcept {
  // Slots1/2 own the retained sidebar windows; slot3 is the full640 owner.
  // The ambient selector may flip after the caption; current tableEntry is independent.
  if((int32_t(word)>>16)<0||(int32_t(word)>>16)>1)return false;
  if(!lifetime||!handle||tableEntry!=handle||target.owner!=handle||
     !nfs_small_logo_live::target(target,343)||
     (target.ownerState[1]!=target.owner&&target.ownerState[2]!=target.owner)||
     target.ownerState[0]!=word||
     target.ownerState[1+(uint32_t(word)>>16)]!=handle||
     target.w[3]||target.w[4]||target.w[5]!=343||target.w[6]!=480||
     target.header!=std::array<uint32_t,4>{{0x78,343u|(480u<<16),0,0}}||
     source.size()!=SidebarBytes)return false;
  for(unsigned k=0;k<4;++k)if(nfs_small_logo_live::le32(source.data()+4*k)!=target.header[k])return false;
  return true;
 }
 bool operator==(const SidebarBinding&o)const noexcept {
  return lifetime==o.lifetime&&handle==o.handle&&tableEntry==o.tableEntry&&word==o.word&&
   nfs_logo_capture::same(target,o.target)&&source==o.source;
 }
};
struct ScopedDraw {
 af::Binding entry{},exit{};
 af::Target before{},after{};
 uint64_t begin=0,end=0;
 uint32_t complete=0,error=0;
 std::array<uint32_t,4>args{};
 std::vector<uint8_t>sourceBefore,sourceAfter;
};
struct BackgroundRecord {
 af::Binding binding{};
 uint64_t begin=0,end=0;
 uint32_t complete=0,error=0;
 ScopedDraw reset,sidebar;
 SidebarBinding sidebarBefore{},sidebarAfter{};
 std::vector<uint16_t>preReset,postReset,postSidebar;
};
class BackgroundVerifier;
class VerifiedBackground final {
 af::Binding binding_{};
 std::shared_ptr<const af::BackgroundCatalog>catalog_;
 SidebarBinding sidebar_;
 std::vector<uint16_t>pixels_;
 VerifiedBackground()=default;
 friend class BackgroundVerifier;
public:
 const af::Binding&binding()const noexcept{return binding_;}
 const std::shared_ptr<const af::BackgroundCatalog>&catalog()const noexcept{return catalog_;}
 const SidebarBinding&sidebar()const noexcept{return sidebar_;}
 const std::vector<uint16_t>&pixels()const noexcept{return pixels_;}
};
class BackgroundVerifier final {
 static bool draw(const ScopedDraw&d,const af::Binding&b,uint32_t mode,size_t bytes)noexcept {
  return d.complete==1&&!d.error&&d.begin&&d.begin<d.end&&d.entry==b&&d.exit==b&&
   af::targetValid(d.before,b)&&d.before.ownerState[3]==b.owner&&
   nfs_logo_capture::same(d.before,d.after)&&d.args[0]&&d.args[1]==mode&&!d.args[2]&&!d.args[3]&&
   d.sourceBefore.size()==bytes&&d.sourceBefore==d.sourceAfter&&
   uint64_t(d.args[0])+bytes<=0x100000000ULL&&
   !nfs_small_logo_live::overlap(d.args[0],uint32_t(bytes),b.surface.geometry[8],614416);
 }
public:
 // current and sidebarCurrent are independent observations of the live owner,
 // never reconstructed from candidate getters. All original calls still execute.
 static std::shared_ptr<const VerifiedBackground>verify(const BackgroundRecord&r,
   const af::Binding&current,const SidebarBinding&sidebarCurrent,
   const std::shared_ptr<const af::BackgroundCatalog>&catalog)noexcept {
  if(!current.valid()||!catalog||!(r.binding==current)||r.complete!=1||r.error||
     !r.begin||!(r.begin<r.reset.begin&&r.reset.end<r.sidebar.begin&&r.sidebar.end<r.end)||
     r.preReset.size()!=Pixels||r.postReset.size()!=Pixels||r.postSidebar.size()!=Pixels||
     !sidebarCurrent.valid()||sidebarCurrent.target.owner==current.owner||!(r.sidebarBefore==sidebarCurrent)||!(r.sidebarAfter==sidebarCurrent)||
     !draw(r.reset,current,0x400,614416)||!draw(r.sidebar,current,0x1000,SidebarBytes)||
     !nfs_logo_capture::same(r.reset.after,r.sidebar.before)||
     r.reset.sourceBefore!=catalog->source()||r.sidebar.sourceBefore!=sidebarCurrent.source||
     r.sidebar.args[0]!=sidebarCurrent.target.w[8]||
     nfs_small_logo_live::overlap(r.reset.args[0],614416,r.sidebar.args[0],SidebarBytes))return {};
  // The two complete immutable source spans are checked before any replay.
  // Reset overwrites every RGB565 word. Sidebar mode copies nonzero words into
  // the left343 columns; zero is transparent and every other column is retained.
  for(size_t y=0;y<480;++y)for(size_t x=0;x<640;++x){
   const size_t at=y*640+x;
   const uint16_t map=nfs_small_logo_live::le16(catalog->source().data()+16+at*2);
   uint16_t expected=map;
   if(x<343){const uint16_t s=nfs_small_logo_live::le16(sidebarCurrent.source.data()+16+(y*343+x)*2);if(s)expected=s;}
   if(r.postReset[at]!=map||r.postSidebar[at]!=expected)return {};
  }
  try{auto p=std::shared_ptr<VerifiedBackground>(new VerifiedBackground);
   p->binding_=current;p->catalog_=catalog;p->sidebar_=sidebarCurrent;p->pixels_=r.postSidebar;return p;
  }catch(...){return {};}
 }
};
class GraphicsFramePacket final {
 af::Binding binding_{};
 uint64_t serial_=0;
 std::shared_ptr<const VerifiedBackground>background_;
 std::shared_ptr<const nfs_soft::FramePacket>text_;
 GraphicsFramePacket()=default;
public:
 static std::shared_ptr<const GraphicsFramePacket>create(
   const std::shared_ptr<const VerifiedBackground>&background,
   const std::shared_ptr<const nfs_soft::FramePacket>&text,const af::Binding&current,
   uint64_t acceptedTextSerial,const uint16_t*currentPost,size_t count)noexcept {
  if(!current.valid()||!background||!text||!(background->binding()==current)||
     !acceptedTextSerial||!currentPost||count!=Pixels||background->pixels()!=text->clean()||
     !text->font()||!text->commands()||!text->writes()||text->vertices().empty()||text->pixels()!=Pixels||
     !text->matches(current.surface,acceptedTextSerial,currentPost,count))return {};
  try{auto p=std::shared_ptr<GraphicsFramePacket>(new GraphicsFramePacket);
   p->binding_=current;p->serial_=acceptedTextSerial;p->background_=background;p->text_=text;return p;
  }catch(...){return {};}
 }
 const af::Binding&binding()const noexcept{return binding_;}
 uint64_t publication()const noexcept{return serial_;}
 const std::shared_ptr<const VerifiedBackground>&background()const noexcept{return background_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&text()const noexcept{return text_;}
 bool matches(const af::Binding&current,uint64_t serial,const uint16_t*post,size_t count)const noexcept {
  return current==binding_&&serial==serial_&&text_->matches(current.surface,serial,post,count);
 }
};
}
