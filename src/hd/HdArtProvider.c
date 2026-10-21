// SPDX-License-Identifier: MIT
#include "HdArtProvider.h"
#include <stdatomic.h>
static NfsHdArtProvider installed;
static atomic_int sealed;
static int ready;
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
