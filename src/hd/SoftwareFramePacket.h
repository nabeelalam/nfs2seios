// SPDX-License-Identifier: MIT
// Original-preserved software text packet. No game, GL or guest pointers.
#pragma once
#include "SoftwareTextVertices.h"
#include <array>
#include <memory>
#include <vector>
#include <cstring>
#include <limits>
namespace nfs_soft {
struct SurfaceIdentity {
 uint64_t lifetime=0;
 std::array<uint32_t,20> geometry{};
 // Caller must additionally validate every interior LUT entry before constructing this identity.
 bool valid() const noexcept {return lifetime&&geometry[1]==640&&geometry[2]==480&&(geometry[7]&255)==16&&geometry[8]&&uint64_t(geometry[8])+16+640*480*2<=0x100000000ULL&&geometry[10]==1280&&geometry[11]&&geometry[12]&&geometry[3]<=geometry[5]&&geometry[4]<=geometry[6]&&geometry[5]<=640&&geometry[6]<=480&&geometry[14]==0&&geometry[15]==2&&geometry[16]==1278&&geometry[17]==16&&geometry[18]==1296&&geometry[19]==16+479*1280;}
 bool operator==(const SurfaceIdentity&o)const noexcept{return lifetime==o.lifetime&&geometry==o.geometry;}
};
struct TextCommand {
 std::array<uint8_t,96> text{};
 uint32_t length=0,role=0,alignment=0,sourceSize=0;
 int32_t x=0,y=0,originX=0,originY=0,advance=0;
 uint16_t color=0;
 Clip clip{0,0,640,480};
};
class FramePacket {
 std::shared_ptr<const FontCatalog> font_;SurfaceIdentity surface_;uint64_t serial_=0;
 std::vector<uint16_t> clean_,original_;
 std::vector<Vertex> vertices_;
 size_t commands_=0,writes_=0;
 friend class FramePacketBuilder;
 FramePacket(std::shared_ptr<const FontCatalog> font,SurfaceIdentity surface,uint64_t serial,std::vector<uint16_t>&&clean,std::vector<uint16_t>&&original,std::vector<Vertex>&&vertices,size_t commands,size_t writes):font_(std::move(font)),surface_(surface),serial_(serial),clean_(std::move(clean)),original_(std::move(original)),vertices_(std::move(vertices)),commands_(commands),writes_(writes){}
 public:
 static constexpr size_t pixelCount=640*480;
 const std::shared_ptr<const FontCatalog>&font()const noexcept{return font_;}
 const SurfaceIdentity&surface()const noexcept{return surface_;}
 uint64_t serial()const noexcept{return serial_;}
 const std::vector<uint16_t>&clean()const noexcept{return clean_;}
 const std::vector<uint16_t>&original()const noexcept{return original_;}
 const std::vector<Vertex>&vertices()const noexcept{return vertices_;}
 size_t commands()const noexcept{return commands_;}
 size_t writes()const noexcept{return writes_;}
 bool matches(const SurfaceIdentity&current,uint64_t acceptedSerial,const uint16_t*source,size_t count)const noexcept{
  return acceptedSerial==serial_&&surface_==current&&source&&count==pixelCount&&std::memcmp(original_.data(),source,pixelCount*sizeof(uint16_t))==0;
 }
};
class FramePacketBuilder {
 public:
 enum class Failure {none,invalidBegin,notActive,command,capacity,wordMismatch,sourceMismatch,unclosedString,allocation,identity};
 static constexpr size_t maxCommands=1024,maxVertices=12288,maxWrites=524288;
 private:
 std::shared_ptr<const FontCatalog> font_;SurfaceIdentity surface_;uint64_t serial_=0,lastSerial_=0;bool active_=false,stringActive_=false;
 Failure failure_=Failure::notActive;
 std::vector<uint16_t> clean_,replay_;
 std::vector<Vertex> vertices_;
 size_t used_=0,commands_=0,writes_=0,stringStartWrites_=0;
 TextCommand entry_{};
 static bool sameEntry(const TextCommand&a,const TextCommand&b)noexcept{return a.text==b.text&&a.length==b.length&&a.role==b.role&&a.alignment==b.alignment&&a.sourceSize==b.sourceSize&&a.x==b.x&&a.y==b.y&&a.originX==b.originX&&a.originY==b.originY&&a.color==b.color&&a.clip.left==b.clip.left&&a.clip.top==b.clip.top&&a.clip.right==b.clip.right&&a.clip.bottom==b.clip.bottom;}
 bool fail(Failure why)noexcept{active_=false;stringActive_=false;failure_=why;return false;}
 public:
 explicit FramePacketBuilder(std::shared_ptr<const FontCatalog> font)noexcept:font_(std::move(font)){}
 bool active()const noexcept{return active_;}
 Failure failure()const noexcept{return failure_;}
 bool begin(SurfaceIdentity surface,uint64_t serial,const uint16_t*pre,size_t count)noexcept{
  if(!font_||active_||!surface.valid()||!serial||serial<=lastSerial_||!pre||count!=FramePacket::pixelCount)return fail(Failure::invalidBegin);
  lastSerial_=serial;surface_=surface;serial_=serial;used_=commands_=writes_=0;stringActive_=false;
  try{clean_.assign(pre,pre+count);replay_=clean_;vertices_.resize(maxVertices);}catch(...){return fail(Failure::allocation);}
  active_=true;failure_=Failure::none;return true;
 }
 // entry is copied from the exact original call before its body; advance is determined on exit.
 bool beginString(const TextCommand&entry)noexcept{
  if(!active_)return false;
  if(stringActive_)return fail(Failure::unclosedString);
  if(commands_>=maxCommands)return fail(Failure::capacity);
  entry_=entry;stringStartWrites_=writes_;stringActive_=true;return true;
 }
 bool word(size_t index,uint16_t before,uint16_t computed,uint16_t after)noexcept{
  if(!active_)return false;
  if(!stringActive_)return fail(Failure::unclosedString);
  if(index>=FramePacket::pixelCount||writes_>=maxWrites)return fail(Failure::capacity);
  if(computed!=after||replay_[index]!=before)return fail(Failure::wordMismatch);
  replay_[index]=after;++writes_;return true;
 }
 bool endString(const TextCommand&c)noexcept{
  if(!active_)return false;
  if(!stringActive_)return fail(Failure::unclosedString);
  if(!sameEntry(entry_,c)||writes_==stringStartWrites_||!c.length||c.length>95||c.role>1||c.alignment>2||c.sourceSize!=18||c.advance<=0||c.advance>4096)return fail(Failure::command);
  for(size_t i=0;i<c.length;++i)if(!c.text[i])return fail(Failure::command);
  for(size_t i=c.length;i<c.text.size();++i)if(c.text[i])return fail(Failure::command);
  const int64_t px=int64_t(c.x)+c.originX,py=int64_t(c.y)+c.originY+24-font_->description().baseline[c.role];
  if(px < -8192||px>8192||py < -8192||py>8192)return fail(Failure::command);
  Layout shaped;
  if(!layout(*font_,c.text.data(),c.length+1,c.role,float(px),float(py),float(c.advance),c.alignment,c.clip,shaped))return fail(Failure::command);
  if(!append_vertices(shaped,c.color,vertices_.data(),vertices_.size(),used_))return fail(Failure::capacity);
  ++commands_;stringActive_=false;return true;
 }
 void invalidate(Failure why=Failure::identity)noexcept{fail(why);}
 std::shared_ptr<const FramePacket> finish(SurfaceIdentity current,const uint16_t*post,size_t count)noexcept{
  if(!active_)return {};
  if(stringActive_){fail(Failure::unclosedString);return {};}
  if(!(current==surface_)){fail(Failure::identity);return {};}
  if(!post||count!=FramePacket::pixelCount||!commands_||!writes_||!used_){fail(Failure::sourceMismatch);return {};}
  if(std::memcmp(replay_.data(),post,count*sizeof(uint16_t))){fail(Failure::sourceMismatch);return {};}
  try{
   vertices_.resize(used_);
   auto packet=std::shared_ptr<const FramePacket>(new FramePacket(font_,surface_,serial_,std::move(clean_),std::move(replay_),std::move(vertices_),commands_,writes_));
   active_=false;failure_=Failure::none;return packet;
  }catch(...){fail(Failure::allocation);return {};}
 }
};
}
