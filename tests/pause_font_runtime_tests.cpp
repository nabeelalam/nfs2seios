// SPDX-License-Identifier: MIT
// Authored artwork and captured UV geometry only; no original game pixels.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <set>
#include <vector>
#include "TextureDigest.h"
#include "PauseFontPixels.inc"

static const unsigned GR_TEXFMT_ARGB_1555=11;
struct TextureInfo {unsigned fmt,size;unsigned char *data;int hdDigits;};
#include "PauseFontRuntime.inc"
#define CHECK(v) do {if(!(v)){std::fprintf(stderr,"Pause check failed: %d\n",__LINE__);std::abort();}} while(0)

static void authored_storage(){
 const size_t capacity=4194304,guard=32;
 const unsigned char sentinel=0xa7;
 std::vector<unsigned char> storage(capacity+guard*2,sentinel),before=storage;
 auto *image=storage.data()+guard;
 CHECK(!nfs_pause_expand(nullptr,capacity));
 CHECK(storage==before);
 for(size_t shortCapacity:{size_t(0),capacity-1}){
  CHECK(!nfs_pause_expand(image,shortCapacity));CHECK(storage==before);
 }
 CHECK(nfs_pause_expand(image,capacity));
 std::vector<unsigned char> region;region.reserve(size_t(704)*384*4);
 size_t visible=0,partial=0,opaque=0;
 std::set<unsigned> colors;
 for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x){
  const size_t at=(size_t(y)*1024+x)*4;
  if(x>=320 && y>=640){
   region.insert(region.end(),image+at,image+at+4);
   if(image[at+3]){++visible;colors.insert((unsigned(image[at])<<16)|(unsigned(image[at+1])<<8)|image[at+2]);}
   if(image[at+3]>0 && image[at+3]<255)++partial;
   if(image[at+3]==255)++opaque;
  }else for(unsigned c=0;c<4;++c)CHECK(image[at+c]==sentinel);
 }
 CHECK(region.size()==size_t(704)*384*4);
 CHECK(visible>1000 && partial>100 && opaque>100 && colors.size()>16);
 unsigned char digest[32];CHECK(nfs_texture_sha256(region.data(),region.size(),digest));
 // Independently authored complete region digest.
 static const char expectedHex[]="cf28531359fe430bd88960bc559dd439987333d91d0a02e9c94289f425c9e603";
 CHECK(std::strlen(expectedHex)==64);
 char actualHex[65];for(unsigned i=0;i<32;++i)std::snprintf(actualHex+2*i,3,"%02x",unsigned(digest[i]));
 CHECK(!std::strcmp(actualHex,expectedHex));
 for(size_t i=0;i<guard;++i)CHECK(storage[i]==sentinel && storage[guard+capacity+i]==sentinel);
 const auto decoded=storage;CHECK(nfs_pause_expand(image,capacity));CHECK(storage==decoded);
 // A short-buffer refusal remains inert even after an earlier successful decode.
 CHECK(!nfs_pause_expand(image,capacity-1));CHECK(storage==decoded);
}

static void exact_uv_selector(){
 // Independent captured English pause-cell rectangles, not borrowed from the implementation.
 static const float bounds[6][4]={
  {.498046875f,.498046875f,154.39453125f,23.90625f},
  {.498046875f,25.400390625f,154.39453125f,48.80859375f},
  {.498046875f,73.212890625f,154.39453125f,96.62109375f},
  {.498046875f,97.119140625f,254.00390625f,120.52734375f},
  {.498046875f,49.306640625f,154.39453125f,72.71484375f},
  {.498046875f,198.720703125f,254.00390625f,226.11328125f}
 };
 unsigned complete=0,rejected=0;
 auto decline=[&](const std::array<float,6>&uv){
  const auto before=uv;CHECK(!nfs_pause_triangle(uv.data()));CHECK(!std::memcmp(before.data(),uv.data(),sizeof(uv)));++rejected;
 };
 for(unsigned label=0;label<6;++label)for(unsigned omitted=0;omitted<4;++omitted){
  std::array<float,6> uv{};unsigned n=0;
  for(unsigned corner=0;corner<4;++corner)if(corner!=omitted){
   uv[2*n]=bounds[label][corner&1?2:0];uv[2*n+1]=bounds[label][corner&2?3:1];++n;
  }
  const auto before=uv;CHECK(nfs_pause_triangle(uv.data())==int(50+label));
  CHECK(!std::memcmp(before.data(),uv.data(),sizeof(uv)));++complete;
  auto bad=uv;bad[0]+=.02f;decline(bad); // Clipped/non-corner coordinate.
  bad=uv;bad[1]=(bounds[label][1]+bounds[label][3])*.5f;decline(bad);
  bad=uv;bad[1]=bounds[(label+1)%6][1];decline(bad); // Cross-cell triangle.
  bad=uv;for(unsigned i=1;i<3;++i){bad[2*i]=bad[0];bad[2*i+1]=bad[1];}decline(bad);
  bad=uv;bad[2]=bad[0];bad[3]=bad[1];decline(bad); // Duplicate corner.
  for(float invalid:{std::numeric_limits<float>::quiet_NaN(),
                     std::numeric_limits<float>::infinity(),
                     -std::numeric_limits<float>::infinity()}){
   for(unsigned component=0;component<6;++component){bad=uv;bad[component]=invalid;decline(bad);}
  }
 }
 CHECK(complete==24 && rejected==552);
}

