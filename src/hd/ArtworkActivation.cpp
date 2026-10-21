// SPDX-License-Identifier: MIT
#include "ArtworkActivation.h"
#include "BackgroundSceneCatalog.h"
#include <cstdlib>
#include <memory>
#ifndef NFS_HD_ARTWORK_SHA256
#define NFS_HD_ARTWORK_SHA256 ""
#endif
namespace nfs_hd {
namespace {
// Intentional process-lifetime ownership matches the immutable C provider ABI.
// Allocate this holder before installation; assigning a shared_ptr cannot throw.
std::shared_ptr<const ArtworkPack>& retained(){static auto*p=new std::shared_ptr<const ArtworkPack>;return *p;}
bool enableScene()noexcept{
#ifdef _WIN32
 return _putenv_s("NFS_PRIVATE_BACKGROUND_SCENE_HD","1")==0;
#else
 return setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD","1",1)==0;
#endif
}
int hex(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
bool pin(Digest&d){const char*s=NFS_HD_ARTWORK_SHA256;for(unsigned k=0;k<64;++k)if(!s[k]||hex(s[k])<0)return false;if(s[64])return false;for(unsigned k=0;k<32;++k)d[k]=uint8_t(hex(s[2*k])*16+hex(s[2*k+1]));return true;}
}
bool activateArtworkPack(const std::string&root,const Digest&expected)noexcept{
    try{
        auto&owner=retained();if(owner)return owner->digest()==expected&&enableScene();
        if(nfs_hd_art_provider_available())return false;
        auto pack=ArtworkPack::loadFile(root+"/gamedata/remaster.nfshd",expected);
        if(!pack)return false;
        if(!nfs_background_scene_runtime::installCatalog(pack->catalog()))return false;
        const auto provider=pack->provider();if(!nfs_hd_install_art_provider(&provider))return false;
        owner=std::move(pack);
        return enableScene();
    }catch(...){return false;}
}
bool activateConfiguredArtwork(const std::string&root)noexcept{Digest d{};if(!pin(d))return false;return activateArtworkPack(root,d);}
}
