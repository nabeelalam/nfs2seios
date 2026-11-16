// SPDX-License-Identifier: MIT
#include <errno.h>
#include "MainUploadedSourceAbi.h"
#include "MainUploadedNativeProof.h"
static struct {NfsMainUploadedView view;NfsMainUploadedNativeTiles tiles;NfsMainUploadedSeal seal;uint64_t serial;unsigned valid,pending,normal;}nfs_main_upload;
static uint64_t nfs_main_upload_next=1;
static void nfs_main_uploaded_invalidate(void){const int saved=errno;void*p=nfs_main_upload.view.owner;void(*release)(void*)=nfs_main_upload.view.release;void(*status)(void*,unsigned)=nfs_main_upload.view.status;memset(&nfs_main_upload,0,sizeof nfs_main_upload);if(p&&status)status(p,3);if(p&&release)release(p);errno=saved;}
static int nfs_main_uploaded_host_current(void){return nfs_main_upload.view.owner&&nfs_main_upload.view.current&&nfs_main_upload.view.current(nfs_main_upload.view.owner)&&nfs_main_upload.tiles.thread==(uintptr_t)SDL_GetCurrentThreadID()&&nfs_main_upload.tiles.context==nfs_logo_provenance.context&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&g_glCtx&&SDL_GL_GetCurrentContext()==g_glCtx;}
static void nfs_main_uploaded_retired(void){nfs_main_uploaded_invalidate();}
/* Inactive race path is one flag branch. A broad Main call scope can survive
 * until the first overlapping upload; that revokes once, never scans in race. */
