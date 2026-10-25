// SPDX-License-Identifier: MIT
#include "FeedbackArtworkPackFixture.h"
int main(){try{
 // Exact old formats continue loading, and do not accidentally expose Feedback.
 Fixture v1;NativeFixture v2(2,3,false),v3(2,3,true);AudioPackFixture v4;
 for(const Bytes*b:{&v1.bytes,&v2.bytes,&v3.bytes,&v4.bytes}){auto p=ArtworkPack::loadOwned(*b,sha(*b));CHECK(p&&!p->feedbackBackdropProvider().view);}
 FeedbackFixture absent(false);auto noFeedback=ArtworkPack::loadOwned(absent.bytes,sha(absent.bytes));CHECK(noFeedback&&!noFeedback->feedbackBackdropProvider().view&&noFeedback->audioArtProvider().atlas&&noFeedback->nativeMarkProvider().lookup);
 FeedbackFixture f;std::string error;auto pin=sha(f.bytes);auto pack=ArtworkPack::loadOwned(f.bytes,pin,&error);CHECK(pack&&error.empty());auto provider=pack->feedbackBackdropProvider();CHECK(provider.schema==1&&provider.user&&provider.view);
 NfsFeedbackModalBackdropArtView view{};CHECK(provider.view(provider.user,&view));CHECK(view.schema==1&&view.count==2);
 for(unsigned k=0;k<2;++k){const auto&i=view.images[k];CHECK(i.schema==1&&i.role==k+1&&i.straightRgba==1&&i.width==(k?468u:1376u)&&i.height==(k?244u:188u));CHECK(i.bytes==size_t(i.width)*i.height*4&&i.rgba&&i.rgba[0]==251&&i.rgba[3]==k);Digest d{};CHECK(nfs_texture_sha256(i.rgba,i.bytes,d.data())&&!std::memcmp(d.data(),i.sha256,32));}
 NfsFeedbackModalBackdropArtView unchanged;std::memset(&unchanged,0x5a,sizeof unchanged);auto before=unchanged;CHECK(!provider.view(nullptr,&unchanged)&&!std::memcmp(&unchanged,&before,sizeof unchanged));CHECK(!provider.view(provider.user,nullptr));
 // Clearing caller-owned input cannot invalidate the retained immutable pack.
 auto independent=f.bytes;auto retained=ArtworkPack::loadOwned(independent,sha(independent));CHECK(retained);independent[f.imageAt[0]+112]=7;independent.clear();auto retainedProvider=retained->feedbackBackdropProvider();CHECK(retainedProvider.view(retainedProvider.user,&view)&&view.images[0].rgba[0]==251);
 auto wrong=pin;wrong[0]^=1;CHECK(!ArtworkPack::loadOwned(f.bytes,wrong));Digest empty{};CHECK(!ArtworkPack::loadOwned(f.bytes,empty));
 unsigned rejected=0;auto reject=[&](Bytes b){std::string why;CHECK(!ArtworkPack::loadOwned(b,sha(b),&why)&&!why.empty());++rejected;};auto mutate=[&](auto change){auto b=f.bytes;change(b);reject(std::move(b));};
 mutate([](Bytes&b){b[7]='4';p32(b,8,4);});
 mutate([](Bytes&b){p32(b,24,std::numeric_limits<uint32_t>::max());});
 mutate([&](Bytes&b){p32(b,f.imageTable,7);});
 mutate([&](Bytes&b){p32(b,f.imageTable+4,2);});
 mutate([&](Bytes&b){p32(b,f.imageTable+32+4,1);});
 mutate([&](Bytes&b){p32(b,f.imageTable+32,11);});
 mutate([&](Bytes&b){p64(b,f.imageTable+8,read64(b,f.imageTable+8)+1);});
 mutate([&](Bytes&b){p64(b,f.imageTable+16,std::numeric_limits<uint64_t>::max());});
 mutate([&](Bytes&b){p64(b,f.imageTable+24,1);});
 for(unsigned k=0;k<2;++k){const size_t at=f.imageAt[k];
  for(unsigned field:{0u,4u,8u,12u,16u})mutate([&](Bytes&b){p32(b,at+field,read32(b,at+field)^1u);});
  for(unsigned field:{20u,24u,28u,32u,80u})mutate([&](Bytes&b){b[at+field]=1;});
  mutate([&](Bytes&b){p64(b,at+40,std::numeric_limits<uint64_t>::max());});
  mutate([&](Bytes&b){b[at+48]^=1;});
  mutate([&](Bytes&b){std::fill_n(b.begin()+at+48,32,0);});
  mutate([&](Bytes&b){b[at+112]^=1;});
  // A straight pixel may be reauthored independently of alpha; loader preserves it.
  auto b=f.bytes;b[at+112]=255;b[at+115]=0;const size_t n=size_t(read64(b,at+40));CHECK(nfs_texture_sha256(b.data()+at+112,n,b.data()+at+48));auto p=ArtworkPack::loadOwned(b,sha(b));CHECK(p);auto q=p->feedbackBackdropProvider();NfsFeedbackModalBackdropArtView w{};CHECK(q.view(q.user,&w)&&w.images[k].rgba[0]==255&&w.images[k].rgba[3]==0);
 }
 // Construct a genuine one-image v5 tail with an otherwise consistent table.
 {auto one=f.bytes;const unsigned count=read32(one,24);one.resize(f.imageAt[1]);one.erase(one.begin()+64+32*(count-1),one.begin()+64+32*count);p32(one,24,count-1);for(unsigned k=0;k<count-1;++k)p64(one,72+32*k,read64(one,72+32*k)-32);p64(one,16,one.size());reject(std::move(one));}
 // Truncation, orphan pair and extra bytes cannot be hidden by a valid outer pin.
 mutate([](Bytes&b){b.pop_back();p64(b,16,b.size());});
 mutate([](Bytes&b){b.push_back(0);p64(b,16,b.size());});
 mutate([&](Bytes&b){p32(b,f.imageTable+32,7);});
 std::printf("PASS v1-v4 compatibility, optional v5 straight-RGBA pair, retained callbacks, %u structural negatives\n",rejected);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
