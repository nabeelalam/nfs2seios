// SPDX-License-Identifier: MIT
#ifndef NFS_MENU_SOURCE_COMMAND_MODEL_H
#define NFS_MENU_SOURCE_COMMAND_MODEL_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "BackgroundSceneLayersAbi.h"
/* Semantic backdrop source receipts (kind1 map / kind3 complete Main) only. Never framebuffer pixels or a GL texture lease.
   Renderer-owner-only store; all functions are bounded and allocation-free.
   Root native-paint observer supplies current keys and invalidates on unknown
   writes, clears, context loss, resize or source-domain retirement. */
typedef struct NfsMenuSourcePaintKey {
 uintptr_t context;uint64_t generation,paintRevision;
 uint32_t nativeFbo,nativeColor,nativeDepth,width,height;
} NfsMenuSourcePaintKey;
typedef struct NfsMenuMapSource {
 NfsMenuSourcePaintKey key;uintptr_t sourceGame,sourceCatalog;uint32_t sourceArchive;
 uint64_t sourceLife,sourceSerial,sourceContent,uploads,clearEvent;
 uint32_t sourceKind,mapRole,drawKind;NfsBackgroundSceneLayers layers;int32_t minimum,maximum,direction,phase,originX,originY,stepX;
 unsigned char originalSha256[32];
} NfsMenuMapSource;
typedef struct NfsMenuMapNativeWitness {uint64_t serial,uploads,clearEvent;uint32_t selections,triangles;}NfsMenuMapNativeWitness;
typedef struct NfsMenuMapSourceStore {NfsMenuMapSource pending,completed[2];unsigned pendingValid,completedValid[2];}NfsMenuMapSourceStore;
static inline int nfs_menu_source_key_valid(const NfsMenuSourcePaintKey*k){return k&&k->context&&k->generation&&k->paintRevision&&k->nativeFbo&&k->nativeColor&&k->nativeDepth&&k->width&&k->height&&k->width<=16384&&k->height<=16384;}
static inline int nfs_menu_source_same_destination(const NfsMenuSourcePaintKey*a,const NfsMenuSourcePaintKey*b){return a&&b&&a->context==b->context&&a->generation==b->generation&&a->nativeFbo==b->nativeFbo&&a->nativeColor==b->nativeColor&&a->nativeDepth==b->nativeDepth&&a->width==b->width&&a->height==b->height;}
static inline int nfs_menu_source_same_paint(const NfsMenuSourcePaintKey*a,const NfsMenuSourcePaintKey*b){return nfs_menu_source_same_destination(a,b)&&a->paintRevision==b->paintRevision;}
/* Structural limits only; exact immutable catalogue validation is mandatory
   again in the renderer before capturing or reconstructing these commands. */
static inline int nfs_menu_backdrop_zero(const void*p,size_t bytes){const unsigned char*b=(const unsigned char*)p;for(size_t i=0;i<bytes;++i)if(b[i])return 0;return 1;}
static inline int nfs_menu_backdrop_clip(const int32_t*c){return c&&c[0]>=0&&c[1]>=0&&c[2]<=640&&c[3]<=480&&c[0]<=c[2]&&c[1]<=c[3];}
static inline int nfs_menu_backdrop_layers_valid(const NfsBackgroundSceneLayers*v){
 if(!v||v->mapRole>1||v->wordCount>4||!nfs_menu_backdrop_clip(v->panelClip)||v->logos[0].role<1||v->logos[0].role>4||v->logos[1].role!=v->logos[0].role%4+1||v->logos[2].role||v->logos[0].scalar256!=256||v->logos[2].scalar256!=256)return 0;
 {unsigned known=0;for(unsigned i=0;i<80;++i)if(v->logos[1].scalar256==i*255u/80u)known=1;if(!known)return 0;}
 for(unsigned i=0;i<4;++i){const NfsBackgroundSceneWordLayer*w=v->words+i;if(i>=v->wordCount){if(!nfs_menu_backdrop_zero(w,sizeof*w))return 0;continue;}if(w->sourceRole<1||w->sourceRole>12||w->scalar256>256||w->left< -32768||w->left>32767||w->top< -32768||w->top>32767||!nfs_menu_backdrop_clip(w->clip))return 0;}
 for(unsigned i=0;i<3;++i){const NfsSmallLogoLayer*l=v->logos+i;if(!isfinite(l->left)||!isfinite(l->top)||!isfinite(l->right)||!isfinite(l->bottom)||l->left< -32768||l->left>32767||l->top< -32768||l->top>32767||l->right<=l->left||l->bottom<=l->top||l->right-l->left>640||l->bottom-l->top>480)return 0;for(unsigned j=0;j<4;++j)if(!isfinite(l->clip[j]))return 0;if(l->clip[0]<0||l->clip[1]<0||l->clip[2]>640||l->clip[3]>480||l->clip[0]>l->clip[2]||l->clip[1]>l->clip[3])return 0;}
 return 1;
}
static inline int nfs_menu_map_source_valid(const NfsMenuMapSource*s){unsigned nonzero=0;if(!s||(s->sourceKind!=1&&s->sourceKind!=3)||(s->sourceKind==1?!nfs_menu_backdrop_zero(&s->layers,sizeof s->layers):(s->layers.mapRole!=s->mapRole||!nfs_menu_backdrop_layers_valid(&s->layers)))||!nfs_menu_source_key_valid(&s->key)||!s->sourceGame||!s->sourceCatalog||!s->sourceArchive||!s->sourceLife||!s->sourceSerial||!s->sourceContent||!s->clearEvent||s->mapRole>1||(s->drawKind!=1&&s->drawKind!=2)||s->minimum<0||s->maximum>255||s->minimum>s->maximum||s->direction<0||s->direction>1||s->phase< -1||s->phase>255||s->originX< -32768||s->originX>32767||s->originY< -32768||s->originY>32767||s->stepX<0||s->stepX>32)return 0;for(unsigned i=0;i<32;++i)nonzero|=s->originalSha256[i];return nonzero!=0;}
static inline void nfs_menu_map_source_cancel_pending(NfsMenuMapSourceStore*s){if(s){s->pendingValid=0;memset(&s->pending,0,sizeof s->pending);}}
static inline void nfs_menu_map_source_invalidate(NfsMenuMapSourceStore*s){if(s)memset(s,0,sizeof*s);}
/* Starting another request does NOT mutate the existing destination. */
static inline int nfs_menu_map_source_begin(NfsMenuMapSourceStore*s,const NfsMenuMapSource*source){if(!s)return 0;nfs_menu_map_source_cancel_pending(s);if(!nfs_menu_map_source_valid(source))return 0;s->pending=*source;s->pendingValid=1;return 1;}
/* Call only after the existing native source/target/blend/geometry admission
   and actual300/600 finish. Completion advances the paint occurrence once;
   matching depth/FBO names alone never create a receipt. */
