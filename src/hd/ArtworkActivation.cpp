// SPDX-License-Identifier: MIT
#include "ArtworkActivation.h"
#ifdef NFS_CLASSIC_UI
#include "SetupEmblemProvider.h"
#endif
#ifdef NFS_CLASSIC_BUNDLED_ART
#include "OwnedArtwork.h"
#include "WideMenuArtwork.h"
#include "ResultsBackdropArtwork.h"
#include "ClassicScreens/IntroArtworkLoader.h"
#include "ClassicScreens/IntroArtworkPin.h"
#include "ClassicScreens/IntroArtProvider.h"
#include "ClassicScreens/TrackInfoArtworkLoader.h"
#include "ClassicScreens/TrackInfoArtworkPin.h"
#include "ClassicScreens/TrackInfoArtProvider.h"
#include "ClassicScreens/CarInfoArtworkLoader.h"
#include "ClassicScreens/CarInfoArtworkPin.h"
#include "ClassicScreens/CarInfoArtProvider.h"
#include "ClassicScreens/HistoryArtworkLoader.h"
#include "ClassicScreens/HistoryArtworkPin.h"
#include "ClassicScreens/HistoryArtProvider.h"
#include "SlideshowStartup.h"
#if defined(__has_include)
#if __has_include("hd/ClassicScreens/private/SlideshowFooterData.h")
#include "SlideshowFooterResource.h"
#include "hd/ClassicScreens/private/SlideshowFooterData.h"
#define NFS_PRIVATE_SLIDESHOW_FOOTER_PINS 1
#endif
#endif
#include "PlayerPhotoArtProvider.h"
#include "PlayerPhotoArtworkLoader.h"
#include "PlayerPhotoArtworkPin.h"
#include "TrackBadgeArtProvider.h"
#include "TrackBadgeArtworkLoader.h"
#include "TrackBadgeArtworkPin.h"
#include <cstdio>
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
 const char*names[]={"NFS_PRIVATE_LOADSAVE_HD","NFS_PRIVATE_LOADSAVE_WORD_ART","NFS_PRIVATE_LOADSAVE_FULLSCENE","NFS_PRIVATE_RECORDS_HD","NFS_PRIVATE_STATS_UI","NFS_PRIVATE_RESULTS_HD","NFS_PRIVATE_VICTORY_HD","NFS_PRIVATE_LOADING_UI","NFS_PRIVATE_TRACK_INFO_UI","NFS_PRIVATE_CAR_INFO_UI","NFS_PRIVATE_HISTORY_UI","NFS_PRIVATE_INTRO_UI","NFS_PRIVATE_CREDITS_UI","NFS_PRIVATE_LOADSAVE_CHILD_UI"};
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
 Digest d{};if(!pin(d))return false;bool activated=activateArtworkPack(root,d);
