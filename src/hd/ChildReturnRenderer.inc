// SPDX-License-Identifier: MIT
/* Actual Escape return only. New ReturnOwner is the source authority; the
 * shared typed-resource shape and GPU geometry grant no preview-change entry. */
#include "ChildReturnPreludeAbi.h"
#include "ChildReturnNativeGeometry.h"
static struct {NfsChildReturnPreludeView view;NfsTransitionMotion motion;unsigned active;GLuint program;GLint matrix;uintptr_t context;uint64_t generation;}nfs_cr;
static int nfs_cr_current(void){return nfs_cr.active&&(nfs_ce.channel==2||nfs_ce.channel==3)&&nfs_ce.channel==nfs_cr.view.route+1&&nfs_ce.view.serial==nfs_cr.view.semantic.serial&&nfs_ce_current()&&(nfs_cr.view.schema!=2||(nfs_cr.view.semanticSidebarCurrent&&nfs_cr.view.semanticSidebarCurrent(nfs_cr.view.semantic.owner)==1));}
static void nfs_cr_note(unsigned stage,unsigned reason){struct Key{unsigned route,stage,reason,count;};static struct Key keys[48];static unsigned used;const int saved=errno;if(!nfs_logo_trace_enabled()){errno=saved;return;}unsigned k;for(k=0;k<used;++k)if(keys[k].route==nfs_cr.view.route&&keys[k].stage==stage&&keys[k].reason==reason)break;if(k==used){if(used==48){errno=saved;return;}keys[used++]=(struct Key){nfs_cr.view.route,stage,reason,0};}if(keys[k].count++<2)fprintf(stderr,"CHILD RETURN GPU route=%u frame=%u stage=%u reason=%u phase=%u baseSelect=%u baseTri=%u sideSelect=%u sideTri=%u photoSelect=%u photoTri=%u\n",nfs_cr.view.route,nfs_cr.view.frameIndex,stage,reason,nfs_ce.phase,nfs_ce.fullPass.selects,nfs_ce.fullPass.triangles,nfs_ce.sidebarPass.selects,nfs_ce.sidebarPass.triangles,nfs_ce.photoSelects,nfs_ce.photoTriangles);errno=saved;}
#define NFS_CR_RETURN(call) do{if(!nfs_cr_current())return 0;call;if(!nfs_cr_current())return 0;}while(0)
#define NFS_CR_GL(call) do{if(!nfs_cr_current())goto done;call;if(!nfs_cr_current())goto done;}while(0)
static GLuint nfs_cr_shader(GLenum type,const char*source){GLuint sh=0;GLint okay=0;if(!nfs_cr_current())return 0;sh=glCreateShader(type);if(!nfs_cr_current())return 0;if(!sh)return 0;NFS_CR_GL(glShaderSource(sh,1,&source,NULL));NFS_CR_GL(glCompileShader(sh));NFS_CR_GL(glGetShaderiv(sh,GL_COMPILE_STATUS,&okay));if(okay)return sh;done:if(nfs_ce_context()&&sh)glDeleteShader(sh);return 0;}
static int nfs_cr_program(void){GLuint vs=0,fs=0,p=0;GLint okay=0;static const char vertex[]="attribute vec2 vertex;attribute float coefficient;uniform mat4 matrix;varying highp float gray;void main(){gl_Position=matrix*vec4(vertex,1.0,1.0);gray=coefficient;}";static const char fragment[]="precision highp float;varying highp float gray;void main(){gl_FragColor=vec4(gray,gray,gray,1.0);}";if(!nfs_cr_current())return 0;if(nfs_cr.program&&nfs_cr.context==nfs_ce.context&&nfs_cr.generation==nfs_ce.generation){GLboolean live=glIsProgram(nfs_cr.program);return nfs_cr_current()&&live;}nfs_cr.program=0;nfs_cr.context=nfs_ce.context;nfs_cr.generation=nfs_ce.generation;vs=nfs_cr_shader(GL_VERTEX_SHADER,vertex);fs=nfs_cr_shader(GL_FRAGMENT_SHADER,fragment);if(!vs||!fs)goto done;NFS_CR_GL(p=glCreateProgram());if(!p)goto done;NFS_CR_GL(glAttachShader(p,vs));NFS_CR_GL(glAttachShader(p,fs));NFS_CR_GL(glBindAttribLocation(p,0,"vertex"));NFS_CR_GL(glBindAttribLocation(p,1,"coefficient"));NFS_CR_GL(glLinkProgram(p));NFS_CR_GL(glGetProgramiv(p,GL_LINK_STATUS,&okay));if(!okay)goto done;NFS_CR_GL(nfs_cr.matrix=glGetUniformLocation(p,"matrix"));if(nfs_cr.matrix<0)goto done;nfs_cr.program=p;p=0;done:if(nfs_ce_context()){if(vs)glDeleteShader(vs);if(nfs_ce_context()&&fs)glDeleteShader(fs);if(nfs_ce_context()&&p)glDeleteProgram(p);}return nfs_cr_current()&&nfs_cr.program;}
/* Relative to the freshly reconstructed RGBA target p, this exactly preserves
 * the native gray shader's intent: RGB=g*p.rgb, A=p.a. DST_COLOR/ZERO for RGB
 * and ZERO/ONE for alpha requires no texture feedback/readback or scratch. */