static inline int nfs_menu_map_source_finish(NfsMenuMapSourceStore*s,const NfsMenuSourcePaintKey*completed,const NfsMenuMapNativeWitness*w){unsigned slot=2;if(!s||!s->pendingValid)return 0;if(!w||!nfs_menu_source_key_valid(completed)||!nfs_menu_source_same_destination(&s->pending.key,completed)||completed->paintRevision<=s->pending.key.paintRevision||w->serial!=s->pending.sourceSerial||w->uploads!=s->pending.uploads||w->clearEvent!=s->pending.clearEvent||w->selections!=300||w->triangles!=600){nfs_menu_map_source_cancel_pending(s);return 0;}for(unsigned i=0;i<2;++i)if(s->completedValid[i]&&s->completed[i].key.nativeFbo==completed->nativeFbo){slot=i;break;}if(slot==2)for(unsigned i=0;i<2;++i)if(!s->completedValid[i]){slot=i;break;}if(slot==2)slot=s->completed[0].key.paintRevision<s->completed[1].key.paintRevision?0u:1u;s->completed[slot]=s->pending;s->completed[slot].key=*completed;s->completedValid[slot]=1;nfs_menu_map_source_cancel_pending(s);return 1;}
/* Synchronous immutable semantic borrow. Consumer reconstructs from trusted
   authored map source +recorded20column motion, never a prior ready texture.
   Copying this data is NOT a current-paint capability: recheck the store/key
   before actual use; unknown-paint invalidation must also cancel that consumer. */
static inline const NfsMenuMapSource*nfs_menu_map_source_acquire(const NfsMenuMapSourceStore*s,const NfsMenuSourcePaintKey*current,uintptr_t game,uint64_t life,uint32_t archive,uintptr_t catalog,uint32_t mapRole){if(!s||!nfs_menu_source_key_valid(current)||!game||!life||!archive||!catalog||mapRole>1)return NULL;for(unsigned i=0;i<2;++i){const NfsMenuMapSource*p=s->completed+i;if(s->completedValid[i]&&nfs_menu_source_same_paint(&p->key,current)&&p->sourceGame==game&&p->sourceLife==life&&p->sourceArchive==archive&&p->sourceCatalog==catalog&&p->mapRole==mapRole)return p;}return NULL;}
/* A sidebar has no map pixels and cannot supply a map-role selector.
   Borrow the unique already witnessed map on this exact destination/domain.
   Preserve its own role; even duplicate same-role records are ambiguous. */
static inline const NfsMenuMapSource*nfs_menu_map_source_acquire_destination(const NfsMenuMapSourceStore*s,const NfsMenuSourcePaintKey*current,uintptr_t game,uint64_t life,uint32_t archive,uintptr_t catalog){
 const NfsMenuMapSource*found=NULL;
 if(!s||!nfs_menu_source_key_valid(current)||!game||!life||!archive||!catalog)return NULL;
 for(unsigned i=0;i<2;++i){const NfsMenuMapSource*p=s->completed+i;
  if(!s->completedValid[i]||!nfs_menu_map_source_valid(p)||!nfs_menu_source_same_paint(&p->key,current)||p->sourceGame!=game||p->sourceLife!=life||p->sourceArchive!=archive||p->sourceCatalog!=catalog)continue;
  if(found)return NULL;
  found=p;
 }
 return found;
}
#endif
