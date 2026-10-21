// SPDX-License-Identifier: MIT
#include "HdArtworkPackFixture.h"
#include "hd/ArtworkActivation.h"
#include "hd/BackgroundSceneCatalog.h"
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif
static void directory(const std::string&p){
#ifdef _WIN32
 _mkdir(p.c_str());
#else
 mkdir(p.c_str(),0700);
#endif
}
int main(int argc,char**argv){try{
 CHECK(argc==3);const std::string root=argv[1],mode=argv[2];
#ifdef _WIN32
 _putenv_s("NFS_PRIVATE_BACKGROUND_SCENE_HD","");
#else
 unsetenv("NFS_PRIVATE_BACKGROUND_SCENE_HD");
#endif
 Fixture f;const auto expected=sha(f.bytes);directory(root);directory(root+"/gamedata");const std::string path=root+"/gamedata/remaster.nfshd";
 CHECK(!nfs_hd_art_provider_available()&&!nfs_background_scene_runtime::catalogConfigured());
 if(mode!="missing"){std::ofstream file(path,std::ios::binary);file.write(reinterpret_cast<const char*>(f.bytes.data()),f.bytes.size());CHECK(bool(file));}
 if(mode=="disabled"){CHECK(!nfs_hd::activateConfiguredArtwork(root));CHECK(!nfs_hd_art_provider_available()&&!nfs_background_scene_runtime::catalogConfigured()&&!std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_HD"));}
 else if(mode=="missing"){CHECK(!nfs_hd::activateArtworkPack(root,expected));CHECK(!nfs_hd_art_provider_available()&&!nfs_background_scene_runtime::catalogConfigured());}
 else if(mode=="wrong-pin"){auto wrong=expected;wrong[0]^=1;CHECK(!nfs_hd::activateArtworkPack(root,wrong));CHECK(!nfs_hd_art_provider_available()&&!nfs_background_scene_runtime::catalogConfigured());}
 else if(mode=="getter-already-sealed"){NfsSmallLogoArt l{};CHECK(!nfs_private_small_logo_art(&l));CHECK(!nfs_hd::activateArtworkPack(root,expected)&&!nfs_hd_art_provider_available());CHECK(!std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_HD"));}
 else if(mode=="provider-already-installed"){auto p=ArtworkPack::loadOwned(f.bytes,expected);CHECK(p);auto a=p->provider();CHECK(nfs_hd_install_art_provider(&a));CHECK(!nfs_hd::activateArtworkPack(root,expected)&&!nfs_background_scene_runtime::catalogConfigured());NfsSmallLogoArt l{};CHECK(nfs_private_small_logo_art(&l)&&l.pixels[0]==60);CHECK(!std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_HD"));}
 else if(mode=="retained-success"){
  CHECK(nfs_hd::activateArtworkPack(root,expected)&&nfs_hd_art_provider_available()&&nfs_background_scene_runtime::catalogConfigured());CHECK(std::string(std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_HD"))=="1");std::remove(path.c_str());
  NfsSmallLogoArt l{};CHECK(nfs_private_small_logo_art(&l)&&l.width==32&&l.pixels[0]==60);auto c=nfs_background_scene_runtime::acquireCatalog();CHECK(c&&c->identities[0].width==343&&!nfs_background_scene_runtime::installCatalog(c));CHECK(nfs_hd::activateArtworkPack(root,expected));auto wrong=expected;wrong[0]^=1;CHECK(!nfs_hd::activateArtworkPack(root,wrong));CHECK(nfs_private_small_logo_art(&l)&&l.pixels[0]==60);
 }else CHECK(false);
 std::remove(path.c_str());std::printf("PASS artwork startup %s\n",mode.c_str());return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
