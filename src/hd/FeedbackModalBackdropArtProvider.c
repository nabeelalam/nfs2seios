// SPDX-License-Identifier: MIT
#include "FeedbackModalBackdropArtProvider.h"
#include <stdatomic.h>
static NfsFeedbackModalBackdropArtProvider installed;
static atomic_int sealed;
static int ready;
int nfs_hd_feedback_modal_backdrop_art_provider_available(void){return ready;}
int nfs_hd_install_feedback_modal_backdrop_art_provider(const NfsFeedbackModalBackdropArtProvider*p){
 if(atomic_load_explicit(&sealed,memory_order_relaxed)||ready||!p||p->schema!=NFS_FEEDBACK_MODAL_BACKDROP_ART_PROVIDER_SCHEMA||!p->view)return 0;
 installed=*p;ready=1;return 1;
}
int nfs_hd_feedback_modal_backdrop_art(NfsFeedbackModalBackdropArtView*out){
 NfsFeedbackModalBackdropArtView value={0};unsigned i,k;
 atomic_store_explicit(&sealed,1,memory_order_relaxed);
 if(!ready||!out||!installed.view(installed.user,&value)||value.schema!=NFS_FEEDBACK_MODAL_BACKDROP_ART_SCHEMA||value.count!=NFS_FEEDBACK_MODAL_BACKDROP_ART_IMAGES)return 0;
 for(i=0;i<NFS_FEEDBACK_MODAL_BACKDROP_ART_IMAGES;++i){const NfsFeedbackModalBackdropArtImage*p=&value.images[i];unsigned nonzero=0;
  const uint32_t width=i?NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM_WIDTH:NFS_FEEDBACK_MODAL_BACKDROP_BANNER_WIDTH;
  const uint32_t height=i?NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM_HEIGHT:NFS_FEEDBACK_MODAL_BACKDROP_BANNER_HEIGHT;
  const size_t bytes=i?NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM_BYTES:NFS_FEEDBACK_MODAL_BACKDROP_BANNER_BYTES;
  if(p->schema!=NFS_FEEDBACK_MODAL_BACKDROP_ART_SCHEMA||p->role!=i+1||p->width!=width||p->height!=height||p->straightRgba!=1||!p->rgba||p->bytes!=bytes)return 0;
  for(k=0;k<32;++k)nonzero|=p->sha256[k];if(!nonzero)return 0;
 }
 *out=value;return 1;
}
