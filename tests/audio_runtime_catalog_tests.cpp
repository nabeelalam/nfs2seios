// SPDX-License-Identifier: MIT
#include "AudioPacketFixture.h"
#include "hd/AudioRuntimeCatalog.h"
#include <thread>
#include <atomic>
#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(0)
int main(){try{
 namespace ar=nfs_audio_runtime;auto f=std::unique_ptr<nfs_audio_test::Fixture>(new nfs_audio_test::Fixture);auto meta=f->identities;std::array<ar::Digest,2>maps{};maps[0][0]=1;maps[1][0]=2;auto view=f->art->view();auto expected=f->artSha;
 CHECK(!ar::acquireTrustedAssets()&&!ar::installTrustedAssets({}));auto trusted=ar::TrustedAssets::create(meta,maps,view,expected);CHECK(trusted&&trusted->sources()[0].sha==meta[0].sha&&trusted->maps()==maps&&trusted->art()->view().pixels!=view.pixels);
 unsigned negatives=0;auto reject=[&](auto change){auto m=meta;change(m);CHECK(!ar::TrustedAssets::create(m,maps,view,expected));++negatives;};
 for(unsigned k=0;k<6;++k){reject([&](auto&m){m[k].sha={};});reject([&](auto&m){m[k].span=0;});reject([&](auto&m){m[k].span=UINT32_MAX;});reject([&](auto&m){m[k].width++;});reject([&](auto&m){m[k].height++;});reject([&](auto&m){m[k].hotX++;});reject([&](auto&m){m[k].hotY++;});reject([&](auto&m){m[k].paletteCount=0;});reject([&](auto&m){m[k].paletteCount=257;});reject([&](auto&m){m[k].paletteOffset=UINT32_MAX;});reject([&](auto&m){m[k].header[0]^=1;});reject([&](auto&m){m[k].header[1]^=1;});reject([&](auto&m){m[k].header[2]^=1;});}
 auto badmaps=maps;badmaps[0]={};CHECK(!ar::TrustedAssets::create(meta,badmaps,view,expected));badmaps=maps;badmaps[1]=badmaps[0];CHECK(!ar::TrustedAssets::create(meta,badmaps,view,expected));auto badpin=expected;badpin[0]^=1;CHECK(!ar::TrustedAssets::create(meta,maps,view,badpin));
 CHECK(ar::installTrustedAssets(trusted)&&!ar::installTrustedAssets(trusted));meta={};maps={};f.reset();CHECK(ar::acquireTrustedAssets()==trusted&&trusted->art()->view().pixels[3]<=255);std::atomic<unsigned>good{0};std::array<std::thread,4>workers;for(auto&t:workers)t=std::thread([&]{for(unsigned n=0;n<1000;++n){auto p=ar::acquireTrustedAssets();if(p==trusted&&p->sources()[0].width==183)++good;}});for(auto&t:workers)t.join();CHECK(good==4000);std::printf("PASS Audio trusted runtime catalog, %u metadata negatives, 4000 concurrent reads\n",negatives);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
