// SPDX-License-Identifier: MIT
// Pure, bounded original indexed-chart replay. No guest pointers or current-owner authority.
#pragma once
#include "SoftwareFramePacket.h"
#include "SteeringBlend565.h"
#include <array>
#include <cstring>
#include <memory>
#include <vector>
namespace nfs_steering_chart {
struct Binding {
 uint64_t lifetime=0,ownerEpoch=0,textSerial=0;
 uint32_t form=0,handler=0,archive=0,sensitivity=0,targetBitmap=0;
 bool valid()const noexcept{return lifetime&&ownerEpoch&&textSerial&&form&&handler&&archive&&sensitivity<4&&targetBitmap&&uint64_t(targetBitmap)+614416<=UINT64_C(0x100000000);}
 bool operator==(const Binding&o)const noexcept{return lifetime==o.lifetime&&ownerEpoch==o.ownerEpoch&&textSerial==o.textSerial&&form==o.form&&handler==o.handler&&archive==o.archive&&sensitivity==o.sensitivity&&targetBitmap==o.targetBitmap;}
};
class Builder;
class Packet {
 Binding binding_;uint32_t sourceAddress_=0;std::shared_ptr<const nfs_soft::FramePacket>text_;
 std::vector<uint8_t>source_;std::vector<uint16_t>original_;unsigned rows_=0,stores_=0;
 friend class Builder;
 Packet(Binding b,uint32_t sourceAddress,std::shared_ptr<const nfs_soft::FramePacket>text,std::vector<uint8_t>&&source,std::vector<uint16_t>&&original,unsigned rows,unsigned stores):binding_(b),sourceAddress_(sourceAddress),text_(std::move(text)),source_(std::move(source)),original_(std::move(original)),rows_(rows),stores_(stores){}
 public:
 const Binding&binding()const noexcept{return binding_;}
 uint32_t sourceAddress()const noexcept{return sourceAddress_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&text()const noexcept{return text_;}
 const std::vector<uint8_t>&source()const noexcept{return source_;}
 const std::vector<uint16_t>&original()const noexcept{return original_;}
 unsigned rows()const noexcept{return rows_;}unsigned stores()const noexcept{return stores_;}
 bool matches(const Binding&b,const uint16_t*plane,size_t count)const noexcept{return binding_==b&&plane&&count==original_.size()&&std::memcmp(plane,original_.data(),count*sizeof(uint16_t))==0;}
};
class Builder {
 public:
 enum class Failure {none,notActive,invalidBegin,nesting,source,identity,row,palette,word,missingStore,finalPlane,allocation};
 static constexpr size_t sourceSize=18500,pixelCount=640*480;
 static constexpr unsigned width=356,height=49,maxStores=width*height;
 using Palette=std::array<uint32_t,256>;using Row=std::array<uint16_t,width>;
 private:
 Binding binding_{};std::shared_ptr<const nfs_soft::FramePacket>text_;
 std::vector<uint8_t>source_;std::vector<uint16_t>replay_;
 Palette palette_{};Row expected_{};uint32_t sourceAddress_=0,destination_=0;
 unsigned rows_=0,stores_=0,next_=0;bool active_=false,rowActive_=false;Failure failure_=Failure::notActive;
 bool fail(Failure f)noexcept{active_=rowActive_=false;failure_=f;return false;}
 static uint32_t rawWord(const std::vector<uint8_t>&s,size_t at)noexcept{return uint32_t(s[at])|(uint32_t(s[at+1])<<8)|(uint32_t(s[at+2])<<16)|(uint32_t(s[at+3])<<24);}
 static bool sourceValid(const std::vector<uint8_t>&s)noexcept{
  static const uint8_t header[]={0x7b,0x34,0x44,0,0x64,1,0x31,0,0xec,0xfe,0xbc,0xff,0x14,1,0x44,0};
  static const uint8_t attachment[]={0x2a,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0};
  return s.size()==sourceSize&&!std::memcmp(s.data(),header,16)&&!std::memcmp(s.data()+17460,attachment,16);
 }
 static uint32_t converted(uint32_t raw)noexcept{return (raw&UINT32_C(0xff000000))|((raw&UINT32_C(0xf80000))>>8)|((raw&UINT32_C(0xfc00))>>5)|((raw>>3)&31);}
 size_t rowOffset()const noexcept{return size_t(280+rows_)*640+280;}
 void skip()noexcept{while(next_<width&&palette_[source_[16+rows_*width+next_]]<UINT32_C(0x10000000))++next_;}
 public:
 bool active()const noexcept{return active_;}bool rowActive()const noexcept{return rowActive_;}
 Failure failure()const noexcept{return failure_;}unsigned rows()const noexcept{return rows_;}unsigned stores()const noexcept{return stores_;}
 void reset()noexcept{active_=rowActive_=false;binding_=Binding{};text_.reset();source_.clear();replay_.clear();sourceAddress_=destination_=0;rows_=stores_=next_=0;failure_=Failure::notActive;}
 void reject(Failure f=Failure::identity)noexcept{fail(f);}
 bool begin(const Binding&b,std::shared_ptr<const nfs_soft::FramePacket>text,uint32_t sourceAddress,const std::vector<uint8_t>&source)noexcept{
  if(active_)return fail(Failure::nesting);
  if(!b.valid()||!text||!sourceAddress||uint64_t(sourceAddress)+sourceSize>UINT64_C(0x100000000)||!sourceValid(source))return fail(Failure::invalidBegin);
  const auto&s=text->surface();
  if(!s.valid()||s.geometry[1]!=640||s.geometry[2]!=480||s.geometry[8]!=b.targetBitmap||s.lifetime!=b.lifetime||text->serial()!=b.textSerial||text->original().size()!=pixelCount)return fail(Failure::identity);
  try{source_=source;replay_=text->original();}catch(...){return fail(Failure::allocation);}
  binding_=b;text_=std::move(text);sourceAddress_=sourceAddress;rows_=stores_=next_=0;rowActive_=false;
  for(unsigned i=0;i<256;++i)palette_[i]=converted(rawWord(source_,17476+4*i));
  active_=true;failure_=Failure::none;return true;
 }
 bool beginRow(uint32_t sourceAddress,uint32_t destination,unsigned count,const Palette&palette,const Row&before)noexcept{
  if(!active_)return false;
  if(rowActive_)return fail(Failure::nesting);
  if(rows_>=height||count!=width||sourceAddress!=sourceAddress_+16+rows_*width||uint64_t(destination)!=uint64_t(binding_.targetBitmap)+16+uint64_t(280+rows_)*1280+560)return fail(Failure::row);
  if(palette!=palette_)return fail(Failure::palette);
  const size_t at=rowOffset();for(unsigned i=0;i<width;++i){if(before[i]!=replay_[at+i])return fail(Failure::row);expected_[i]=blendIndexedPalette565(palette_[source_[16+rows_*width+i]],before[i]).value;}
  destination_=destination;next_=0;rowActive_=true;skip();return true;
 }
 bool word(uint32_t address,uint16_t before,uint16_t computed,uint16_t after)noexcept{
  if(!active_)return false;
  if(!rowActive_||next_>=width||stores_>=maxStores||uint64_t(address)!=uint64_t(destination_)+2*next_)return fail(Failure::word);
  const size_t at=rowOffset()+next_;if(before!=replay_[at]||computed!=expected_[next_]||after!=computed)return fail(Failure::word);
  replay_[at]=after;++stores_;++next_;skip();return true;
 }
 bool endRow(const Row&after,const Palette&palette)noexcept{
  if(!active_)return false;
  if(!rowActive_)return fail(Failure::row);
  if(palette!=palette_)return fail(Failure::palette);
  if(next_!=width)return fail(Failure::missingStore);
  const size_t at=rowOffset();for(unsigned i=0;i<width;++i)if(after[i]!=expected_[i]||replay_[at+i]!=after[i])return fail(Failure::row);
  ++rows_;rowActive_=false;return true;
 }
 std::shared_ptr<const Packet>finish(const Binding&b,const std::vector<uint8_t>&source,const uint16_t*final,size_t count)noexcept{
  if(!active_)return {};
  if(rowActive_||rows_!=height){fail(Failure::row);return {};}
  if(!(binding_==b)){fail(Failure::identity);return {};}
  if(source!=source_){fail(Failure::source);return {};}
  if(!final||count!=pixelCount||std::memcmp(final,replay_.data(),pixelCount*sizeof(uint16_t))){fail(Failure::finalPlane);return {};}
  try{auto p=std::shared_ptr<const Packet>(new Packet(binding_,sourceAddress_,text_,std::move(source_),std::move(replay_),rows_,stores_));active_=false;failure_=Failure::none;return p;}catch(...){fail(Failure::allocation);return {};}
 }
};
} // namespace nfs_steering_chart
