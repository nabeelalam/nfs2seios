// SPDX-License-Identifier: MIT
#include "AudioArtworkPackFixture.h"
int main(){try{
 AudioPackFixture f;std::string error;auto pack=ArtworkPack::loadOwned(f.bytes,sha(f.bytes),&error);CHECK(pack&&error.empty());auto p=pack->audioArtProvider();CHECK(p.schema==NFS_AUDIO_ART_PROVIDER_SCHEMA&&p.atlas&&p.user);auto meta=pack->audioSourceIdentities();CHECK(meta&&nfs_audio_frame::SourceCatalog::create(*meta,f.sources));NfsAudioArt art{};CHECK(p.atlas(p.user,&art));CHECK(art.identity==0xabc123&&art.bytes==NFS_AUDIO_ART_BYTES&&art.premultiplied==1&&art.opacityBaked==1);Digest artSha{};std::copy_n(art.sha256,32,artSha.begin());CHECK(nfs_audio_frame::ArtBinding::create(art,artSha));CHECK(!p.atlas(nullptr,&art)&&!p.atlas(p.user,nullptr));
 // The immutable pack owns all provider pixels after input buffers are released.
 auto saved=art.pixels;f.bytes[ f.atlas+112 ]=99;CHECK(p.atlas(p.user,&art)&&art.pixels==saved&&art.pixels[0]==20);f.bytes[f.atlas+112]=20;
 for(unsigned k=0;k<27;++k){auto old=read64(f.legacy.bytes,72+32*k),now=read64(f.bytes,72+32*k),n=read64(f.bytes,80+32*k);CHECK(n==read64(f.legacy.bytes,80+32*k)&&!std::memcmp(f.legacy.bytes.data()+old,f.bytes.data()+now,n));}
 auto legacy=ArtworkPack::loadOwned(f.legacy.bytes,sha(f.legacy.bytes));CHECK(legacy&&!legacy->audioArtProvider().schema&&!legacy->audioSourceIdentities());Fixture v1;auto old=ArtworkPack::loadOwned(v1.bytes,sha(v1.bytes));CHECK(old&&!old->audioArtProvider().schema&&!old->audioSourceIdentities());
 unsigned rejects=0;auto reject=[&](const std::function<void(Bytes&)>&change){auto b=f.bytes;change(b);CHECK(!ArtworkPack::loadOwned(b,sha(b)));++rejects;};
 reject([](Bytes&b){b[7]='3';p32(b,8,3);});reject([](Bytes&b){p32(b,24,27);});reject([](Bytes&b){p32(b,64+32*27,9);});reject([](Bytes&b){p32(b,68+32*28,1);});reject([](Bytes&b){p64(b,72+32*28,UINT64_MAX);});reject([](Bytes&b){p64(b,80+32*27,UINT64_MAX);});
 for(unsigned off:{0u,4u,8u,12u})reject([&](Bytes&b){b[f.catalog+off]^=1;});
 for(unsigned k=0;k<6;++k)for(unsigned off:{0u,8u,12u,16u,20u,24u,28u,32u,36u,40u})reject([&](Bytes&b){b[f.catalog+16+80*k+off]^=1;});
 for(unsigned k=0;k<6;++k)reject([&](Bytes&b){p32(b,f.catalog+16+80*k+4,0);});
 reject([&](Bytes&b){fill(b,f.catalog+16+48,0);});
 for(unsigned off:{0u,4u,8u,12u,16u,20u,24u,28u,40u,48u,80u,112u})reject([&](Bytes&b){b[f.atlas+off]^=1;});
 reject([&](Bytes&b){p64(b,f.atlas+32,0);});
 reject([&](Bytes&b){b[f.atlas+112]=255;b[f.atlas+115]=1;CHECK(nfs_texture_sha256(b.data()+f.atlas+112,NFS_AUDIO_ART_BYTES,b.data()+f.atlas+48));});
 auto wrong=sha(f.bytes);wrong[0]^=1;CHECK(!ArtworkPack::loadOwned(f.bytes,wrong));
 std::printf("PASS Audio pack v4, legacy sections preserved, %u structural rejection cases\n",rejects);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
