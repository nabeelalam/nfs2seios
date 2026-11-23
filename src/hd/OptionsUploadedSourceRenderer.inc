// SPDX-License-Identifier: MIT
#include <errno.h>
#include "OptionsUploadedSourceAbi.h"
#include "MainUploadedNativeProof.h"
static struct {NfsOptionsUploadedView view;NfsMainUploadedNativeTiles tiles;NfsMainUploadedSeal seal;uint64_t serial;unsigned valid,pending,normal;}nfs_options_upload;
static uint64_t nfs_options_upload_next=1;
/* Scalar, default-off diagnostics; each reason has an independent small cap. */
static void nfs_options_uploaded_trace(unsigned reason,const NfsOptionsUploadedView*v,uint64_t serial,uint64_t detail){
 const int saved=errno;static unsigned lines[16];const char*p=getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");
 if(reason<16&&p&&p[0]=='1'&&!p[1]&&lines[reason]<8){++lines[reason];fprintf(stderr,"OPTIONS UPLOADED GPU reason=%u normal=%u serial=%llu epoch=%llu upload=%llu token=%llu context=%llu valid=%u pending=%u detail=%llu\n",reason,(unsigned)(v?0:nfs_options_upload.normal),(unsigned long long)serial,(unsigned long long)(v?v->domain.ownerEpoch:nfs_options_upload.view.domain.ownerEpoch),(unsigned long long)(v?v->domain.uploadEvent:nfs_options_upload.view.domain.uploadEvent),(unsigned long long)nfs_options_upload.seal.token,(unsigned long long)nfs_options_upload.seal.context,nfs_options_upload.valid,nfs_options_upload.pending,(unsigned long long)detail);}errno=saved;
}

static void nfs_options_uploaded_invalidate(void){const int saved=errno;void*p=nfs_options_upload.view.owner;void(*release)(void*)=nfs_options_upload.view.release;void(*status)(void*,unsigned)=nfs_options_upload.view.status;memset(&nfs_options_upload,0,sizeof nfs_options_upload);if(p&&status)status(p,3);if(p&&release)release(p);errno=saved;}
static int nfs_options_uploaded_host_current(void){return nfs_options_upload.view.owner&&nfs_options_upload.view.current&&nfs_options_upload.view.current(nfs_options_upload.view.owner)&&nfs_options_upload.tiles.thread==(uintptr_t)SDL_GetCurrentThreadID()&&nfs_options_upload.tiles.context==nfs_logo_provenance.context&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&g_glCtx&&SDL_GL_GetCurrentContext()==g_glCtx;}
static void nfs_options_uploaded_retired(void){nfs_options_uploaded_trace(1,NULL,nfs_options_upload.serial,0);nfs_options_uploaded_invalidate();}
/* Inactive race path is one flag branch. A broad Main call scope can survive
 * until the first overlapping upload; that revokes once, never scans in race. */
