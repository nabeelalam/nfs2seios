// SPDX-License-Identifier: MIT
#include "ClassPictureArtworkLoader.h"
#include "ClassPictureArtworkPin.h"
#include "ClassPictureArtProvider.h"
enum {NFS_CLASS_PICTURE_SOURCE_COUNT=3};
#include "TextureDigest.h"
#include <zlib.h>
#include <array>
#include <cstring>
#include <fstream>
#include <vector>
#include <limits>
namespace {
constexpr size_t MaxFile=9u*1024u*1024u,MaxPayload=9u*1024u*1024u;
void need(bool ok,const char*why){if(!ok)throw why;}
uint32_t le32(const unsigned char*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
float lefloat(const unsigned char*p){static_assert(sizeof(float)==4,"ClassPicture metadata needs binary32");uint32_t bits=le32(p);float value;std::memcpy(&value,&bits,4);return value;}
unsigned hex(char c){if(c>='0'&&c<='9')return unsigned(c-'0');if(c>='a'&&c<='f')return unsigned(c-'a'+10);if(c>='A'&&c<='F')return unsigned(c-'A'+10);throw "ClassPicture artwork digest pin is invalid";}
std::array<unsigned char,32>pin(){const char*p=NFS_CLASS_PICTURE_ARTWORK_SHA256;need(std::strlen(p)==64,"ClassPicture artwork pin is not configured");std::array<unsigned char,32>out{};for(unsigned k=0;k<32;++k)out[k]=static_cast<unsigned char>((hex(p[2*k])<<4)|hex(p[2*k+1]));return out;}
struct Inflater{z_stream z{};bool initialized=false;~Inflater(){if(initialized)inflateEnd(&z);}};
std::vector<unsigned char>inflateExact(const std::vector<unsigned char>&file,size_t bytes){
 need(file.size()>16&&file.size()-16<=std::numeric_limits<uInt>::max()&&bytes&&bytes<=MaxPayload&&bytes<=std::numeric_limits<uInt>::max(),"ClassPicture compressed bounds are invalid");Inflater owner;owner.z.next_in=const_cast<Bytef*>(file.data()+16);owner.z.avail_in=uInt(file.size()-16);need(inflateInit(&owner.z)==Z_OK,"ClassPicture inflater could not start");owner.initialized=true;std::vector<unsigned char>out(bytes);owner.z.next_out=out.data();owner.z.avail_out=uInt(out.size());bool ended=false;
 while(owner.z.avail_out){const auto beforeIn=owner.z.total_in,beforeOut=owner.z.total_out;int r=inflate(&owner.z,Z_NO_FLUSH);need(r==Z_OK||r==Z_STREAM_END,"ClassPicture compressed data is invalid");need(owner.z.total_in!=beforeIn||owner.z.total_out!=beforeOut,"ClassPicture inflater did not advance");if(r==Z_STREAM_END){ended=true;need(owner.z.avail_out==0,"ClassPicture output ended early");break;}}
 if(!ended){unsigned char extra=0;owner.z.next_out=&extra;owner.z.avail_out=1;const auto before=owner.z.total_out;need(inflate(&owner.z,Z_FINISH)==Z_STREAM_END&&owner.z.total_out==before,"ClassPicture output has trailing data");}
 need(owner.z.avail_in==0&&owner.z.total_out==bytes,"ClassPicture compressed input has trailing data");return out;
}
void image(NfsClassPictureArtImage&out,const std::vector<unsigned char>&payload,size_t&at){
 need(at<=payload.size()&&48<=payload.size()-at,"ClassPicture image metadata is truncated");const auto*p=payload.data()+at;out.sourceIndex=le32(p);out.format=1;out.width=le32(p+4);out.height=le32(p+8);need(le32(p+12)==0,"ClassPicture image metadata flags are invalid");out.logicalBounds={lefloat(p+16),lefloat(p+20),lefloat(p+24),lefloat(p+28)};out.focalBounds={lefloat(p+32),lefloat(p+36),lefloat(p+40),lefloat(p+44)};at+=48;
 need(out.width&&out.height&&out.width<=8192&&out.height<=8192&&size_t(out.width)<=MaxPayload/4/out.height,"ClassPicture image dimensions are invalid");const size_t count=size_t(out.width)*out.height*4;need(count<=payload.size()-at,"ClassPicture image exceeds payload");out.bytes=count;out.pixels=payload.data()+at;need(nfs_texture_sha256(out.pixels,out.bytes,out.sha)!=0,"ClassPicture image digest failed");at+=count;
}
}
extern "C" int nfs_class_picture_art_load_file(const char*path,char*error,size_t capacity){
 if(error&&capacity)error[0]=0;
 try{if(nfs_class_picture_art_generation())return 1;const auto expected=pin();const size_t fileBytes=NFS_CLASS_PICTURE_ARTWORK_FILE_BYTES,payloadBytes=NFS_CLASS_PICTURE_ARTWORK_PAYLOAD_BYTES;need(fileBytes>16&&fileBytes<=MaxFile&&payloadBytes>=8&&payloadBytes<=MaxPayload,"ClassPicture artwork size pin is not configured");need(path&&*path,"ClassPicture artwork is not bundled");std::ifstream stream(path,std::ios::binary);need(bool(stream),"ClassPicture artwork is not bundled");stream.seekg(0,std::ios::end);const auto count=stream.tellg();need(count>=0&&uint64_t(count)==fileBytes,"ClassPicture artwork size differs from pin");stream.seekg(0,std::ios::beg);std::vector<unsigned char>file(fileBytes);need(bool(stream.read(reinterpret_cast<char*>(file.data()),std::streamsize(file.size())))&&stream.peek()==std::char_traits<char>::eof(),"ClassPicture artwork read was incomplete");unsigned char sha[32];need(nfs_texture_sha256(file.data(),file.size(),sha)&&!std::memcmp(sha,expected.data(),32),"ClassPicture artwork digest differs from pin");need(!std::memcmp(file.data(),"NFSCLPIC",8)&&le32(file.data()+8)==1&&le32(file.data()+12)==payloadBytes,"ClassPicture artwork header is invalid");const auto payload=inflateExact(file,payloadBytes);need(payload.size()>=8,"ClassPicture metadata is truncated");const uint32_t imageCount=le32(payload.data());need(imageCount&&imageCount<=NFS_CLASS_PICTURE_SOURCE_COUNT&&le32(payload.data()+4)==0,"ClassPicture image count is invalid");std::vector<NfsClassPictureArtImage>images(imageCount);size_t at=8;for(auto&i:images)image(i,payload,at);need(at==payload.size(),"ClassPicture artwork payload has trailing bytes");NfsClassPictureArtInput input{};input.schema=1;input.count=images.size();input.images=images.data();need(nfs_class_picture_art_install(&input)!=0,"ClassPicture immutable artwork was refused");return 1;
 }catch(const char*why){if(error&&capacity){const size_t n=std::strlen(why)<capacity-1?std::strlen(why):capacity-1;std::memcpy(error,why,n);error[n]=0;}return 0;}catch(...){const char*why="ClassPicture artwork could not be loaded";if(error&&capacity){const size_t n=std::strlen(why)<capacity-1?std::strlen(why):capacity-1;std::memcpy(error,why,n);error[n]=0;}return 0;}
}
