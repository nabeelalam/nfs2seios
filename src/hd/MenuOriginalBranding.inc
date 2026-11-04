// SPDX-License-Identifier: MIT
#include "MainOriginalBrandingSource.h"
/* One immutable uploaded A5/B5/C5 witness; unrelated textures cannot evict it into
   acceptance. Rehash only on actual uploads, never on every tail/selection. */
static struct {
 uintptr_t context;uint64_t generation,revision;
 uint32_t address,id;unsigned valid;
} nfs_menu_original;
static struct {
 uintptr_t context;uint64_t generation,revision;
 uint32_t address,id,format,edge;unsigned valid;
} nfs_menu_original_selection;
static uint64_t nfs_menu_original_next_revision;
static int nfs_menu_original_owner(void){return g_glCtx&&SDL_GL_GetCurrentContext()==g_glCtx&&SDL_IsMainThread()&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled;}
static void nfs_menu_original_forget(void){memset(&nfs_menu_original,0,sizeof nfs_menu_original);memset(&nfs_menu_original_selection,0,sizeof nfs_menu_original_selection);nfs_menu_original_next_revision=0;}
static void nfs_menu_original_invalidate(uint32_t address,uint64_t bytes){
 const uint64_t end=bytes>UINT64_MAX-address?UINT64_MAX:(uint64_t)address+bytes;
 if((uint64_t)address<(uint64_t)nfs_menu_original.address+NFS_MAIN_ORIGINAL_A5_BYTES&&(uint64_t)nfs_menu_original.address<end){nfs_menu_original.valid=0;nfs_menu_original_selection.valid=0;}
}
static void nfs_menu_original_upload_begin(TextureInfo*ti){
 if(!ti||(uintptr_t)ti<(uintptr_t)g_textures||(uintptr_t)ti>=(uintptr_t)(g_textures+TextureMem/4))return;
 const uint32_t address=(uint32_t)(ti-g_textures)*4;
 nfs_menu_original_invalidate(address,(uint64_t)ti->size*ti->size*(ti->fmt==GR_TEXFMT_P_8?1u:2u));
}
static void nfs_menu_original_upload_done(TextureInfo*ti){
 unsigned char digest[32];GLboolean live=0;
 if(!nfs_menu_original_owner()||!ti||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->size!=NFS_MAIN_ORIGINAL_A5_EDGE||ti->hdDigits||!ti->id||(uintptr_t)ti<(uintptr_t)g_textures||(uintptr_t)ti>=(uintptr_t)(g_textures+TextureMem/4))return;
 const uint32_t address=(uint32_t)(ti-g_textures)*4;
 if(address>TextureMem-NFS_MAIN_ORIGINAL_A5_BYTES||ti->data!=g_textureMem+address)return;
 if(!nfs_texture_sha256(ti->data,NFS_MAIN_ORIGINAL_A5_BYTES,digest)||!nfs_main_original_tier_digest(digest))return;
 if(!nfs_menu_original_owner())return;live=glIsTexture(ti->id);if(!nfs_menu_original_owner()||!live||glGetError()!=GL_NO_ERROR)return;
 if(nfs_menu_original_next_revision==UINT64_MAX){nfs_menu_original.valid=0;return;}
 nfs_menu_original.context=(uintptr_t)g_glCtx;nfs_menu_original.generation=nfs_logo_provenance.context;nfs_menu_original.revision=++nfs_menu_original_next_revision;nfs_menu_original.address=address;nfs_menu_original.id=ti->id;nfs_menu_original.valid=1;
}
static void nfs_menu_original_select(unsigned tmu,uint32_t address,uint32_t evenOdd,const GrTexInfo*info,TextureInfo*ti){
 (void)evenOdd;nfs_menu_original_selection.valid=0;
 nfs_menu_original_selection.address=address;nfs_menu_original_selection.id=ti?ti->id:0;nfs_menu_original_selection.format=ti?ti->fmt:0;nfs_menu_original_selection.edge=ti?ti->size:0;
 if(!nfs_menu_original_owner()||tmu!=0||!info||!ti||(address&3)||address>TextureMem-NFS_MAIN_ORIGINAL_A5_BYTES||ti!=g_textures+(address>>2)||ti->data!=g_textureMem+address||ti->fmt!=GR_TEXFMT_ARGB_1555||info->format!=ti->fmt||ti->size!=128||ti->hdDigits||info->largeLod!=GR_LOD_128||info->smallLod!=GR_LOD_128||info->aspectRatio!=GR_ASPECT_1x1)return;
 if(!nfs_menu_original.valid||nfs_menu_original.context!=(uintptr_t)g_glCtx||nfs_menu_original.generation!=nfs_logo_provenance.context||nfs_menu_original.address!=address||nfs_menu_original.id!=ti->id)return;
 nfs_menu_original_selection.context=nfs_menu_original.context;nfs_menu_original_selection.generation=nfs_menu_original.generation;nfs_menu_original_selection.revision=nfs_menu_original.revision;nfs_menu_original_selection.valid=1;
}
static int nfs_menu_original_branding(uint64_t epoch){
 NfsMenuResponsiveDebugState source;GLint active=0,binding=0,location=-1,sampler=-1;
 if(!nfs_menu_original_owner()||!nfs_menu_original.valid||!nfs_menu_original_selection.valid||nfs_menu_original.context!=(uintptr_t)g_glCtx||nfs_menu_original.generation!=nfs_logo_provenance.context||nfs_menu_original_selection.context!=nfs_menu_original.context||nfs_menu_original_selection.generation!=nfs_menu_original.generation||nfs_menu_original_selection.revision!=nfs_menu_original.revision||nfs_menu_original_selection.address!=nfs_menu_original.address||nfs_menu_original_selection.id!=nfs_menu_original.id)return 0;
 memset(&source,0,sizeof source);nfs_menu_responsive_debug_state(&source);
 if(source.schema!=3||source.sourceForm!=nfs_menu_wide.ownerForm||!source.cursorValid||source.sourceEpoch!=epoch||source.originalOpponentValid!=1||source.originalOpponentAddress!=nfs_menu_original.address)return 0;
 TextureInfo*ti=g_textures+(nfs_menu_original.address>>2);
 if(ti->id!=nfs_menu_original.id||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->size!=128||ti->hdDigits||ti->data!=g_textureMem+nfs_menu_original.address)return 0;
 glGetIntegerv(GL_ACTIVE_TEXTURE,&active);if(!nfs_menu_original_owner()||active!=GL_TEXTURE0)return 0;
 glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);if(!nfs_menu_original_owner()||(GLuint)binding!=nfs_menu_original.id)return 0;
#ifdef GLES2
 location=glGetUniformLocation(g_shaderProgram,"uTextureSampler");if(!nfs_menu_original_owner()||location<0)return 0;
 glGetUniformiv(g_shaderProgram,location,&sampler);if(!nfs_menu_original_owner()||sampler!=0)return 0;
#else
 /* Existing classic menu deliverables use GLES2; no unverified desktop sampler. */
 return 0;
#endif
 return glGetError()==GL_NO_ERROR;
}
