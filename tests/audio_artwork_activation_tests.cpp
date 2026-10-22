// SPDX-License-Identifier: MIT
#include "AudioArtworkPackFixture.h"
#include "hd/ArtworkActivation.h"
#include "hd/AudioRuntimeCatalog.h"
#include "hd/BackgroundSceneCatalog.h"
#include <filesystem>
int main(int argc,char**argv){try{
 CHECK(argc==3);std::string root=argv[1],mode=argv[2];CHECK(mode=="retained"||mode=="sealed-optional");
 AudioPackFixture fixture;auto expected=sha(fixture.bytes);std::filesystem::create_directories(root+"/gamedata");auto path=root+"/gamedata/remaster.nfshd";{std::ofstream f(path,std::ios::binary);CHECK(bool(f.write(reinterpret_cast<const char*>(fixture.bytes.data()),fixture.bytes.size())));}
 CHECK(!nfs_hd_audio_art_provider_available()&&!nfs_hd_art_provider_available()&&!nfs_audio_runtime::acquireTrustedAssets());
 if(mode=="sealed-optional"){NfsAudioArt a{};CHECK(!nfs_hd_audio_art(&a));}
 CHECK(nfs_hd::activateArtworkPack(root,expected));std::filesystem::remove(path);fixture.bytes.clear();fixture.sources={};CHECK(nfs_hd_art_provider_available()&&nfs_background_scene_runtime::catalogConfigured());
 NfsAudioArt a{};if(mode=="retained"){CHECK(nfs_hd_audio_art_provider_available()&&nfs_hd_audio_art(&a));auto trusted=nfs_audio_runtime::acquireTrustedAssets();CHECK(trusted&&trusted->sources()[0].width==183&&trusted->art()->view().identity==0xabc123);CHECK(a.bytes==NFS_AUDIO_ART_BYTES&&a.identity==0xabc123&&a.pixels[0]==20&&a.pixels[3]==100);Digest expectedArt{};std::copy_n(a.sha256,32,expectedArt.begin());CHECK(nfs_audio_frame::ArtBinding::create(a,expectedArt));}else{CHECK(!nfs_hd_audio_art_provider_available()&&!nfs_hd_audio_art(&a)&&!nfs_audio_runtime::acquireTrustedAssets());}
 CHECK(nfs_hd::activateArtworkPack(root,expected));auto wrong=expected;wrong[0]^=1;CHECK(!nfs_hd::activateArtworkPack(root,wrong));NfsSmallLogoArt logo{};CHECK(nfs_private_small_logo_art(&logo)&&logo.pixels[0]==60);std::printf("PASS Audio startup %s\n",mode.c_str());return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
