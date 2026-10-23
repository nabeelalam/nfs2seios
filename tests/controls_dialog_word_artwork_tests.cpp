// SPDX-License-Identifier: MIT
#include "hd/ControlsDialogWordArtwork.h"
#include "hd/AudioDialogWordArtwork.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <type_traits>
#include <utility>
namespace art=nfs_controls_dialog_display;
#define CHECK(value) do{if(!(value)){std::fprintf(stderr,"Controls dialog artwork check failed at line %d\n",__LINE__);std::abort();}}while(0)
static_assert(std::is_same<decltype(art::artwork(1)),std::shared_ptr<const art::Artwork>>::value,"published image owners are immutable");
static_assert(std::is_same<decltype(*art::artwork(1)),const art::Artwork&>::value,"shared aliases cannot mutate artwork");
static_assert(std::is_same<decltype(std::declval<const art::Artwork&>().rgba.data()),const unsigned char*>::value,"published pixel spans are const");
struct Runs {const unsigned short*data;size_t count;};
static const Runs sources[5]={
 {nfs_controls_dialog_word_alpha_rle_1,sizeof(nfs_controls_dialog_word_alpha_rle_1)/sizeof(unsigned short)},
 {nfs_controls_dialog_word_alpha_rle_2,sizeof(nfs_controls_dialog_word_alpha_rle_2)/sizeof(unsigned short)},
 {nfs_controls_dialog_word_alpha_rle_3,sizeof(nfs_controls_dialog_word_alpha_rle_3)/sizeof(unsigned short)},
 {nfs_controls_dialog_word_alpha_rle_4,sizeof(nfs_controls_dialog_word_alpha_rle_4)/sizeof(unsigned short)},
 {nfs_controls_dialog_word_alpha_rle_5,sizeof(nfs_controls_dialog_word_alpha_rle_5)/sizeof(unsigned short)}};
static const char*expectedHex[5]={
 "1776f2c15b5936b59f44a5346ed0524149026b4a1f8f8b666ab400bc6db31f53",
 "8129b04761c19702a89f5a705ac5252db8a92ac50dd0e70d8feacd529ec0d197",
 "8c68421bdcb0a2a5eaa3f89c5cf97104da5989ab241e6669a4035719b590320f",
 "603163b4df5f7b5b02a26906c60827f681593d56a1ca6ca14159665302812bdc",
 "dcca47f4cdceb4011aff18d22fb419c5bbd3ebb4abdd8a7abca1e74853d447aa"};
static unsigned nibble(char c){return c<='9'?unsigned(c-'0'):unsigned(c-'a'+10);}
static void imageChecks(unsigned word){
 auto image=art::artwork(word);CHECK(image);
 static const unsigned widths[5]={272,144,492,384,140};
 const unsigned width=widths[word-1];
 CHECK(image->word==word&&image->width==width&&image->height==128&&image->rgba.size()==size_t(width)*128*4);
 unsigned char digest[32];CHECK(nfs_texture_sha256(image->rgba.data(),image->rgba.size(),digest));
 CHECK(!std::memcmp(digest,nfs_controls_dialog_word_sha256[word-1],32));
 CHECK(!std::memcmp(digest,image->sha.data(),32));
 for(unsigned i=0;i<32;++i)CHECK(digest[i]==16*nibble(expectedHex[word-1][2*i])+nibble(expectedHex[word-1][2*i+1]));
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
 // Independently walk the finite alpha stream against every published pixel.
 size_t alphaIndex=0;
 for(size_t pair=0;pair<source.count;pair+=2){
  const unsigned run=source.data[pair],value=source.data[pair+1];
  CHECK(run && value<=255 && alphaIndex+run<=size_t(width)*128);
  for(unsigned j=0;j<run;++j)CHECK(bytes[(alphaIndex++)*4+3]==value);
 }
 CHECK(alphaIndex==size_t(width)*128);
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
 CHECK(!art::decode(word,reinterpret_cast<const unsigned short*>(uintptr_t(1)),size_t(width)*128*2+2));
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
 const unsigned other=word%5+1;const Runs wrong=sources[other-1];
 CHECK(!art::decode(word,wrong.data,wrong.count));
 CHECK(image->rgba==bytes&&alias->rgba==bytes&&image->sha==sha&&decoded->rgba==decodedBytes);
 CHECK(art::artwork(word)==image);
}
int main(){
 for(unsigned word:{0u,6u,std::numeric_limits<unsigned>::max()}){
  CHECK(!art::artwork(word));
  CHECK(!art::decode(word,reinterpret_cast<const unsigned short*>(uintptr_t(1)),2));
 }
 for(unsigned word=1;word<=5;++word)imageChecks(word);
 CHECK(art::artwork(1)!=art::artwork(2)&&art::artwork(2)!=art::artwork(3));
 // Reuse pixel identity, never another scene's cache owner or placement state.
 auto done=art::artwork(5);auto audio=nfs_audio_dialog_display::artwork(9);
 CHECK(done&&audio&&done->word==5&&audio->word==9);
 CHECK(done->width==140&&audio->width==140&&done->height==128&&audio->height==128);
 CHECK(done->rgba==audio->rgba&&done->sha==audio->sha);
 CHECK(static_cast<const void*>(done.get())!=static_cast<const void*>(audio.get()));
 CHECK(done->rgba.data()!=audio->rgba.data());
 CHECK(art::artwork(5)==done&&nfs_audio_dialog_display::artwork(9)==audio);
 std::puts("PASS five pinned Controls dialog images, geometry, full RGBA digests, four padded edges, immutable shared/copied ownership and inert malformed RLE rejection");
}
