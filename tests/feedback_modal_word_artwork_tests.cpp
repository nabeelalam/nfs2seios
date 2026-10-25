// SPDX-License-Identifier: MIT
#include "../src/hd/FeedbackModalWordArtwork.h"
#include <cassert>
#include <type_traits>
int main(){
 namespace word=nfs_feedback_modal_word;
 static_assert(!std::is_default_constructible<word::Artwork>::value,"Artwork is validated only");
 auto title=word::artwork(1),message=word::artwork(2);
 assert(title&&message&&!word::artwork(0)&&!word::artwork(3));
 assert(title->width()==384&&title->height()==128&&title->word()==1);
 assert(message->width()==432&&message->height()==128&&message->word()==2);
 assert(title->rgba().data()==nfs_controls_dialog_display::artwork(4)->rgba.data());
 for(unsigned i=0;i<2;++i){auto a=word::artwork(i+1);std::array<unsigned char,32>sha{};
  assert(nfs_texture_sha256(a->rgba().data(),a->rgba().size(),sha.data()));
  assert(a->sha()==sha&&!std::memcmp(sha.data(),nfs_feedback_modal_word_sha256[i],32));
 }
 const auto*encoded=nfs_feedback_modal_word_message_alpha_rle;
 const size_t count=sizeof(nfs_feedback_modal_word_message_alpha_rle)/sizeof(unsigned short);
 assert(!word::decodeMessage(nullptr,count)&&!word::decodeMessage(encoded,0));
 assert(!word::decodeMessage(encoded,count-1)&&!word::decodeMessage(encoded,size_t(432)*128*2+2));
 std::vector<unsigned short>runs(encoded,encoded+count);
 auto owned=word::decodeMessage(runs.data(),runs.size());assert(owned);
 auto original=owned->rgba();runs[1]^=1;
 assert(owned->rgba()==original&&!word::decodeMessage(runs.data(),runs.size()));
 runs.assign(encoded,encoded+count);runs[0]=0;assert(!word::decodeMessage(runs.data(),runs.size()));
 runs.assign(encoded,encoded+count);runs[1]=256;assert(!word::decodeMessage(runs.data(),runs.size()));
 runs.assign(encoded,encoded+count);runs[0]=65535;assert(!word::decodeMessage(runs.data(),runs.size()));
 const unsigned short empty[]={55296,0};assert(!word::decodeMessage(empty,2));
 const unsigned short solid[]={55296,255};assert(!word::decodeMessage(solid,2));
 return 0;
}