static int nfs_cr_multiply(void){NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_ce_guard();NfsTransitionBand bands[20];int okay=0;if(!nfs_cr_current()||!nfs_cr_program()||!nfs_transition_bands(&nfs_cr.motion,nfs_ce.width,nfs_ce.height,bands)||!nfs_graphics_scene_save(&saved,&guard))return 0;NFS_CR_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_ce.slots[nfs_ce.index].fbo));if(!nfs_ce_attachment(nfs_ce.slots+nfs_ce.index))goto done;NFS_CR_GL(glViewport(0,0,nfs_ce.width,nfs_ce.height));for(unsigned i=0;i<8;++i)NFS_CR_GL(glDisable(nfs_frame_caps[i]));NFS_CR_GL(glColorMask(1,1,1,1));NFS_CR_GL(glDepthMask(GL_FALSE));NFS_CR_GL(glEnable(GL_BLEND));NFS_CR_GL(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));NFS_CR_GL(glBlendFuncSeparate(GL_DST_COLOR,GL_ZERO,GL_ZERO,GL_ONE));NFS_CR_GL(glUseProgram(nfs_cr.program));NFS_CR_GL(glUniformMatrix4fv(nfs_cr.matrix,1,GL_FALSE,nfs_ce.matrix));NFS_CR_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_CR_GL(glEnableVertexAttribArray(0));NFS_CR_GL(glEnableVertexAttribArray(1));NFS_CR_GL(glDisableVertexAttribArray(2));NFS_CR_GL(glDisableVertexAttribArray(3));for(unsigned i=0;i<20;++i){const NfsTransitionBand*b=bands+i;const GLfloat q[]={b->left,b->top,b->right,b->top,b->left,b->bottom,b->left,b->bottom,b->right,b->top,b->right,b->bottom};const GLfloat left=nfs_cr.view.route==1?b->grayLeft:140.f/255, right=nfs_cr.view.route==1?b->grayRight:140.f/255;const GLfloat gray[]={left,right,left,left,right,right};NFS_CR_GL(glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,q));NFS_CR_GL(glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,gray));NFS_CR_GL(glDrawArrays(GL_TRIANGLES,0,6));}okay=nfs_ce_error()==GL_NO_ERROR;done:if(nfs_ce_context()&&!nfs_graphics_scene_restore(&saved,&guard))okay=0;return okay&&nfs_cr_current();}
static int nfs_cr_bare(void){NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_ce_guard();int okay=0;GLfloat oldDepth=1;if(!nfs_cr_current()||nfs_cr.view.baseMapRole!=0||!nfs_graphics_scene_save(&saved,&guard))return 0;NFS_CR_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_ce.slots[nfs_ce.index].fbo));if(!nfs_ce_attachment(nfs_ce.slots+nfs_ce.index))goto done;for(unsigned i=0;i<8;++i)NFS_CR_GL(glDisable(nfs_frame_caps[i]));NFS_CR_GL(glViewport(0,0,nfs_ce.width,nfs_ce.height));NFS_CR_GL(glColorMask(1,1,1,1));NFS_CR_GL(glDepthMask(GL_TRUE));NFS_CR_GL(glGetFloatv(GL_DEPTH_CLEAR_VALUE,&oldDepth));
#ifdef GLES2
 NFS_CR_GL(glClearDepthf(g_depthMask?0:1));
