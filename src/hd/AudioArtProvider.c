// SPDX-License-Identifier: MIT
#include "AudioArtProvider.h"
#include <stdatomic.h>
static NfsAudioArtProvider installed;
static atomic_int sealed;
static int ready;
int nfs_hd_audio_art_provider_available(void){return ready;}
int nfs_hd_install_audio_art_provider(const NfsAudioArtProvider*p){
 if(atomic_load_explicit(&sealed,memory_order_relaxed)||ready||!p||
    p->schema!=NFS_AUDIO_ART_PROVIDER_SCHEMA||!p->atlas)return 0;
 installed=*p;ready=1;return 1;
}
int nfs_hd_audio_art(NfsAudioArt*out){
 NfsAudioArt value={0};unsigned nonzero=0;
 atomic_store_explicit(&sealed,1,memory_order_relaxed);
 if(!ready||!out||!installed.atlas(installed.user,&value))return 0;
 if(!value.pixels||value.bytes!=NFS_AUDIO_ART_BYTES||
    value.width!=NFS_AUDIO_ART_WIDTH||value.height!=NFS_AUDIO_ART_HEIGHT||
    value.schema!=NFS_AUDIO_ART_SCHEMA||value.premultiplied!=1||
    value.opacityBaked!=1||!value.identity)return 0;
 for(unsigned k=0;k<32;++k)nonzero|=value.sha256[k];
 if(!nonzero)return 0;
 *out=value;return 1;
}
