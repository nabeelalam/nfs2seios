// SPDX-License-Identifier: MIT
#include "TrackBadgeArtworkLoader.h"
#include "TrackBadgeArtworkPin.h"
#include "TrackBadgeArtProvider.h"
enum {NFS_TRACK_BADGE_SOURCE_COUNT=18};
#include "TextureDigest.h"
#include <zlib.h>
#include <array>
#include <cstring>
#include <fstream>
#include <vector>
#include <limits>
namespace {
constexpr size_t MaxFile=128u*1024u*1024u,MaxPayload=384u*1024u*1024u;
void need(bool ok,const char*why){if(!ok)throw why;}
uint32_t le32(const unsigned char*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
float lefloat(const unsigned char*p){static_assert(sizeof(float)==4,"TrackBadge metadata needs binary32");uint32_t bits=le32(p);float value;std::memcpy(&value,&bits,4);return value;}
unsigned hex(char c){if(c>='0'&&c<='9')return unsigned(c-'0');if(c>='a'&&c<='f')return unsigned(c-'a'+10);if(c>='A'&&c<='F')return unsigned(c-'A'+10);throw "TrackBadge artwork digest pin is invalid";}
std::array<unsigned char,32>pin(){const char*p=NFS_TRACK_BADGE_ARTWORK_SHA256;need(std::strlen(p)==64,"TrackBadge artwork pin is not configured");std::array<unsigned char,32>out{};for(unsigned k=0;k<32;++k)out[k]=static_cast<unsigned char>((hex(p[2*k])<<4)|hex(p[2*k+1]));return out;}
struct Inflater{z_stream z{};bool initialized=false;~Inflater(){if(initialized)inflateEnd(&z);}};
std::vector<unsigned char>inflateExact(const std::vector<unsigned char>&file,size_t bytes){
 need(file.size()>16&&file.size()-16<=std::numeric_limits<uInt>::max()&&bytes&&bytes<=MaxPayload&&bytes<=std::numeric_limits<uInt>::max(),"TrackBadge compressed bounds are invalid");Inflater owner;owner.z.next_in=const_cast<Bytef*>(file.data()+16);owner.z.avail_in=uInt(file.size()-16);need(inflateInit(&owner.z)==Z_OK,"TrackBadge inflater could not start");owner.initialized=true;std::vector<unsigned char>out(bytes);owner.z.next_out=out.data();owner.z.avail_out=uInt(out.size());bool ended=false;
 while(owner.z.avail_out){const auto beforeIn=owner.z.total_in,beforeOut=owner.z.total_out;int r=inflate(&owner.z,Z_NO_FLUSH);need(r==Z_OK||r==Z_STREAM_END,"TrackBadge compressed data is invalid");need(owner.z.total_in!=beforeIn||owner.z.total_out!=beforeOut,"TrackBadge inflater did not advance");if(r==Z_STREAM_END){ended=true;need(owner.z.avail_out==0,"TrackBadge output ended early");break;}}
 if(!ended){unsigned char extra=0;owner.z.next_out=&extra;owner.z.avail_out=1;const auto before=owner.z.total_out;need(inflate(&owner.z,Z_FINISH)==Z_STREAM_END&&owner.z.total_out==before,"TrackBadge output has trailing data");}
 need(owner.z.avail_in==0&&owner.z.total_out==bytes,"TrackBadge compressed input has trailing data");return out;
}
void image(NfsTrackBadgeArtImage&out,const std::vector<unsigned char>&payload,size_t&at){
 need(at<=payload.size()&&48<=payload.size()-at,"TrackBadge image metadata is truncated");const auto*p=payload.data()+at;out.sourceIndex=le32(p);out.format=1;out.width=le32(p+4);out.height=le32(p+8);need(le32(p+12)==0,"TrackBadge image metadata flags are invalid");out.logicalBounds={lefloat(p+16),lefloat(p+20),lefloat(p+24),lefloat(p+28)};out.focalBounds={lefloat(p+32),lefloat(p+36),lefloat(p+40),lefloat(p+44)};at+=48;
 need(out.width&&out.height&&out.width<=8192&&out.height<=8192&&size_t(out.width)<=MaxPayload/4/out.height,"TrackBadge image dimensions are invalid");const size_t count=size_t(out.width)*out.height*4;need(count<=payload.size()-at,"TrackBadge image exceeds payload");out.bytes=count;out.pixels=payload.data()+at;need(nfs_texture_sha256(out.pixels,out.bytes,out.sha)!=0,"TrackBadge image digest failed");at+=count;
}
}
extern "C" int nfs_track_badge_art_load_file(const char*path,char*error,size_t capacity){
 if(error&&capacity)error[0]=0;
 try{if(nfs_track_badge_art_generation())return 1;const auto expected=pin();const size_t fileBytes=NFS_TRACK_BADGE_ARTWORK_FILE_BYTES,payloadBytes=NFS_TRACK_BADGE_ARTWORK_PAYLOAD_BYTES;need(fileBytes>16&&fileBytes<=MaxFile&&payloadBytes>=8&&payloadBytes<=MaxPayload,"TrackBadge artwork size pin is not configured");need(path&&*path,"TrackBadge artwork is not bundled");std::ifstream stream(path,std::ios::binary);need(bool(stream),"TrackBadge artwork is not bundled");stream.seekg(0,std::ios::end);const auto count=stream.tellg();need(count>=0&&uint64_t(count)==fileBytes,"TrackBadge artwork size differs from pin");stream.seekg(0,std::ios::beg);std::vector<unsigned char>file(fileBytes);need(bool(stream.read(reinterpret_cast<char*>(file.data()),std::streamsize(file.size())))&&stream.peek()==std::char_traits<char>::eof(),"TrackBadge artwork read was incomplete");unsigned char sha[32];need(nfs_texture_sha256(file.data(),file.size(),sha)&&!std::memcmp(sha,expected.data(),32),"TrackBadge artwork digest differs from pin");need(!std::memcmp(file.data(),"NFSTRBDG",8)&&le32(file.data()+8)==1&&le32(file.data()+12)==payloadBytes,"TrackBadge artwork header is invalid");const auto payload=inflateExact(file,payloadBytes);need(payload.size()>=8,"TrackBadge metadata is truncated");const uint32_t imageCount=le32(payload.data());need(imageCount&&imageCount<=NFS_TRACK_BADGE_SOURCE_COUNT&&le32(payload.data()+4)==0,"TrackBadge image count is invalid");std::vector<NfsTrackBadgeArtImage>images(imageCount);size_t at=8;for(auto&i:images)image(i,payload,at);need(at==payload.size(),"TrackBadge artwork payload has trailing bytes");NfsTrackBadgeArtInput input{};input.schema=1;input.count=images.size();input.images=images.data();need(nfs_track_badge_art_install(&input)!=0,"TrackBadge immutable artwork was refused");return 1;
 }catch(const char*why){if(error&&capacity){const size_t n=std::strlen(why)<capacity-1?std::strlen(why):capacity-1;std::memcpy(error,why,n);error[n]=0;}return 0;}catch(...){const char*why="TrackBadge artwork could not be loaded";if(error&&capacity){const size_t n=std::strlen(why)<capacity-1?std::strlen(why):capacity-1;std::memcpy(error,why,n);error[n]=0;}return 0;}
}