#else
 NFS_CR_GL(glClearDepth(g_depthMask?0:1));
#endif
 NFS_CR_GL(glClear(GL_DEPTH_BUFFER_BIT));
#ifdef GLES2
 NFS_CR_GL(glClearDepthf(oldDepth));
#else
 NFS_CR_GL(glClearDepth(oldDepth));
#endif
 NFS_CR_GL(glDisable(GL_BLEND));okay=nfs_ce_quad(nfs_ce.art[0],1);done:if(nfs_ce_context()&&!nfs_graphics_scene_restore(&saved,&guard))okay=0;return okay&&nfs_cr_current();}
/* Bounded entry diagnostics distinguish source submission from GPU admission. */
static int nfs_cr_begin_refuse(const NfsChildReturnPreludeView*v,unsigned reason){const int saved=errno;static unsigned count[7];if(reason<7&&nfs_logo_trace_enabled()&&count[reason]++<2)fprintf(stderr,"CHILD RETURN GPU begin-refused route=%u frame=%u reason=%u\n",v?v->route:0,v?v->frameIndex:0,reason);errno=saved;return 0;}
/* Current semantic authority is restricted to the same complete parent scene
   already authenticated by the base image and actual native full pass. The
   cached legacy sidebar is still drawn/observed, but its obsolete RGB values
   cannot select the authored parent scene. Other families retain exact pixels. */
static int nfs_cr_semantic_sidebar(void){
 const NfsChildReturnPreludeView*v=&nfs_cr.view;
 if(v->schema!=2||v->route!=NFS_CHILD_RETURN_FADE||!v->semanticSidebarCurrent||
 !nfs_cr_current()||v->semanticSidebarCurrent(v->semantic.owner)!=1)return 0;
 const NfsBackgroundSceneFrameView*scene=nfs_ce_scene(&v->semantic);
 if(!scene||scene->planeWords!=307200||!scene->original565||v->baseWords!=307200||
 v->baseMapRole!=scene->mapRole||memcmp(v->baseKeys,scene->rendererAddress,sizeof v->baseKeys)||
 memcmp(v->baseOriginal565,scene->original565,307200u*2u))return 0;
 return (v->semantic.family==NFS_CHILD_EXIT_SETUP&&v->semantic.form==0x4d2790u&&v->semantic.setup.schema==NFS_SETUP_SCENE_RUNTIME_SCHEMA)||
 (v->semantic.family==NFS_CHILD_EXIT_OPTIONS&&v->semantic.form==0x4d3a88u&&v->semantic.options.schema==NFS_OPTIONS_SCENE_RUNTIME_SCHEMA);
}
int nfs_private_child_return_begin(const NfsChildReturnPreludeView*v){if(!v||(v->schema!=1&&v->schema!=2)||(v->schema==1&&v->semanticSidebarCurrent)||!v->baseOriginal565||v->baseWords!=307200||!v->baseContent||v->baseMapRole>1)return nfs_cr_begin_refuse(v,1);if(v->schema==2&&(!v->semantic.owner||v->route!=NFS_CHILD_RETURN_FADE||!v->semanticSidebarCurrent||v->semanticSidebarCurrent(v->semantic.owner)!=1))return nfs_cr_begin_refuse(v,3);NfsTransitionMotion m={v->minimum,v->maximum,v->direction,v->phase,v->originX,v->originY,v->stepX};if(!nfs_child_return_motion(v->route,v->frameIndex,&m,v->rgba))return nfs_cr_begin_refuse(v,2);const NfsBackgroundSceneFrameView*scene=nfs_ce_scene(&v->semantic);if(!scene||!nfs_ce_resource_shape(&v->semantic))return nfs_cr_begin_refuse(v,3);if(v->route==1&&(v->baseMapRole!=scene->mapRole||memcmp(v->baseKeys,scene->rendererAddress,sizeof v->baseKeys)||memcmp(v->baseOriginal565,scene->original565,307200u*2u)))return nfs_cr_begin_refuse(v,4);if(v->route==2&&v->baseMapRole!=0)return nfs_cr_begin_refuse(v,5);NfsChildExitPreludeView resource=v->semantic;resource.clearedFrame=1;resource.motion=v->route==2?211-15*(int)v->frameIndex:0;if(!nfs_ce_begin_borrow(&resource,v->route+1,v->route==2?100u:0u))return nfs_cr_begin_refuse(v,6);nfs_cr.view=*v;nfs_cr.motion=m;nfs_cr.active=1;nfs_cr_note(0,0);return 1;}
static int nfs_child_return_base_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){NfsAudioNativeStamp stamp;NfsPlayerIntroNativePass*p=&nfs_ce.fullPass;if(!nfs_cr_current()||nfs_ce.phase!=2||!nfs_ce_stamp(&stamp)||!nfs_player_intro_pass_current(p,&stamp,nfs_logo_provenance.uploads)||p->triangles>=600||p->selects!=p->triangles/2+1||!g_textureEnabled||g_fogEnabled)return 0;const GrVertex*in[3]={a,b,c};unsigned tile=p->triangles/2,half=p->triangles&1;const unsigned corners[3]={0,half?2u:1u,half?3u:2u};for(unsigned i=0;i<3;++i){const GrVertex*v=in[i];if(!v)return 0;NfsAudioNativeVertex n={v->x,v->y,v->r,v->g,v->b,v->a,v->oow,v->tmuvtx[0].sow,v->tmuvtx[0].tow};if(!nfs_child_return_native_corner(&n,tile,corners[i],nfs_cr.view.route,&nfs_cr.motion))return 0;}++p->triangles;return 1;}
/* Bounded read-only oracle for an exact sidebar cache rejection. Comparing a
   second already retained source never grants admission or changes any pixels. */
