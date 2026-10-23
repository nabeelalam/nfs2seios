// SPDX-License-Identifier: MIT
#include "hd/CompactCaptionWordArtwork.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <type_traits>
#include <utility>
namespace art=nfs_compact_caption_display;
#define CHECK(value) do{if(!(value)){std::fprintf(stderr,"Compact caption artwork check failed at line %d\n",__LINE__);std::abort();}}while(0)
static_assert(std::is_same<decltype(art::artwork(1)),std::shared_ptr<const art::Artwork>>::value,"published image owners are immutable");
static_assert(std::is_same<decltype(*art::artwork(1)),const art::Artwork&>::value,"shared aliases cannot mutate artwork");
static_assert(std::is_same<decltype(std::declval<const art::Artwork&>().rgba.data()),const unsigned char*>::value,"published pixel spans are const");
struct Runs {const unsigned short*data;size_t count;};
static const Runs sources[3]={
 {nfs_compact_caption_word_alpha_rle_1,sizeof(nfs_compact_caption_word_alpha_rle_1)/sizeof(unsigned short)},
 {nfs_compact_caption_word_alpha_rle_2,sizeof(nfs_compact_caption_word_alpha_rle_2)/sizeof(unsigned short)},
 {nfs_compact_caption_word_alpha_rle_3,sizeof(nfs_compact_caption_word_alpha_rle_3)/sizeof(unsigned short)}};
static void imageChecks(unsigned word){
 auto image=art::artwork(word);CHECK(image);
 const unsigned width=word==1?300:word==2?208:192;
 CHECK(image->word==word&&image->width==width&&image->height==128&&image->rgba.size()==size_t(width)*128*4);
 unsigned char digest[32];CHECK(nfs_texture_sha256(image->rgba.data(),image->rgba.size(),digest));
 CHECK(!std::memcmp(digest,nfs_compact_caption_word_sha256[word-1],32));
 CHECK(!std::memcmp(digest,image->sha.data(),32));
 bool ink=false;
 for(unsigned y=0;y<128;++y)for(unsigned x=0;x<width;++x){
  const size_t at=(size_t(y)*width+x)*4;
  CHECK(image->rgba[at]==255&&image->rgba[at+1]==255&&image->rgba[at+2]==255);
  if(!x||x==width-1||!y||y==127)CHECK(image->rgba[at+3]==0);
  ink|=image->rgba[at+3]!=0;
 }
 CHECK(ink);
 const auto bytes=image->rgba;const auto sha=image->sha;
 auto alias=art::artwork(word);CHECK(alias==image&&alias->rgba.data()==image->rgba.data());
 const Runs source=sources[word-1];
 std::vector<unsigned short>owned(source.data,source.data+source.count);
 auto decoded=art::decode(word,owned.data(),owned.size());CHECK(decoded&&decoded!=image&&decoded->rgba==bytes);
 const auto decodedBytes=decoded->rgba;
 std::fill(owned.begin(),owned.end(),0);
 CHECK(decoded->rgba==decodedBytes&&image->rgba==bytes&&image->sha==sha);
 // Every declined decode leaves retained owners and aliases unchanged.
 CHECK(!art::decode(word,nullptr,source.count));
 CHECK(!art::decode(word,source.data,0));
 CHECK(!art::decode(word,source.data,source.count-1));
 CHECK(!art::decode(word,source.data,source.count-2));
 CHECK(!art::decode(word,source.data,size_t(width)*128*2+2));
 const unsigned short zero[]={0,0},badValue[]={1,256},overflow[]={65535,0},incomplete[]={1,255};
 CHECK(!art::decode(word,zero,2)&&!art::decode(word,badValue,2));
 CHECK(!art::decode(word,overflow,2)&&!art::decode(word,incomplete,2));
 std::vector<unsigned short>empty{static_cast<unsigned short>(width*128),0};
 CHECK(!art::decode(word,empty.data(),empty.size()));
 for(size_t offset:{size_t(0),size_t(1),source.count-1}){
  std::vector<unsigned short>bad(source.data,source.data+source.count);
  if(offset==0)bad[offset]=0;else bad[offset]^=1;
  CHECK(!art::decode(word,bad.data(),bad.size()));
 }
 // An authentic image for another fixed identity is still not interchangeable.
 const unsigned other=word%3+1;const Runs wrong=sources[other-1];
 CHECK(!art::decode(word,wrong.data,wrong.count));
 CHECK(image->rgba==bytes&&alias->rgba==bytes&&image->sha==sha&&decoded->rgba==decodedBytes);
 CHECK(art::artwork(word)==image);
}
int main(){
 for(unsigned word:{0u,4u,std::numeric_limits<unsigned>::max()}){
  CHECK(!art::artwork(word));
  CHECK(!art::decode(word,reinterpret_cast<const unsigned short*>(uintptr_t(1)),2));
 }
 for(unsigned word=1;word<=3;++word)imageChecks(word);
 CHECK(art::artwork(1)!=art::artwork(2)&&art::artwork(2)!=art::artwork(3));
 std::puts("PASS three pinned compact caption images, geometry, full RGBA digests, four padded edges, immutable shared/copied ownership and inert malformed RLE rejection");
}
