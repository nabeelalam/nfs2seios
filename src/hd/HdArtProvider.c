// SPDX-License-Identifier: MIT
#include "HdArtProvider.h"
#include <stdatomic.h>
static NfsHdArtProvider installed;
static atomic_int sealed;
static int ready;
static NfsWideMenuLayers wideLayers[2];
static atomic_int wideReady;
static NfsResultsBackdropImage resultsBackdrops[2];
static atomic_int resultsReady;
int nfs_results_install_backdrops(const NfsResultsBackdropImage images[2]){
    if(!images || images[0].width!=1440 || images[1].width!=1920 ||
       images[0].height!=1080 || images[1].height!=1080 ||
       !images[0].pixels || !images[1].pixels) return 0;
    int empty=0;
    if(!atomic_compare_exchange_strong(&resultsReady,&empty,1)) return 0;
    resultsBackdrops[0]=images[0];
    resultsBackdrops[1]=images[1];
    atomic_store_explicit(&resultsReady,2,memory_order_release);
    return 1;
}
int nfs_results_backdrop_image(unsigned ratio,NfsResultsBackdropImage *out){
    if(!out || ratio>=2 || atomic_load_explicit(&resultsReady,memory_order_acquire)!=2) return 0;
    *out=resultsBackdrops[ratio];
    return 1;
}
int nfs_wide_menu_install_layers(const NfsWideMenuLayers layers[2]){
    if(!layers || layers[0].width!=1180 || layers[0].height!=820 ||
       layers[1].width!=1280 || layers[1].height!=720 ||
       !layers[0].map || !layers[0].panel || !layers[1].map || !layers[1].panel) return 0;
    int empty=0;
    if(!atomic_compare_exchange_strong(&wideReady,&empty,1)) return 0;
    wideLayers[0]=layers[0];
    wideLayers[1]=layers[1];
    atomic_store_explicit(&wideReady,2,memory_order_release);
    return 1;
}
int nfs_wide_menu_layers(unsigned ratio,NfsWideMenuLayers *out){
    if(!out || ratio>=2 || atomic_load_explicit(&wideReady,memory_order_acquire)!=2) return 0;
    *out=wideLayers[ratio];
    return 1;
}
int nfs_hd_art_provider_available(void){return ready;}
int nfs_hd_install_art_provider(const NfsHdArtProvider*p){
 if(atomic_load_explicit(&sealed,memory_order_relaxed)||ready||!p||p->schema!=NFS_HD_ART_PROVIDER_SCHEMA||
    !p->texture||!p->word||!p->logo||!p->logoRegion)return 0;
 installed=*p;ready=1;return 1;
}
int nfs_private_background_scene_texture(uint32_t role,NfsBackgroundSceneTexture*out){
 NfsBackgroundSceneTexture value={0};atomic_store_explicit(&sealed,1,memory_order_relaxed);
 if(!ready||!out||role>=NFS_BACKGROUND_SCENE_TEXTURE_COUNT||
    !installed.texture(installed.user,role,&value))return 0;
 *out=value;return 1;
}
int nfs_private_background_scene_word(uint32_t role,NfsBackgroundSceneWord*out){
 NfsBackgroundSceneWord value={0};atomic_store_explicit(&sealed,1,memory_order_relaxed);
 if(!ready||!out||!role||role>NFS_BACKGROUND_SCENE_WORD_COUNT||
    !installed.word(installed.user,role,&value))return 0;
 *out=value;return 1;
}
int nfs_private_small_logo_art(NfsSmallLogoArt*out){
 NfsSmallLogoArt value={0};atomic_store_explicit(&sealed,1,memory_order_relaxed);
 if(!ready||!out||!installed.logo(installed.user,&value))return 0;
 *out=value;return 1;
}
int nfs_private_small_logo_region(uint32_t role,NfsSmallLogoRegion*out){
 NfsSmallLogoRegion value={0};atomic_store_explicit(&sealed,1,memory_order_relaxed);
 if(!ready||!out||role>=NFS_SMALL_LOGO_ROLE_COUNT||
    !installed.logoRegion(installed.user,role,&value))return 0;
 *out=value;return 1;
}