static void nfs_cr_cache_source_note(void){
 const int saved=errno;const NfsBackgroundSceneFrameView*scene=nfs_ce_scene(&nfs_cr.view.semantic);
 size_t rawDiff=0,fullCropDiff=0;unsigned valid=1,first=0,rawWord=0,fullWord=0,nativeWord=0;
 if(!nfs_cr_current()||!scene||!scene->original565||scene->planeWords!=307200||
 !nfs_cr.view.semantic.sidebar565||nfs_cr.view.semantic.sidebarWords!=307200){errno=saved;return;}
 for(unsigned tile=0;tile<300&&valid;++tile){
  const uint32_t at=nfs_cr.view.semantic.sidebarKeys[tile];
  if((at&3)||at>TextureMem-2048){valid=0;break;}
  const TextureInfo*ti=g_textures+(at>>2);
  if(!ti->id||ti->size!=32||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||
  ti->data!=g_textureMem+at||!nfs_logo_native_upload_valid(at,ti)){valid=0;break;}
  for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){
   const unsigned px=(tile/15)*32+x,py=(tile%15)*32+y,index=py*640+px;
   const uint16_t raw=nfs_cr.view.semantic.sidebar565[index],full=px<343?scene->original565[index]:0;
   uint16_t a=(uint16_t)(((raw&0xf800)>>1)|((raw&0x7c0)>>1)|(raw&31));a=a?(uint16_t)(a|0x8000):0;
   uint16_t b=(uint16_t)(((full&0xf800)>>1)|((full&0x7c0)>>1)|(full&31));b=b?(uint16_t)(b|0x8000):0;
   const uint16_t expectedRaw=(uint16_t)((a<<1)|(a>>15)),expectedFull=(uint16_t)((b<<1)|(b>>15));
   uint16_t got=0;memcpy(&got,(const unsigned char*)ti->data+2*(y*32+x),2);
   if(got!=expectedRaw){if(!rawDiff){first=index;rawWord=raw;fullWord=full;nativeWord=got;}++rawDiff;}
   if(got!=expectedFull)++fullCropDiff;
  }
 }
 if(nfs_cr_current())fprintf(stderr,"CHILD RETURN SOURCE ORACLE route=%u family=%u frame=%u valid=%u rawDiff=%zu fullCropDiff=%zu first=%u raw565=%u full565=%u native5551=%u\n",nfs_cr.view.route,nfs_cr.view.semantic.family,nfs_cr.view.frameIndex,valid,rawDiff,fullCropDiff,first,rawWord,fullWord,nativeWord);
 errno=saved;
}
int nfs_private_child_return_stage(uint64_t serial,unsigned stage,int32_t x,int32_t y){NfsAudioNativeStamp stamp;uint32_t ids[300];GLenum filter=GL_LINEAR;drawTriangles();if(!nfs_cr_current()||serial!=nfs_ce.view.serial)return 0;int fade=nfs_cr.view.route==1;
 if(stage==NFS_CHILD_RETURN_CLEAR){if(nfs_ce.phase!=1||!nfs_ce.cleared||nfs_ce.clearEvent!=nfs_logo_provenance.clearEvent)goto refused;return 1;}
 if(stage==NFS_CHILD_RETURN_BASE_BEGIN){if(nfs_ce.phase!=1||!nfs_ce.cleared||!nfs_ce_choose(1)||!nfs_ce_stamp(&stamp)||!nfs_player_intro_cached_tiles(nfs_cr.view.baseOriginal565,nfs_cr.view.baseKeys,ids,&filter)||!nfs_player_intro_pass_begin(&nfs_ce.fullPass,&stamp,serial,nfs_logo_provenance.uploads,nfs_cr.view.baseKeys,ids))goto refused;nfs_ce.phase=2;return 1;}
 if(stage==NFS_CHILD_RETURN_BASE_END){if(nfs_ce.phase!=2||!nfs_ce_stamp(&stamp)||!nfs_player_intro_pass_finish(&nfs_ce.fullPass,&stamp,nfs_logo_provenance.uploads,serial)||(fade?!nfs_ce_compose(1):!nfs_cr_bare())||!nfs_cr_multiply())goto refused;nfs_ce.phase=3;nfs_ce.tailOrder=0;return 1;}
 if(stage==NFS_CHILD_RETURN_SIDEBAR_BEGIN){
  /* Keep every admission guard; distinguish a stale source-tile cache from
   * a phase/order/stamp refusal after several Setup mode changes. */
  unsigned refusal=0;
  if(nfs_ce.phase!=3)refusal=10;
  else if(nfs_ce.section)refusal=11;
  else if(fade?nfs_ce.tailOrder!=0:nfs_ce.tailOrder!=2)refusal=12;
  else if(!nfs_ce_stamp(&stamp))refusal=13;
  else if(nfs_cr.view.schema==2&&nfs_cr.view.semanticSidebarCurrent){
   if(!nfs_cr_semantic_sidebar())refusal=16;
   else if(!nfs_player_intro_cached_bindings(nfs_ce.view.sidebarKeys,ids,&filter))refusal=17;
  }else if(!nfs_player_intro_cached_tiles(nfs_ce.view.sidebar565,nfs_ce.view.sidebarKeys,ids,&filter))refusal=14;
  if(!refusal&&!nfs_player_intro_pass_begin(&nfs_ce.sidebarPass,&stamp,serial,nfs_logo_provenance.uploads,nfs_ce.view.sidebarKeys,ids))refusal=15;
  if(refusal){if(refusal==14&&nfs_logo_trace_enabled()){static unsigned count;if(count++<4){nfs_cr_cache_source_note();const int saved=errno;fprintf(stderr,"CHILD RETURN CACHE failure=%u tile=%u address=%u x=%u y=%u want=%u got=%u\n",nfs_player_intro_cache_diagnostic.reason,nfs_player_intro_cache_diagnostic.tile,nfs_player_intro_cache_diagnostic.address,nfs_player_intro_cache_diagnostic.x,nfs_player_intro_cache_diagnostic.y,nfs_player_intro_cache_diagnostic.want,nfs_player_intro_cache_diagnostic.got);errno=saved;}}nfs_cr_note(stage,refusal);goto refused;}
  nfs_ce.phase=5;return 1;
 }
 if(stage==NFS_CHILD_RETURN_SIDEBAR_END){if(nfs_ce.phase!=5||!nfs_ce_stamp(&stamp)||!nfs_player_intro_pass_finish(&nfs_ce.sidebarPass,&stamp,nfs_logo_provenance.uploads,serial)||!nfs_ce_compose(0))goto refused;nfs_ce.phase=fade?3:7;++nfs_ce.tailOrder;return 1;}
 if(stage==NFS_CHILD_RETURN_BRANDING_BEGIN){if(nfs_ce.phase!=3||nfs_ce.section||(fade?nfs_ce.tailOrder!=1:nfs_ce.tailOrder!=0))goto refused;nfs_ce.section=NFS_CHILD_EXIT_BRANDING_BEGIN;nfs_ce.ordinal=0;return 1;}
 if(stage==NFS_CHILD_RETURN_BRANDING_END){if(nfs_ce.phase!=3||nfs_ce.section!=NFS_CHILD_EXIT_BRANDING_BEGIN||nfs_ce.armed||nfs_ce.pending)goto refused;nfs_ce.section=0;++nfs_ce.tailOrder;if(fade)nfs_ce.phase=7;return 1;}
 if(stage==NFS_CHILD_RETURN_PHOTO_BEGIN){if(fade||nfs_ce.phase!=3||nfs_ce.tailOrder!=1||nfs_ce.section||x!=211-15*(int)nfs_cr.view.frameIndex||y!=316)goto refused;nfs_ce.phase=4;nfs_ce.section=NFS_CHILD_EXIT_PHOTO_BEGIN;nfs_ce.ordinal=0;return 1;}
 if(stage==NFS_CHILD_RETURN_PHOTO_END){if(fade||nfs_ce.phase!=4||nfs_ce.photoSelects!=100||nfs_ce.photoTriangles!=200||nfs_ce.armed||nfs_ce.pending)goto refused;nfs_ce.phase=3;nfs_ce.section=0;++nfs_ce.tailOrder;return 1;}
 if(stage==NFS_CHILD_RETURN_READY){if(nfs_ce.phase!=7||nfs_ce.section||(fade?nfs_ce.tailOrder!=2:nfs_ce.tailOrder!=3))goto refused;return 1;}
 if(stage==NFS_CHILD_RETURN_END){if(nfs_ce.phase!=9)goto refused;nfs_ce_pending();nfs_cr.active=0;memset(&nfs_cr.view,0,sizeof nfs_cr.view);return 1;}
 refused:nfs_cr_note(stage,1);nfs_child_exit_other();nfs_cr.active=0;return 0;}