static void exact_header_selector(){
 // Independently reconstruct the original half-pixel lower / integer upper formula.
 const unsigned source[2][4]={{0,229,130,29},{209,227,38,29}};
 unsigned accepted=0;
 for(unsigned label=0;label<2;++label){
  const auto &r=source[label];
  const float b[4]={(r[0]+.5f)*255/256,(r[1]+.5f)*255/256,float(r[0]+r[2])*255/256,float(r[1]+r[3])*255/256};
  for(unsigned omitted=0;omitted<4;++omitted){
   std::array<unsigned,3> order{};unsigned n=0;for(unsigned k=0;k<4;++k)if(k!=omitted)order[n++]=k;
   do {
    std::array<float,6> uv{};for(unsigned i=0;i<3;++i){uv[i*2]=b[(order[i]&1)?2:0];uv[i*2+1]=b[(order[i]&2)?3:1];}
    const auto before=uv;CHECK(nfs_pause_triangle(uv.data())==int(73+label));CHECK(uv==before);++accepted;
    auto bad=uv;bad[0]+=.02f;CHECK(!nfs_pause_triangle(bad.data()));
    bad=uv;bad[2]=bad[0];bad[3]=bad[1];CHECK(!nfs_pause_triangle(bad.data()));
    bad=uv;bad[1]=(b[1]+b[3])*.5f;CHECK(!nfs_pause_triangle(bad.data()));
    for(float invalid:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()})for(unsigned i=0;i<6;++i){bad=uv;bad[i]=invalid;CHECK(!nfs_pause_triangle(bad.data()));}
   }while(std::next_permutation(order.begin(),order.end()));
  }
 }
 CHECK(accepted==48);
 // Cropping the over-range title to atlas row256 is not the captured complete triangle.
 const float cropped[6]={.498046875f,228.603515625f,129.4921875f,228.603515625f,129.4921875f,255.f};CHECK(!nfs_pause_triangle(cropped));
}

static void unauthenticated_source_rejection(){
 // An authored test buffer cannot establish the original game's source identity.
 // Never replace/mock the production original digest to manufacture a positive match.
 std::vector<unsigned char> bytes(131072,0),originalBefore(nfs_pause_original,nfs_pause_original+sizeof nfs_pause_original);
 const auto inputBefore=bytes;
 const int validBefore=nfs_pause_valid;
 auto refuse=[&](TextureInfo value){
  const TextureInfo before=value;
  CHECK(!nfs_pause_match(&value));
  CHECK(value.fmt==before.fmt && value.size==before.size && value.data==before.data && value.hdDigits==before.hdDigits);
  CHECK(bytes==inputBefore && nfs_pause_valid==validBefore);
  CHECK(!std::memcmp(nfs_pause_original,originalBefore.data(),originalBefore.size()));
 };
 refuse({GR_TEXFMT_ARGB_1555,256,bytes.data(),-19});
 refuse({GR_TEXFMT_ARGB_1555+1,256,nullptr,-19});
 for(unsigned size:{0u,128u,255u,257u,512u})refuse({GR_TEXFMT_ARGB_1555,size,nullptr,-19});
 bytes[65535]=1;const auto altered=bytes;
 TextureInfo wrong{GR_TEXFMT_ARGB_1555,256,bytes.data(),-19};
 CHECK(!nfs_pause_match(&wrong) && wrong.hdDigits==-19 && bytes==altered && nfs_pause_valid==validBefore);
 CHECK(!std::memcmp(nfs_pause_original,originalBefore.data(),originalBefore.size()));
}

int main(){
 authored_storage();exact_uv_selector();exact_header_selector();unauthenticated_source_rejection();
 std::puts("PASS authored pause storage/digest/canaries, 24 captured-cell triangles/552 malformed negatives and unauthenticated-source fallback; original-source positive match requires private game evidence");
}
