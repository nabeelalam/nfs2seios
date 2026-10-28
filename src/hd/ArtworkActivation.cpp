// SPDX-License-Identifier: MIT
#include "ArtworkActivation.h"
#ifdef NFS_CLASSIC_UI
#include "SetupEmblemProvider.h"
#endif
#ifdef NFS_CLASSIC_BUNDLED_ART
#include "OwnedArtwork.h"
#endif
#include "BackgroundSceneCatalog.h"
#include "SoftwareFontBuiltin.h"
#include "AudioRuntimeCatalog.h"
#include <cstdlib>
#include <memory>
#ifdef NFS_HD_ARTWORK_CONFIG_HEADER
#include NFS_HD_ARTWORK_CONFIG_HEADER
#endif
#ifndef NFS_HD_ARTWORK_SHA256
#define NFS_HD_ARTWORK_SHA256 ""
#endif
namespace nfs_hd {
namespace {
// Intentional process-lifetime ownership matches the immutable C provider ABI.
// Allocate this holder before installation; assigning a shared_ptr cannot throw.
std::shared_ptr<const ArtworkPack>& retained(){static auto*p=new std::shared_ptr<const ArtworkPack>;return *p;}
#ifdef NFS_CLASSIC_UI
std::shared_ptr<const SetupEmblemProvider>&retainedSetupEmblems(){static auto*p=new std::shared_ptr<const SetupEmblemProvider>;return *p;}
#endif
bool enableVariable(const char*name)noexcept{
#ifdef _WIN32
 return _putenv_s(name,"1")==0;
#else
 return setenv(name,"1",1)==0;
#endif
}
bool enableScene()noexcept{
 if(!enableVariable("NFS_PRIVATE_BACKGROUND_SCENE_HD"))return false;
#ifdef NFS_CLASSIC_UI
 const char*names[]={"NFS_PRIVATE_LOADSAVE_HD","NFS_PRIVATE_LOADSAVE_WORD_ART","NFS_PRIVATE_LOADSAVE_FULLSCENE","NFS_PRIVATE_RECORDS_HD","NFS_PRIVATE_STATS_UI","NFS_PRIVATE_RESULTS_HD","NFS_PRIVATE_VICTORY_HD","NFS_PRIVATE_CREDITS_UI","NFS_PRIVATE_LOADSAVE_CHILD_UI"};
 for(const auto*name:names)if(!enableVariable(name))return false;
#endif
 return true;
}
int hex(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
bool pin(Digest&d){const char*s=NFS_HD_ARTWORK_SHA256;for(unsigned k=0;k<64;++k)if(!s[k]||hex(s[k])<0)return false;if(s[64])return false;for(unsigned k=0;k<32;++k)d[k]=uint8_t(hex(s[2*k])*16+hex(s[2*k+1]));return true;}
}
static bool installArtworkPack(std::shared_ptr<const ArtworkPack>pack,const Digest&expected)noexcept{
    try{
        auto&owner=retained();if(owner)return owner->digest()==expected&&enableScene();
        if(nfs_hd_art_provider_available())return false;
        if(!pack)return false;
        if(!nfs_background_scene_runtime::installCatalog(pack->catalog()))return false;
        const auto provider=pack->provider();if(!nfs_hd_install_art_provider(&provider))return false;
        owner=std::move(pack);
        // Retain ownership before publishing optional borrowed mark pixels.
        // A missing/sealed mark registry preserves originals, not valid scenes.
        auto marks=owner->nativeMarkProvider();
#ifdef NFS_CLASSIC_UI
        auto&setup=retainedSetupEmblems();setup=SetupEmblemProvider::create(owner);
        if(setup)marks=setup->provider();
#endif
        if(marks.schema)(void)nfs_hd_install_native_mark_provider(&marks);
        const auto feedback=owner->feedbackBackdropProvider();
        if(feedback.schema)(void)nfs_hd_install_feedback_modal_backdrop_art_provider(&feedback);
        const auto audio=owner->audioArtProvider();
        if(audio.schema&&nfs_hd_install_audio_art_provider(&audio)){
            NfsAudioArt art{};const auto*metadata=owner->audioSourceIdentities();
            if(metadata&&audio.atlas(audio.user,&art)){
                nfs_audio_runtime::Digest expectedArt{};std::copy_n(art.sha256,32,expectedArt.begin());
                auto trusted=nfs_audio_runtime::TrustedAssets::create(*metadata,owner->catalog()->mapSha,art,expectedArt);
                if(trusted)(void)nfs_audio_runtime::installTrustedAssets(std::move(trusted));
            }
        }
        return enableScene();
    }catch(...){return false;}
}
bool activateArtworkPack(const std::string&root,const Digest&expected)noexcept{try{if(retained())return retained()->digest()==expected&&enableScene();if(nfs_hd_art_provider_available())return false;return installArtworkPack(ArtworkPack::loadFile(root+"/gamedata/remaster.nfshd",expected),expected);}catch(...){return false;}}

bool activateConfiguredArtwork(const std::string&root,const std::string&bundledTemplate)noexcept{
 // Called by app startup before GameSession creates a worker. Font failure is
 // optional and cannot change existing artwork-pack or original-rendering paths.
 (void)nfs_soft::installFontCatalog(nfs_soft::createBuiltinFontCatalog());
 Digest d{};if(!pin(d))return false;if(activateArtworkPack(root,d))return true;
#ifdef NFS_CLASSIC_BUNDLED_ART
 if(!bundledTemplate.empty())return installArtworkPack(loadBundledOwnedArtwork(root,bundledTemplate,d),d);
#else
 (void)bundledTemplate;
#endif
 return false;
}
bool activateConfiguredArtwork(const std::string&root)noexcept{return activateConfiguredArtwork(root,std::string());}

}