static void nfs_options_uploaded_upload(TextureInfo*ti){
 if(!nfs_options_upload.valid&&!nfs_options_upload.pending)return;
 if(!nfs_options_uploaded_host_current()){nfs_options_uploaded_trace(2,NULL,nfs_options_upload.serial,0);nfs_options_uploaded_invalidate();return;}
 if(!ti||!ti->data||(uintptr_t)ti->data<(uintptr_t)g_textureMem||(uintptr_t)ti->data-(uintptr_t)g_textureMem>TextureMem||!ti->size||ti->size>4096){nfs_options_uploaded_trace(3,NULL,nfs_options_upload.serial,0);nfs_options_uploaded_invalidate();return;}
 const uint64_t bytes=(uint64_t)ti->size*ti->size*(ti->fmt==GR_TEXFMT_P_8?1u:2u);const uint32_t address=(uint32_t)((uintptr_t)ti->data-(uintptr_t)g_textureMem);
 if(nfs_main_uploaded_overlap(&nfs_options_upload.tiles,address,bytes)){nfs_options_uploaded_trace(4,NULL,nfs_options_upload.serial,address);nfs_options_uploaded_invalidate();}
}
static int nfs_options_uploaded_same_frame(const NfsBackgroundSceneFrameView*a,const NfsBackgroundSceneFrameView*b){return a->schema==b->schema&&a->width==b->width&&a->height==b->height&&a->planeWords==b->planeWords&&a->original565==b->original565&&a->contentRevision==b->contentRevision&&a->mapRole==b->mapRole&&a->wordCount==b->wordCount&&a->tailKind==b->tailKind&&!memcmp(a->rendererAddress,b->rendererAddress,sizeof a->rendererAddress)&&!memcmp(a->panelClip,b->panelClip,sizeof a->panelClip)&&!memcmp(a->words,b->words,sizeof a->words)&&!memcmp(a->logos,b->logos,sizeof a->logos);}
static int nfs_options_uploaded_tiles_current(void){
 if(!nfs_options_uploaded_host_current())return 0;
 for(unsigned i=0;i<300;++i){const unsigned slot=nfs_options_upload.tiles.slots[i];if(slot>=1024)return 0;const NfsLogoUploadWitness*w=&nfs_logo_uploads[slot];const uint32_t a=nfs_options_upload.tiles.addresses[i];if(w->format!=GR_TEXFMT_ARGB_1555||!nfs_main_uploaded_witness_matches(&nfs_options_upload.tiles,i,w->context,w->revision,w->address,w->id,w->valid)||a>TextureMem-2048)return 0;const TextureInfo*ti=&g_textures[a>>2];if(ti->id!=w->id||ti->data!=g_textureMem+a||ti->size!=32||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||memcmp(w->bytes,ti->data,2048))return 0;}
 return nfs_options_uploaded_host_current();
}
static int nfs_options_uploaded_capture(const NfsOptionsUploadedView*v){
 memset(&nfs_options_upload.tiles,0,sizeof nfs_options_upload.tiles);nfs_options_upload.tiles.context=nfs_logo_provenance.context;nfs_options_upload.tiles.thread=(uintptr_t)SDL_GetCurrentThreadID();nfs_options_upload.tiles.low=TextureMem;
 for(unsigned i=0;i<300;++i){const uint32_t a=v->frame.scene.rendererAddress[i];if((a&3)||a>TextureMem-2048)return 0;const TextureInfo*ti=&g_textures[a>>2];unsigned found=1024;
  for(unsigned j=0;j<1024;++j){const NfsLogoUploadWitness*w=&nfs_logo_uploads[j];if(w->valid&&w->format==GR_TEXFMT_ARGB_1555&&w->context==nfs_logo_provenance.context&&w->address==a&&w->id==ti->id&&w->revision&&w->revision<=nfs_logo_provenance.uploads&&!memcmp(w->bytes,ti->data,2048)){if(found!=1024)return 0;found=j;}}
  if(found==1024)return 0;const NfsLogoUploadWitness*w=&nfs_logo_uploads[found];nfs_options_upload.tiles.addresses[i]=a;nfs_options_upload.tiles.ids[i]=w->id;nfs_options_upload.tiles.slots[i]=found;nfs_options_upload.tiles.revisions[i]=w->revision;if(a<nfs_options_upload.tiles.low)nfs_options_upload.tiles.low=a;if(a+2048>nfs_options_upload.tiles.high)nfs_options_upload.tiles.high=a+2048;
 }
 return 1;
}
int nfs_private_options_uploaded_begin(const NfsOptionsUploadedView*v,const NfsMainUploadedSeal*expected){
 if(!v||v->schema!=1||!v->owner||!v->retain||!v->release||!v->current||!v->status||!v->current(v->owner)||v->domain.form!=0x4d3a88u||!v->domain.game||!v->domain.life||!v->domain.catalog||!v->domain.archive||!v->domain.ownerEpoch||!v->domain.uploadEvent){nfs_options_uploaded_trace(5,v,v?v->frame.scene.serial:0,expected!=NULL);return 0;}
 if(expected){
  nfs_options_uploaded_trace(6,v,v->frame.scene.serial,expected->token);
  if(!nfs_options_upload.valid||nfs_options_upload.pending||expected->token!=nfs_options_upload.seal.token||expected->context!=nfs_options_upload.seal.context||v->owner!=nfs_options_upload.view.owner||memcmp(&v->domain,&nfs_options_upload.view.domain,sizeof v->domain)||!nfs_options_uploaded_same_frame(&v->frame.scene,&nfs_options_upload.view.frame.scene)||!nfs_options_uploaded_tiles_current()){nfs_options_uploaded_trace(7,v,v->frame.scene.serial,expected->token);nfs_options_uploaded_invalidate();return 0;}
 }else{
  nfs_options_uploaded_trace(8,v,v->frame.scene.serial,0);
  nfs_options_uploaded_invalidate();if(!nfs_options_upload_next||nfs_options_upload_next==UINT64_MAX){nfs_options_uploaded_trace(9,v,v->frame.scene.serial,0);return 0;}
  nfs_options_upload.view=*v;v->retain(v->owner);v->status(v->owner,1);nfs_options_upload.seal.token=nfs_options_upload_next++;nfs_options_upload.seal.context=nfs_logo_provenance.context;
 }
 if(!nfs_private_options_scene_begin(&v->frame)){nfs_options_uploaded_trace(10,v,v->frame.scene.serial,0);nfs_options_uploaded_invalidate();return 0;}
 if(!expected&&!nfs_options_uploaded_capture(v)){nfs_options_uploaded_trace(11,v,v->frame.scene.serial,0);nfs_options_uploaded_invalidate();return 0;}
 if(!nfs_options_uploaded_tiles_current()){nfs_options_uploaded_trace(12,v,v->frame.scene.serial,0);nfs_options_uploaded_invalidate();return 0;}
 nfs_options_upload.serial=v->frame.scene.serial;nfs_options_upload.pending=1;nfs_options_upload.normal=expected==NULL;return 1;
}
int nfs_private_options_uploaded_finish(uint64_t serial,NfsMainUploadedSeal*out){
 if(out)memset(out,0,sizeof*out);
 if(!nfs_options_upload.pending||serial!=nfs_options_upload.serial||!nfs_options_uploaded_tiles_current()){nfs_options_uploaded_trace(13,NULL,serial,0);nfs_options_uploaded_invalidate();return 0;}
 if(!nfs_private_options_scene_finish(serial)){nfs_options_uploaded_trace(14,NULL,serial,1);nfs_options_uploaded_invalidate();return 0;}
 if(!nfs_options_uploaded_tiles_current()){nfs_options_uploaded_trace(14,NULL,serial,2);nfs_options_uploaded_invalidate();return 0;}
 nfs_options_upload.pending=0;nfs_options_upload.valid=1;nfs_options_upload.view.status(nfs_options_upload.view.owner,2);nfs_options_uploaded_trace(15,NULL,serial,2);if(out)*out=nfs_options_upload.seal;return 1;
}
