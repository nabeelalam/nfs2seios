// SPDX-License-Identifier: MIT
#include "CreditsArtworkLoader.h"
#include "CreditsArtworkPin.h"
#include "CreditsArtProvider.h"
#include "TextureDigest.h"
#include <zlib.h>
#include <array>
#include <cstring>
#include <fstream>
#include <vector>
#include <limits>
namespace {
constexpr size_t MaxFile=64u*1024u*1024u,MaxPayload=96u*1024u*1024u;
void need(bool ok,const char*why){if(!ok)throw why;}
uint32_t le32(const unsigned char*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
unsigned hex(char c){if(c>='0'&&c<='9')return unsigned(c-'0');if(c>='a'&&c<='f')return unsigned(c-'a'+10);if(c>='A'&&c<='F')return unsigned(c-'A'+10);throw "Credits artwork digest pin is invalid";}
std::array<unsigned char,32>pin(){const char*p=NFS_CREDITS_ARTWORK_SHA256;need(std::strlen(p)==64,"Credits artwork pin is not configured");std::array<unsigned char,32>out{};for(unsigned k=0;k<32;++k)out[k]=static_cast<unsigned char>((hex(p[2*k])<<4)|hex(p[2*k+1]));return out;}
struct Inflater{z_stream z{};bool initialized=false;~Inflater(){if(initialized)inflateEnd(&z);}};
std::vector<unsigned char>inflateExact(const std::vector<unsigned char>&file,size_t bytes){
 need(file.size()>16&&file.size()-16<=std::numeric_limits<uInt>::max()&&bytes&&bytes<=MaxPayload&&bytes<=std::numeric_limits<uInt>::max(),"Credits compressed bounds are invalid");Inflater owner;owner.z.next_in=const_cast<Bytef*>(file.data()+16);owner.z.avail_in=uInt(file.size()-16);need(inflateInit(&owner.z)==Z_OK,"Credits inflater could not start");owner.initialized=true;std::vector<unsigned char>out(bytes);owner.z.next_out=out.data();owner.z.avail_out=uInt(out.size());bool ended=false;
 while(owner.z.avail_out){const auto beforeIn=owner.z.total_in,beforeOut=owner.z.total_out;int r=inflate(&owner.z,Z_NO_FLUSH);need(r==Z_OK||r==Z_STREAM_END,"Credits compressed data is invalid");need(owner.z.total_in!=beforeIn||owner.z.total_out!=beforeOut,"Credits inflater did not advance");if(r==Z_STREAM_END){ended=true;need(owner.z.avail_out==0,"Credits output ended early");break;}}
 if(!ended){unsigned char extra=0;owner.z.next_out=&extra;owner.z.avail_out=1;const auto before=owner.z.total_out;need(inflate(&owner.z,Z_FINISH)==Z_STREAM_END&&owner.z.total_out==before,"Credits output has trailing data");}
 need(owner.z.avail_in==0&&owner.z.total_out==bytes,"Credits compressed input has trailing data");return out;
}
void image(NfsCreditsArtImage&out,const std::vector<unsigned char>&payload,size_t&at){
 need(at<=payload.size()&&payload.size()-at>=44,"Credits layer metadata is incomplete");
 out.width=le32(payload.data()+at);out.height=le32(payload.data()+at+4);out.bytes=le32(payload.data()+at+8);std::memcpy(out.sha256,payload.data()+at+12,32);at+=44;
 need(out.width&&out.height&&out.width<=8192&&out.height<=8192&&size_t(out.width)<=MaxPayload/4/out.height,"Credits layer dimensions are invalid");
 need(out.bytes==size_t(out.width)*out.height*4&&out.bytes<=payload.size()-at,"Credits layer exceeds payload");
 out.pixels=payload.data()+at;at+=out.bytes;
}
}
extern "C" int nfs_credits_art_load_file(const char*path,char*error,size_t capacity){
 if(error&&capacity)error[0]=0;
 try{
  if(nfs_credits_art_generation())return 1;
  const auto expected=pin();const size_t fileBytes=NFS_CREDITS_ARTWORK_FILE_BYTES,payloadBytes=NFS_CREDITS_ARTWORK_PAYLOAD_BYTES;
  need(fileBytes>16&&fileBytes<=MaxFile&&payloadBytes>=44*NFS_CREDITS_ART_COUNT&&payloadBytes<=MaxPayload,"Credits artwork size pin is invalid");
  need(path&&*path,"Credits artwork is not bundled");std::ifstream stream(path,std::ios::binary);need(bool(stream),"Credits artwork is not bundled");
  stream.seekg(0,std::ios::end);const auto count=stream.tellg();need(count>=0&&uint64_t(count)==fileBytes,"Credits artwork size differs from pin");stream.seekg(0,std::ios::beg);
  std::vector<unsigned char>file(fileBytes);need(bool(stream.read(reinterpret_cast<char*>(file.data()),std::streamsize(file.size())))&&stream.peek()==std::char_traits<char>::eof(),"Credits artwork read was incomplete");
  unsigned char sha[32];need(nfs_texture_sha256(file.data(),file.size(),sha)&&!std::memcmp(sha,expected.data(),32),"Credits artwork digest differs from pin");
  need(!std::memcmp(file.data(),"NFSCRD01",8)&&le32(file.data()+8)==1&&le32(file.data()+12)==payloadBytes,"Credits artwork header is invalid");
  const auto payload=inflateExact(file,payloadBytes);size_t at=0;NfsCreditsArtInput input{};input.schema=1;
  for(auto&layer:input.images)image(layer,payload,at);
  need(at==payload.size(),"Credits artwork has trailing layers");need(nfs_credits_art_install(&input)!=0,"Credits artwork layers were refused");return 1;
 }catch(const char*why){if(error&&capacity){const size_t n=std::strlen(why)<capacity-1?std::strlen(why):capacity-1;std::memcpy(error,why,n);error[n]=0;}return 0;}
 catch(...){const char*why="Credits artwork could not be loaded";if(error&&capacity){const size_t n=std::strlen(why)<capacity-1?std::strlen(why):capacity-1;std::memcpy(error,why,n);error[n]=0;}return 0;}
}
