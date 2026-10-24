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
 bool valid() const noexcept {
  const uint32_t width=geometry[1],pitch=width*2;
  return lifetime&&(width==343||width==640)&&geometry[2]==480&&(geometry[7]&255)==16&&geometry[8]&&
   uint64_t(geometry[8])+16+uint64_t(width)*480*2<=0x100000000ULL&&geometry[10]==pitch&&geometry[11]&&geometry[12]&&
   geometry[3]<=geometry[5]&&geometry[4]<=geometry[6]&&geometry[5]<=width&&geometry[6]<=480&&
   geometry[14]==0&&geometry[15]==2&&geometry[16]==2*(width-1)&&geometry[17]==16&&geometry[18]==16+pitch&&geometry[19]==16+479*pitch;
 }
 size_t pixelCount()const noexcept{return valid()?size_t(geometry[1])*480:0;}
 bool operator==(const SurfaceIdentity&o)const noexcept{return lifetime==o.lifetime&&geometry==o.geometry;}
};
struct TextCommand {
 std::array<uint8_t,96> text{};
 uint32_t length=0,role=0,alignment=0,sourceSize=0;
 int32_t x=0,y=0,originX=0,originY=0,advance=0;
 uint16_t color=0;
 Clip clip{0,0,640,480};
};
// Immutable source-command history for display-only finite word substitution.
// This records original observations; it does not supply form/current-owner authority.
struct RecordedCommand {TextCommand command{};size_t vertexFirst=0,vertexCount=0;};
class FramePacket {
 std::shared_ptr<const FontCatalog> font_;SurfaceIdentity surface_;uint64_t serial_=0;
 std::vector<uint16_t> clean_,original_;
 std::vector<Vertex> vertices_;
 std::vector<RecordedCommand>records_;
 size_t commands_=0,writes_=0;
 friend class FramePacketBuilder;
 FramePacket(std::shared_ptr<const FontCatalog> font,SurfaceIdentity surface,uint64_t serial,std::vector<uint16_t>&&clean,std::vector<uint16_t>&&original,std::vector<Vertex>&&vertices,std::vector<RecordedCommand>&&records,size_t commands,size_t writes):font_(std::move(font)),surface_(surface),serial_(serial),clean_(std::move(clean)),original_(std::move(original)),vertices_(std::move(vertices)),records_(std::move(records)),commands_(commands),writes_(writes){}
 public:
 // Legacy constant remains the maximum plane capacity, not every packet size.
 static constexpr size_t maxPixelCount=640*480,pixelCount=maxPixelCount;
 size_t pixels()const noexcept{return original_.size();}
 const std::shared_ptr<const FontCatalog>&font()const noexcept{return font_;}
 const SurfaceIdentity&surface()const noexcept{return surface_;}
 uint64_t serial()const noexcept{return serial_;}
 const std::vector<uint16_t>&clean()const noexcept{return clean_;}
 const std::vector<uint16_t>&original()const noexcept{return original_;}
 const std::vector<Vertex>&vertices()const noexcept{return vertices_;}
 const std::vector<RecordedCommand>&records()const noexcept{return records_;}
 size_t commands()const noexcept{return commands_;}
 size_t writes()const noexcept{return writes_;}
 bool matches(const SurfaceIdentity&current,uint64_t acceptedSerial,const uint16_t*source,size_t count)const noexcept{
  return acceptedSerial==serial_&&surface_==current&&source&&count==pixels()&&std::memcmp(original_.data(),source,pixels()*sizeof(uint16_t))==0;
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
 std::vector<RecordedCommand>records_;
 const uint32_t expectedSourceSize_;
 uint32_t stringExpectedSourceSize_=0;
 size_t used_=0,commands_=0,writes_=0,stringStartWrites_=0;
 TextCommand entry_{};
 static bool sameEntry(const TextCommand&a,const TextCommand&b)noexcept{return a.text==b.text&&a.length==b.length&&a.role==b.role&&a.alignment==b.alignment&&a.sourceSize==b.sourceSize&&a.x==b.x&&a.y==b.y&&a.originX==b.originX&&a.originY==b.originY&&a.color==b.color&&a.clip.left==b.clip.left&&a.clip.top==b.clip.top&&a.clip.right==b.clip.right&&a.clip.bottom==b.clip.bottom;}
 bool fail(Failure why)noexcept{active_=false;stringActive_=false;failure_=why;return false;}
 public:
 explicit FramePacketBuilder(std::shared_ptr<const FontCatalog> font,uint32_t expectedSourceSize=18)noexcept:font_(std::move(font)),expectedSourceSize_(expectedSourceSize){}
 bool active()const noexcept{return active_;}
 Failure failure()const noexcept{return failure_;}
 bool begin(SurfaceIdentity surface,uint64_t serial,const uint16_t*pre,size_t count)noexcept{
  if((expectedSourceSize_!=18&&expectedSourceSize_!=16)||!font_||active_||!surface.valid()||!serial||serial<=lastSerial_||!pre||count!=surface.pixelCount())return fail(Failure::invalidBegin);
  lastSerial_=serial;surface_=surface;serial_=serial;used_=commands_=writes_=0;stringActive_=false;
  try{records_.clear();clean_.assign(pre,pre+count);replay_=clean_;vertices_.resize(maxVertices);}catch(...){return fail(Failure::allocation);}
  active_=true;failure_=Failure::none;return true;
 }
 // entry is copied from the exact original call before its body; advance is determined on exit.
 bool beginString(const TextCommand&entry,uint32_t expectedSourceSize=0)noexcept{
  if(!active_)return false;
  if(stringActive_)return fail(Failure::unclosedString);
  if(commands_>=maxCommands)return fail(Failure::capacity);
  if(expectedSourceSize&&expectedSourceSize!=16&&expectedSourceSize!=18)return fail(Failure::command);
  stringExpectedSourceSize_=expectedSourceSize?expectedSourceSize:expectedSourceSize_;
  entry_=entry;stringStartWrites_=writes_;stringActive_=true;return true;
 }
 bool word(size_t index,uint16_t before,uint16_t computed,uint16_t after)noexcept{
  if(!active_)return false;
  if(!stringActive_)return fail(Failure::unclosedString);
  if(index>=replay_.size()||writes_>=maxWrites)return fail(Failure::capacity);
  if(computed!=after||replay_[index]!=before)return fail(Failure::wordMismatch);
  replay_[index]=after;++writes_;return true;
 }
 bool endString(const TextCommand&c)noexcept{
  if(!active_)return false;
  if(!stringActive_)return fail(Failure::unclosedString);
  if(!sameEntry(entry_,c)||writes_==stringStartWrites_||!c.length||c.length>95||c.role>1||c.alignment>2||c.sourceSize!=stringExpectedSourceSize_||c.advance<=0||c.advance>4096)return fail(Failure::command);
  for(size_t i=0;i<c.length;++i)if(!c.text[i])return fail(Failure::command);
  for(size_t i=c.length;i<c.text.size();++i)if(c.text[i])return fail(Failure::command);
  const int64_t px=int64_t(c.x)+c.originX,py=int64_t(c.y)+c.originY+24-font_->description().baseline[c.role];
  if(px < -8192||px>8192||py < -8192||py>8192)return fail(Failure::command);
  // Logical text coordinates stay unchanged; the actual surface clip is narrower
  // for compact surfaces and must not be widened to the global layout ceiling.
  if(c.clip.left<surface_.geometry[3]||c.clip.top<surface_.geometry[4]||c.clip.right>surface_.geometry[5]||c.clip.bottom>surface_.geometry[6])return fail(Failure::command);
  Layout shaped;
  if(!layout(*font_,c.text.data(),c.length+1,c.role,float(px),float(py),float(c.advance),c.alignment,c.clip,shaped))return fail(Failure::command);
  const size_t first=used_;
  if(!append_vertices(shaped,c.color,vertices_.data(),vertices_.size(),used_))return fail(Failure::capacity);
  try{records_.push_back({c,first,used_-first});}catch(...){return fail(Failure::allocation);}
  ++commands_;stringActive_=false;return true;
 }
 void invalidate(Failure why=Failure::identity)noexcept{fail(why);}
 std::shared_ptr<const FramePacket> finish(SurfaceIdentity current,const uint16_t*post,size_t count)noexcept{
  if(!active_)return {};
  if(stringActive_){fail(Failure::unclosedString);return {};}
  if(!(current==surface_)){fail(Failure::identity);return {};}
  if(!post||count!=surface_.pixelCount()||!commands_||!writes_||!used_){fail(Failure::sourceMismatch);return {};}
  if(std::memcmp(replay_.data(),post,count*sizeof(uint16_t))){fail(Failure::sourceMismatch);return {};}
  try{
   vertices_.resize(used_);
   auto packet=std::shared_ptr<const FramePacket>(new FramePacket(font_,surface_,serial_,std::move(clean_),std::move(replay_),std::move(vertices_),std::move(records_),commands_,writes_));
   active_=false;failure_=Failure::none;return packet;
  }catch(...){fail(Failure::allocation);return {};}
 }
};
}