#ifdef NFS_CLASSIC_BUNDLED_ART
 if(!activated&&!bundledTemplate.empty())activated=installArtworkPack(loadBundledOwnedArtwork(root,bundledTemplate,d),d);
 if(activated){
  try{
   const auto slash=bundledTemplate.find_last_of("/\\");
   const std::string directory=slash==std::string::npos?root+"/gamedata/":bundledTemplate.substr(0,slash+1);
   const char*overridePath=std::getenv("NFS_PRIVATE_SLIDESHOW_PACK");
   const char*preview=std::getenv("NFS_PRIVATE_SLIDESHOW_PREVIEW");
   nfs_slideshow::setStartup(overridePath&&*overridePath?overridePath:directory+"slideshow-ui.ownedz",preview&&preview[0]=='1'&&!preview[1]);
#ifdef NFS_PRIVATE_SLIDESHOW_FOOTER_PINS
   // Private generated identities, never routing read from an untrusted pack.
   const auto footerPath=directory+"slideshow-footer.ownedz";
   nfs_slideshow::setStartupFooters(nfs_slideshow_footer::loadResource(
       footerPath.c_str(),nfs_slideshow_footer_private::fileBytes,
       nfs_slideshow_footer_private::images,
       sizeof(nfs_slideshow_footer_private::images)/sizeof(nfs_slideshow_footer_private::images[0]),
       nfs_slideshow_footer_private::bindings()));
#endif
  }catch(...){}
 }
 if(activated){
  try{
   NfsWideMenuLayers existing{};
   if(!nfs_wide_menu_layers(0,&existing)){
    const auto slash=bundledTemplate.find_last_of("/\\");
    const std::string directory=slash==std::string::npos?root+"/gamedata/":bundledTemplate.substr(0,slash+1);
    auto wide=std::make_unique<WideMenuArtwork>();
    if(wide->load(directory+"wide-menu-ui.ownedz")){
     const auto a=wide->layers(0),b=wide->layers(1);
     const NfsWideMenuLayers layers[2]={{a.width,a.height,a.map,a.panel},{b.width,b.height,b.map,b.panel}};
     if(nfs_wide_menu_install_layers(layers)) (void)wide.release(); // Immutable process-lifetime provider.
    }
   }
  }catch(...){}
 }
 if(activated){
  try{
   NfsResultsBackdropImage image{};
   if(!nfs_results_backdrop_image(0,&image)){
    const auto slash=bundledTemplate.find_last_of("/\\");
    const std::string directory=slash==std::string::npos?root+"/gamedata/":bundledTemplate.substr(0,slash+1);
    (void)loadResultsBackdrops(directory+"results-backdrop-ui.ownedz");
   }
  }catch(...){}
 }
 if(activated&&!nfs_intro_art_generation()){
  try{const auto slash=bundledTemplate.find_last_of("/\\");const std::string intro=slash==std::string::npos?root+"/gamedata/"+NFS_INTRO_ARTWORK_FILENAME:bundledTemplate.substr(0,slash+1)+NFS_INTRO_ARTWORK_FILENAME;char error[128]{};
   if(!nfs_intro_art_load_file(intro.c_str(),error,sizeof error))std::fprintf(stderr,"Optional intro artwork: %s\n",error);
  }catch(...){}
 }

 // An empty pin deliberately disables unfinished track-info artwork. Install
 // immutable scene assets before the worker can seal their provider registry.
 if(activated&&NFS_TRACK_INFO_ARTWORK_SHA256[0]&&!nfs_track_info_art_generation()){
  try{const auto slash=bundledTemplate.find_last_of("/\\");const std::string trackInfo=slash==std::string::npos?root+"/gamedata/"+NFS_TRACK_INFO_ARTWORK_FILENAME:bundledTemplate.substr(0,slash+1)+NFS_TRACK_INFO_ARTWORK_FILENAME;char error[128]{};
   if(!nfs_track_info_art_load_file(trackInfo.c_str(),error,sizeof error))std::fprintf(stderr,"Optional track-info artwork: %s\n",error);
  }catch(...){}
 }
 if(activated&&NFS_CAR_INFO_ARTWORK_SHA256[0]&&!nfs_car_info_art_generation()){
  try{const auto slash=bundledTemplate.find_last_of("/\\");const std::string carInfo=slash==std::string::npos?root+"/gamedata/"+NFS_CAR_INFO_ARTWORK_FILENAME:bundledTemplate.substr(0,slash+1)+NFS_CAR_INFO_ARTWORK_FILENAME;char error[128]{};
   if(!nfs_car_info_art_load_file(carInfo.c_str(),error,sizeof error))std::fprintf(stderr,"Optional car-info artwork: %s\n",error);
  }catch(...){}
 }
 if(activated&&NFS_HISTORY_ARTWORK_SHA256[0]&&!nfs_history_art_generation()){
  try{const auto slash=bundledTemplate.find_last_of("/\\");const std::string history=slash==std::string::npos?root+"/gamedata/"+NFS_HISTORY_ARTWORK_FILENAME:bundledTemplate.substr(0,slash+1)+NFS_HISTORY_ARTWORK_FILENAME;char error[128]{};
   if(!nfs_history_art_load_file(history.c_str(),error,sizeof error))std::fprintf(stderr,"Optional history artwork: %s\n",error);
  }catch(...){}
 }
 if(activated&&NFS_PLAYER_PHOTO_ARTWORK_SHA256[0]&&!nfs_player_photo_art_generation()){
  try{const auto slash=bundledTemplate.find_last_of("/\\");const std::string playerPhoto=slash==std::string::npos?root+"/gamedata/"+NFS_PLAYER_PHOTO_ARTWORK_FILENAME:bundledTemplate.substr(0,slash+1)+NFS_PLAYER_PHOTO_ARTWORK_FILENAME;char error[128]{};
   if(!nfs_player_photo_art_load_file(playerPhoto.c_str(),error,sizeof error))std::fprintf(stderr,"Optional player-photo artwork: %s\n",error);
  }catch(...){}
 }
 if(activated&&NFS_TRACK_BADGE_ARTWORK_SHA256[0]&&!nfs_track_badge_art_generation()){
  try{const auto slash=bundledTemplate.find_last_of("/\\");const std::string trackBadge=slash==std::string::npos?root+"/gamedata/"+NFS_TRACK_BADGE_ARTWORK_FILENAME:bundledTemplate.substr(0,slash+1)+NFS_TRACK_BADGE_ARTWORK_FILENAME;char error[128]{};
   if(!nfs_track_badge_art_load_file(trackBadge.c_str(),error,sizeof error))std::fprintf(stderr,"Optional track-badge artwork: %s\n",error);
  }catch(...){}
 }

#else
 (void)bundledTemplate;
#endif
 return activated;
}
bool activateConfiguredArtwork(const std::string&root)noexcept{return activateConfiguredArtwork(root,std::string());}

}