int nfs_private_child_return_quad(const NfsChildReturnQuad*q){if(!q||!nfs_cr_current()||q->serial!=nfs_ce.view.serial||(q->section!=NFS_CHILD_RETURN_BRANDING_BEGIN&&q->section!=NFS_CHILD_RETURN_PHOTO_BEGIN))return 0;NfsChildExitQuad copy=*q;copy.section=q->section==NFS_CHILD_RETURN_PHOTO_BEGIN?NFS_CHILD_EXIT_PHOTO_BEGIN:NFS_CHILD_EXIT_BRANDING_BEGIN;if(nfs_ce_quad_accept(&copy))return 1;nfs_cr_note(q->section,20+nfs_ce_quad_reason);nfs_child_exit_other();nfs_cr.active=0;return 0;}
int nfs_private_child_return_cancel(uint64_t serial){if((nfs_ce.channel!=2&&nfs_ce.channel!=3)||!serial||serial!=nfs_ce.view.serial)return 0;nfs_child_exit_other();nfs_cr.active=0;return 1;}
static GLuint nfs_child_return_present(GLuint f,GLuint t,int w,int h){return nfs_cr_current()?nfs_ce_present_geometry(f,t,w,h):0;}
static void nfs_child_return_after_present(int okay){if(!nfs_cr_current())return;nfs_ce_after_present_geometry(okay);if(okay&&nfs_ce.phase==9&&nfs_cr_current()){static unsigned count[3];const int saved=errno;if(nfs_cr.view.route<3&&count[nfs_cr.view.route]++<64&&nfs_logo_trace_enabled())fprintf(stderr,"CHILD RETURN WIDE presented route=%u frame=%u serial=%llu phase=%d output=%ux%u fresh=1\n",nfs_cr.view.route,nfs_cr.view.frameIndex,(unsigned long long)nfs_ce.view.serial,nfs_cr.view.phase,nfs_ce.width,nfs_ce.height);errno=saved;}}
static void nfs_child_return_retired_gpu(int owner){if(owner&&nfs_cr.program&&nfs_cr.context==nfs_ce.context&&nfs_cr.generation==nfs_ce.generation&&nfs_ce_context())glDeleteProgram(nfs_cr.program);memset(&nfs_cr,0,sizeof nfs_cr);}
#undef NFS_CR_RETURN
#undef NFS_CR_GL
