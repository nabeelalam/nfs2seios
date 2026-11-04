// SPDX-License-Identifier: MIT
#include "IntroArtworkLoader.h"
#include "IntroArtworkPin.h"
#include "IntroArtProvider.h"
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
float lefloat(const unsigned char*p){static_assert(sizeof(float)==4,"Intro metadata needs binary32");uint32_t bits=le32(p);float value;std::memcpy(&value,&bits,4);return value;}
unsigned hex(char c){if(c>='0'&&c<='9')return unsigned(c-'0');if(c>='a'&&c<='f')return unsigned(c-'a'+10);if(c>='A'&&c<='F')return unsigned(c-'A'+10);throw "Intro artwork digest pin is invalid";}
std::array<unsigned char,32>pin(){const char*p=NFS_INTRO_ARTWORK_SHA256;need(std::strlen(p)==64,"Intro artwork pin is not configured");std::array<unsigned char,32>out{};for(unsigned k=0;k<32;++k)out[k]=static_cast<unsigned char>((hex(p[2*k])<<4)|hex(p[2*k+1]));return out;}
struct Inflater{z_stream z{};bool initialized=false;~Inflater(){if(initialized)inflateEnd(&z);}};
std::vector<unsigned char>inflateExact(const std::vector<unsigned char>&file,size_t bytes){
 need(file.size()>16&&file.size()-16<=std::numeric_limits<uInt>::max()&&bytes&&bytes<=MaxPayload&&bytes<=std::numeric_limits<uInt>::max(),"Intro compressed bounds are invalid");Inflater owner;owner.z.next_in=const_cast<Bytef*>(file.data()+16);owner.z.avail_in=uInt(file.size()-16);need(inflateInit(&owner.z)==Z_OK,"Intro inflater could not start");owner.initialized=true;std::vector<unsigned char>out(bytes);owner.z.next_out=out.data();owner.z.avail_out=uInt(out.size());bool ended=false;
 while(owner.z.avail_out){const auto beforeIn=owner.z.total_in,beforeOut=owner.z.total_out;int r=inflate(&owner.z,Z_NO_FLUSH);need(r==Z_OK||r==Z_STREAM_END,"Intro compressed data is invalid");need(owner.z.total_in!=beforeIn||owner.z.total_out!=beforeOut,"Intro inflater did not advance");if(r==Z_STREAM_END){ended=true;need(owner.z.avail_out==0,"Intro output ended early");break;}}
 if(!ended){unsigned char extra=0;owner.z.next_out=&extra;owner.z.avail_out=1;const auto before=owner.z.total_out;need(inflate(&owner.z,Z_FINISH)==Z_STREAM_END&&owner.z.total_out==before,"Intro output has trailing data");}
 need(owner.z.avail_in==0&&owner.z.total_out==bytes,"Intro compressed input has trailing data");return out;
}
void image(NfsIntroImage&out,uint32_t format,uint32_t width,uint32_t height,const std::vector<unsigned char>&payload,size_t&at){
 const size_t components=format==NFS_INTRO_RGBA_PREMULTIPLIED?4u:1u;need(width&&height&&width<=8192&&height<=8192&&size_t(width)<=MaxPayload/components/height,"Intro image dimensions are invalid");const size_t count=size_t(width)*height*components;need(at<=payload.size()&&count<=payload.size()-at,"Intro image exceeds payload");out.format=format;out.width=width;out.height=height;out.bytes=count;out.pixels=payload.data()+at;need(nfs_texture_sha256(out.pixels,out.bytes,out.sha)!=0,"Intro image digest failed");at+=count;
}
}
extern "C" int nfs_intro_art_load_file(const char*path,char*error,size_t capacity){
 if(error&&capacity)error[0]=0;
 try{if(nfs_intro_art_generation())return 1;const auto expected=pin();const size_t fileBytes=NFS_INTRO_ARTWORK_FILE_BYTES,payloadBytes=NFS_INTRO_ARTWORK_PAYLOAD_BYTES;need(fileBytes>16&&fileBytes<=MaxFile&&payloadBytes>=72&&payloadBytes<=MaxPayload,"Intro artwork size pin is not configured");need(path&&*path,"Intro artwork is not bundled");std::ifstream stream(path,std::ios::binary);need(bool(stream),"Intro artwork is not bundled");stream.seekg(0,std::ios::end);const auto count=stream.tellg();need(count>=0&&uint64_t(count)==fileBytes,"Intro artwork size differs from pin");stream.seekg(0,std::ios::beg);std::vector<unsigned char>file(fileBytes);need(bool(stream.read(reinterpret_cast<char*>(file.data()),std::streamsize(file.size())))&&stream.peek()==std::char_traits<char>::eof(),"Intro artwork read was incomplete");unsigned char sha[32];need(nfs_texture_sha256(file.data(),file.size(),sha)&&!std::memcmp(sha,expected.data(),32),"Intro artwork digest differs from pin");need(!std::memcmp(file.data(),"NFSINTRO",8)&&le32(file.data()+8)==1&&le32(file.data()+12)==payloadBytes,"Intro artwork header is invalid");const auto payload=inflateExact(file,payloadBytes);NfsIntroArtInput input{};input.schema=1;input.logoRect={lefloat(payload.data()+8),lefloat(payload.data()+12),lefloat(payload.data()+16),lefloat(payload.data()+20)};size_t at=72;image(input.photograph,NFS_INTRO_RGBA_PREMULTIPLIED,le32(payload.data()),le32(payload.data()+4),payload,at);for(unsigned k=0;k<6;++k)image(input.copyright[k],NFS_INTRO_ALPHA8,le32(payload.data()+24+8*k),le32(payload.data()+28+8*k),payload,at);need(at==payload.size(),"Intro artwork payload has trailing bytes");need(nfs_intro_art_install(&input)!=0,"Intro artwork inputs or silver-logo provider were refused");return 1;
 }catch(const char*why){if(error&&capacity){const size_t n=std::strlen(why)<capacity-1?std::strlen(why):capacity-1;std::memcpy(error,why,n);error[n]=0;}return 0;}catch(...){const char*why="Intro artwork could not be loaded";if(error&&capacity){const size_t n=std::strlen(why)<capacity-1?std::strlen(why):capacity-1;std::memcpy(error,why,n);error[n]=0;}return 0;}
}