static void nfs_main_uploaded_upload(TextureInfo*ti){
 if(!nfs_main_upload.valid&&!nfs_main_upload.pending)return;
 if(!nfs_main_uploaded_host_current()){nfs_main_uploaded_invalidate();return;}
 if(!ti||!ti->data||(uintptr_t)ti->data<(uintptr_t)g_textureMem||(uintptr_t)ti->data-(uintptr_t)g_textureMem>TextureMem||!ti->size||ti->size>4096){nfs_main_uploaded_invalidate();return;}
 const uint64_t bytes=(uint64_t)ti->size*ti->size*(ti->fmt==GR_TEXFMT_P_8?1u:2u);const uint32_t address=(uint32_t)((uintptr_t)ti->data-(uintptr_t)g_textureMem);
 if(nfs_main_uploaded_overlap(&nfs_main_upload.tiles,address,bytes))nfs_main_uploaded_invalidate();
}
static int nfs_main_uploaded_same_frame(const NfsBackgroundSceneFrameView*a,const NfsBackgroundSceneFrameView*b){return a->schema==b->schema&&a->width==b->width&&a->height==b->height&&a->planeWords==b->planeWords&&a->original565==b->original565&&a->contentRevision==b->contentRevision&&a->mapRole==b->mapRole&&a->wordCount==b->wordCount&&a->tailKind==b->tailKind&&!memcmp(a->rendererAddress,b->rendererAddress,sizeof a->rendererAddress)&&!memcmp(a->panelClip,b->panelClip,sizeof a->panelClip)&&!memcmp(a->words,b->words,sizeof a->words)&&!memcmp(a->logos,b->logos,sizeof a->logos);}
static int nfs_main_uploaded_tiles_current(void){
 if(!nfs_main_uploaded_host_current())return 0;
 for(unsigned i=0;i<300;++i){const unsigned slot=nfs_main_upload.tiles.slots[i];if(slot>=1024)return 0;const NfsLogoUploadWitness*w=&nfs_logo_uploads[slot];const uint32_t a=nfs_main_upload.tiles.addresses[i];if(w->format!=GR_TEXFMT_ARGB_1555||!nfs_main_uploaded_witness_matches(&nfs_main_upload.tiles,i,w->context,w->revision,w->address,w->id,w->valid)||a>TextureMem-2048)return 0;const TextureInfo*ti=&g_textures[a>>2];if(ti->id!=w->id||ti->data!=g_textureMem+a||ti->size!=32||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||memcmp(w->bytes,ti->data,2048))return 0;}
 return nfs_main_uploaded_host_current();
}
static int nfs_main_uploaded_capture(const NfsMainUploadedView*v){
 memset(&nfs_main_upload.tiles,0,sizeof nfs_main_upload.tiles);nfs_main_upload.tiles.context=nfs_logo_provenance.context;nfs_main_upload.tiles.thread=(uintptr_t)SDL_GetCurrentThreadID();nfs_main_upload.tiles.low=TextureMem;
 for(unsigned i=0;i<300;++i){const uint32_t a=v->frame.rendererAddress[i];if((a&3)||a>TextureMem-2048)return 0;const TextureInfo*ti=&g_textures[a>>2];unsigned found=1024;
  for(unsigned j=0;j<1024;++j){const NfsLogoUploadWitness*w=&nfs_logo_uploads[j];if(w->valid&&w->format==GR_TEXFMT_ARGB_1555&&w->context==nfs_logo_provenance.context&&w->address==a&&w->id==ti->id&&w->revision&&w->revision<=nfs_logo_provenance.uploads&&!memcmp(w->bytes,ti->data,2048)){if(found!=1024)return 0;found=j;}}
  if(found==1024)return 0;const NfsLogoUploadWitness*w=&nfs_logo_uploads[found];nfs_main_upload.tiles.addresses[i]=a;nfs_main_upload.tiles.ids[i]=w->id;nfs_main_upload.tiles.slots[i]=found;nfs_main_upload.tiles.revisions[i]=w->revision;if(a<nfs_main_upload.tiles.low)nfs_main_upload.tiles.low=a;if(a+2048>nfs_main_upload.tiles.high)nfs_main_upload.tiles.high=a+2048;
 }
 return 1;
}
int nfs_private_main_uploaded_begin(const NfsMainUploadedView*v,const NfsMainUploadedSeal*expected){
 if(!v||v->schema!=1||!v->owner||!v->retain||!v->release||!v->current||!v->status||!v->current(v->owner)||v->domain.form!=0x401910u||!v->domain.game||!v->domain.life||!v->domain.catalog||!v->domain.archive||!v->domain.ownerEpoch||!v->domain.uploadEvent)return 0;
 if(expected){
  if(!nfs_main_upload.valid||nfs_main_upload.pending||expected->token!=nfs_main_upload.seal.token||expected->context!=nfs_main_upload.seal.context||v->owner!=nfs_main_upload.view.owner||memcmp(&v->domain,&nfs_main_upload.view.domain,sizeof v->domain)||!nfs_main_uploaded_same_frame(&v->frame,&nfs_main_upload.view.frame)||!nfs_main_uploaded_tiles_current()){nfs_main_uploaded_invalidate();return 0;}
 }else{
  nfs_main_uploaded_invalidate();if(!nfs_main_upload_next||nfs_main_upload_next==UINT64_MAX)return 0;
  nfs_main_upload.view=*v;v->retain(v->owner);v->status(v->owner,1);nfs_main_upload.seal.token=nfs_main_upload_next++;nfs_main_upload.seal.context=nfs_logo_provenance.context;
 }
 if(!nfs_private_background_scene_begin(&v->frame)){nfs_main_uploaded_invalidate();return 0;}
 if(!expected&&!nfs_main_uploaded_capture(v)){nfs_main_uploaded_invalidate();return 0;}
 if(!nfs_main_uploaded_tiles_current()){nfs_main_uploaded_invalidate();return 0;}
 nfs_main_upload.serial=v->frame.serial;nfs_main_upload.pending=1;nfs_main_upload.normal=expected==NULL;return 1;
}
int nfs_private_main_uploaded_finish(uint64_t serial,NfsMainUploadedSeal*out){
 if(out)memset(out,0,sizeof*out);
 if(!nfs_main_upload.pending||serial!=nfs_main_upload.serial||!nfs_main_uploaded_tiles_current()){nfs_main_uploaded_invalidate();return 0;}
 if(!nfs_private_background_scene_finish(serial)||!nfs_main_uploaded_tiles_current()){nfs_main_uploaded_invalidate();return 0;}
 nfs_main_upload.pending=0;nfs_main_upload.valid=1;nfs_main_upload.view.status(nfs_main_upload.view.owner,2);if(out)*out=nfs_main_upload.seal;return 1;
}
