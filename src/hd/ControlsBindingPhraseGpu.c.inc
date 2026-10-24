// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_BINDING_PHRASE_GPU_IMPLEMENTATION
#define NFS_CONTROLS_BINDING_PHRASE_GPU_IMPLEMENTATION
#include "ControlsBindingPhraseGpu.h"
#include "ControlsBindingPhraseIdentity.h"
/* Include after the guarded nested-caption helpers and
   nfs_controls_setup_word_vertices. The caller retains all image owners and
   joins current original source, FIFO and native-upload authority separately.
   No method here binds an FBO or publishes a target. */
static int nfs_controls_binding_phrase_empty(const NfsControlsBindingPhrasesView*v){
 unsigned i,j;if(!v||v->schema||v->imageCount||v->count)return 0;
 for(i=0;i<NFS_CONTROLS_BINDING_PHRASE_SLOTS;++i){const NfsControlsBindingPhraseImageView*a=v->images+i;const NfsControlsBindingPhrasePlacement*p=v->placements+i;
  if(a->phrase||a->width||a->height||a->rgba||a->bytes||p->slot||p->foreground565||p->shadow565||p->vertexFirst||p->vertexCount||p->left||p->top||p->right||p->bottom)return 0;
  for(j=0;j<32;++j)if(a->sha256[j])return 0;
 }return 1;
}
static int nfs_controls_binding_phrase_metadata(const NfsControlsBindingPhrasesView*v,size_t vertices){
 unsigned i,j;size_t cursor=0;
 if(nfs_controls_binding_phrase_empty(v))return 1;
 if(!v||v->schema!=NFS_CONTROLS_BINDING_PHRASE_SCHEMA||!v->imageCount||v->imageCount>NFS_CONTROLS_BINDING_PHRASE_SLOTS||!v->count||v->count>NFS_CONTROLS_BINDING_PHRASE_SLOTS||!vertices||vertices>12288||vertices%6)return 0;
 for(i=0;i<v->imageCount;++i){const NfsControlsBindingPhraseImageView*a=v->images+i;const NfsControlsBindingPhraseIdentity*d=nfs_controls_binding_phrase_identity(a->phrase);
  if(!d||a->width!=d->width||a->height!=128||a->height!=d->height||!a->rgba||a->bytes!=(size_t)a->width*128u*4u||memcmp(a->sha256,d->sha256,32))return 0;
  for(j=0;j<i;++j)if(v->images[j].phrase==a->phrase)return 0;
 }
 for(i=0;i<v->count;++i){const NfsControlsBindingPhrasePlacement*p=v->placements+i;
  if(!p->slot||p->slot>v->imageCount||p->foreground565>65535u||p->shadow565>65535u||!p->vertexCount||p->vertexFirst%6||p->vertexCount%6||p->vertexFirst<cursor||p->vertexFirst>vertices||p->vertexCount>vertices-p->vertexFirst||!isfinite(p->left)||!isfinite(p->top)||!isfinite(p->right)||!isfinite(p->bottom)||p->left<0||p->top<0||p->right>639||p->bottom>479||p->left>=p->right||p->top>=p->bottom||p->right-p->left!=(float)v->images[p->slot-1].width/4.f||p->bottom-p->top!=32.f)return 0;
  cursor=p->vertexFirst+p->vertexCount;
 }return 1;
}
/* Metadata only: no image payload reads and no GL operations. */
static int nfs_controls_binding_phrase_valid(const NfsControlsBindingPhrasesView*v,const NfsSoftwareTextLayerView*text){
 if(nfs_controls_binding_phrase_empty(v))return 1;
 return text&&text->logicalWidth==640&&text->logicalHeight==480&&text->vertices&&nfs_controls_binding_phrase_metadata(v,text->vertexCount);
}
static void nfs_controls_binding_phrase_key(const NfsControlsBindingPhrasesView*v,size_t vertices,unsigned char out[32]){
 unsigned char b[3072];size_t used=0;unsigned i;
#define NFS_CBP_KEY(x) do{memcpy(b+used,&(x),sizeof(x));used+=sizeof(x);}while(0)
 NFS_CBP_KEY(vertices);NFS_CBP_KEY(v->imageCount);NFS_CBP_KEY(v->count);
 for(i=0;i<v->imageCount;++i){const NfsControlsBindingPhraseImageView*a=v->images+i;NFS_CBP_KEY(a->phrase);NFS_CBP_KEY(a->width);NFS_CBP_KEY(a->height);memcpy(b+used,a->sha256,32);used+=32;}
 for(i=0;i<v->count;++i){const NfsControlsBindingPhrasePlacement*p=v->placements+i;NFS_CBP_KEY(p->slot);NFS_CBP_KEY(p->foreground565);NFS_CBP_KEY(p->shadow565);NFS_CBP_KEY(p->vertexFirst);NFS_CBP_KEY(p->vertexCount);NFS_CBP_KEY(p->left);NFS_CBP_KEY(p->top);NFS_CBP_KEY(p->right);NFS_CBP_KEY(p->bottom);}
 nfs_texture_sha256(b,used,out);
#undef NFS_CBP_KEY
}
static void nfs_controls_binding_phrase_forget(NfsControlsBindingPhraseGpu*s){if(s)memset(s,0,sizeof *s);}
static int nfs_controls_binding_phrase_slot(const NfsControlsBindingPhraseGpu*s,unsigned phrase){unsigned i;for(i=0;i<NFS_CONTROLS_BINDING_PHRASE_SLOTS;++i)if(s->phrases[i]==phrase&&s->textures[i])return (int)i;return -1;}
#define NFS_CBP_GL(call) do{if(!nfs_nested_caption_current(t))goto lost;call;if(!nfs_nested_caption_current(t))goto lost;}while(0)
static int nfs_controls_binding_phrase_retire(NfsControlsBindingPhraseGpu*s,const NfsBackgroundSceneStage*t,int owner){unsigned i;if(!s)return 0;
 if(owner&&t&&s->generation==t->generation&&nfs_nested_caption_current(t)){if(s->program){NFS_CBP_GL(glDeleteProgram(s->program));}for(i=0;i<NFS_CONTROLS_BINDING_PHRASE_SLOTS;++i)if(s->textures[i]){NFS_CBP_GL(glDeleteTextures(1,s->textures+i));}}
 nfs_controls_binding_phrase_forget(s);return 1;lost:nfs_controls_binding_phrase_forget(s);return 0;
}
static int nfs_controls_binding_phrase_ready(NfsControlsBindingPhraseGpu*s,const NfsControlsBindingPhrasesView*v,const NfsSoftwareTextLayerView*text,const NfsBackgroundSceneStage*t){
 unsigned i;int slot;GLboolean alive;unsigned char key[32];if(nfs_controls_binding_phrase_empty(v))return 1;
 if(!s||!nfs_controls_binding_phrase_valid(v,text)||!nfs_nested_caption_current(t)||s->generation!=t->generation||!s->program||s->matrix<0||s->sampler<0||s->textVertexCount!=text->vertexCount)return 0;
 nfs_controls_binding_phrase_key(v,text->vertexCount,key);if(memcmp(key,s->key,32))return 0;
 NFS_CBP_GL(alive=glIsProgram(s->program));if(!alive)return 0;
 for(i=0;i<v->imageCount;++i){slot=nfs_controls_binding_phrase_slot(s,v->images[i].phrase);if(slot<0)return 0;NFS_CBP_GL(alive=glIsTexture(s->textures[slot]));if(!alive)return 0;}
 return 1;lost:nfs_controls_binding_phrase_forget(s);return 0;
}
static int nfs_controls_binding_phrase_alias(const NfsControlsBindingPhraseGpu*s,const NfsBackgroundSceneCompositor*b,const NfsSoftwareTextGpu*c,const NfsSoftwareTextGpu*f,const NfsNestedCaptionGpu*side,GLuint color,GLuint original){
 unsigned i,j;if(!s||!b||!c||!f)return 1;
 if(s->program&&(s->program==b->program||s->program==b->copyProgram||s->program==b->logo.program||s->program==c->program||s->program==f->program||(side&&s->program==side->program)))return 1;
 for(i=0;i<NFS_CONTROLS_BINDING_PHRASE_SLOTS;++i)if(s->textures[i]){GLuint a=s->textures[i];if(a==color||a==original||a==b->logo.texture||a==b->previewTexture||a==c->texture||a==f->texture||(side&&a==side->texture))return 1;for(j=0;j<5;++j)if(a==b->textures[j])return 1;for(j=0;j<2;++j)if(a==b->color[j])return 1;for(j=0;j<i;++j)if(a==s->textures[j])return 1;}
 return 0;
}
static int nfs_controls_binding_phrase_prepare(NfsControlsBindingPhraseGpu*s,const NfsControlsBindingPhrasesView*v,const NfsSoftwareTextLayerView*text,const NfsBackgroundSceneStage*t){
 GLuint vs=0,fs=0,program=0,made[20]={0};int slots[20],used[20]={0},missing[20]={0};GLint status=0,matrix=-1,sampler=-1,maxSize=0,units=0,bound=0;GLboolean alive;GLenum error;NfsBgSaved saved;unsigned i,j;unsigned char sha[32];int ok=0;
 static const char vertex[]="attribute vec4 vertex;attribute vec4 color;uniform mat4 matrix;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;tint=color;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_FragColor=tint*texture2D(image,uv).a;}";const char*vp=vertex,*fp=fragment;
 if(nfs_controls_binding_phrase_empty(v))return 1;
 if(!s||!nfs_controls_binding_phrase_valid(v,text)||!nfs_nested_caption_current(t)||(s->generation&&s->generation!=t->generation))return 0;
 for(i=0;i<20;++i){if((s->textures[i]!=0)!=(s->phrases[i]!=0))return 0;if(s->phrases[i]&&!nfs_controls_binding_phrase_identity(s->phrases[i]))return 0;for(j=0;j<i;++j)if(s->phrases[i]&&(s->phrases[i]==s->phrases[j]||s->textures[i]==s->textures[j]))return 0;if(!s->program&&s->textures[i])return 0;}
 for(i=0;i<v->imageCount;++i){slots[i]=nfs_controls_binding_phrase_slot(s,v->images[i].phrase);if(slots[i]>=0)used[slots[i]]=1;else missing[i]=1;}
 /* Authenticate EVERY missing complete phrase before the first GL operation. */
 for(i=0;i<v->imageCount;++i)if(missing[i]){const NfsControlsBindingPhraseImageView*a=v->images+i;if(!nfs_texture_sha256(a->rgba,a->bytes,sha)||memcmp(sha,nfs_controls_binding_phrase_identity(a->phrase)->sha256,32))return 0;}
 for(i=0;i<v->imageCount;++i)if(missing[i]){for(j=0;j<20&&used[j];++j){}if(j==20)return 0;slots[i]=(int)j;used[j]=1;}
 if(s->program){if(!s->generation||s->matrix<0||s->sampler<0)return 0;NFS_CBP_GL(alive=glIsProgram(s->program));if(!alive)return 0;for(i=0;i<20;++i)if(s->textures[i]){NFS_CBP_GL(alive=glIsTexture(s->textures[i]));if(!alive)return 0;}}
 for(i=0;i<v->imageCount&&!missing[i];++i){}
 if(i==v->imageCount&&s->program){s->textVertexCount=text->vertexCount;nfs_controls_binding_phrase_key(v,text->vertexCount,s->key);return 1;}
 NFS_CBP_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));for(i=0;i<v->imageCount;++i)if(maxSize<(GLint)v->images[i].width||maxSize<128)return 0;
 if(!nfs_nested_caption_save(&saved,t))goto lost;
 /* Deleting an inherited binding on ANY unit would alter caller state.
    Query only; no binding is changed by this eviction preflight. */
 for(i=0;i<v->imageCount;++i)if(missing[i]&&s->textures[slots[i]])break;
 if(i<v->imageCount){
  NFS_CBP_GL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,&units));if(units<=0||units>256)goto done;
  for(j=0;j<(unsigned)units;++j){NFS_CBP_GL(glActiveTexture(GL_TEXTURE0+j));NFS_CBP_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound));for(i=0;i<v->imageCount;++i)if(missing[i]&&s->textures[slots[i]]&&(GLuint)bound==s->textures[slots[i]])goto done;}
 }
 if(!s->program){
  NFS_CBP_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_CBP_GL(glShaderSource(vs,1,&vp,NULL));NFS_CBP_GL(glCompileShader(vs));NFS_CBP_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&status));if(!status)goto done;
  NFS_CBP_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_CBP_GL(glShaderSource(fs,1,&fp,NULL));NFS_CBP_GL(glCompileShader(fs));NFS_CBP_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&status));if(!status)goto done;
  NFS_CBP_GL(program=glCreateProgram());if(!program)goto done;NFS_CBP_GL(glAttachShader(program,vs));NFS_CBP_GL(glAttachShader(program,fs));NFS_CBP_GL(glBindAttribLocation(program,0,"vertex"));NFS_CBP_GL(glBindAttribLocation(program,1,"color"));NFS_CBP_GL(glLinkProgram(program));NFS_CBP_GL(glGetProgramiv(program,GL_LINK_STATUS,&status));if(!status)goto done;NFS_CBP_GL(matrix=glGetUniformLocation(program,"matrix"));NFS_CBP_GL(sampler=glGetUniformLocation(program,"image"));if(matrix<0||sampler<0)goto done;
 }
 NFS_CBP_GL(glActiveTexture(GL_TEXTURE0));NFS_CBP_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));
 for(i=0;i<v->imageCount;++i)if(missing[i]){NFS_CBP_GL(glGenTextures(1,made+i));if(!made[i])goto done;NFS_CBP_GL(glBindTexture(GL_TEXTURE_2D,made[i]));NFS_CBP_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_CBP_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_CBP_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_CBP_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_CBP_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,v->images[i].width,128,0,GL_RGBA,GL_UNSIGNED_BYTE,v->images[i].rgba));NFS_CBP_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;}
 ok=1;
 done:
 if(vs){NFS_CBP_GL(glDeleteShader(vs));}if(fs){NFS_CBP_GL(glDeleteShader(fs));}
 if(!ok){if(program){NFS_CBP_GL(glDeleteProgram(program));}for(i=0;i<20;++i)if(made[i]){NFS_CBP_GL(glDeleteTextures(1,made+i));}}
 else for(i=0;i<v->imageCount;++i)if(missing[i]&&s->textures[slots[i]]){NFS_CBP_GL(glDeleteTextures(1,s->textures+slots[i]));s->textures[slots[i]]=0;s->phrases[slots[i]]=0;}
 if(!nfs_nested_caption_restore(&saved,t))goto lost;if(!ok)return 0;
 if(program){s->program=program;s->matrix=matrix;s->sampler=sampler;}
 for(i=0;i<v->imageCount;++i)if(missing[i]){s->textures[slots[i]]=made[i];s->phrases[slots[i]]=v->images[i].phrase;}
 s->generation=t->generation;s->textVertexCount=text->vertexCount;nfs_controls_binding_phrase_key(v,text->vertexCount,s->key);return 1;
 lost:nfs_controls_binding_phrase_forget(s);return 0;
}
/* The parent saves/restores state around the WHOLE ordered text transaction.
   This method neither allocates nor changes context, viewport or framebuffer. */
static int nfs_controls_binding_phrase_draw_into_scope(NfsControlsBindingPhraseGpu*s,const NfsControlsBindingPhrasesView*v,unsigned placementIndex,const NfsBackgroundSceneStage*t){
 const NfsControlsBindingPhrasePlacement*p;NfsSoftwareTextVertex q[6];GLint fbo=0,type=0,name=0,viewport[4];unsigned char key[32];unsigned i,pass;int slot,ok;
 if(nfs_controls_binding_phrase_empty(v))return 1;
 if(!s||!nfs_controls_binding_phrase_metadata(v,s->textVertexCount)||placementIndex>=v->count||!nfs_nested_caption_current(t)||s->generation!=t->generation||!s->program||s->matrix<0||s->sampler<0||!t->framebuffer||!nfs_frame_sampling_valid(t->width,t->height,&t->sampling))return 0;
 nfs_controls_binding_phrase_key(v,s->textVertexCount,key);if(memcmp(key,s->key,32))return 0;p=v->placements+placementIndex;slot=nfs_controls_binding_phrase_slot(s,v->images[p->slot-1].phrase);if(slot<0)return 0;
 NFS_CBP_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_CBP_GL(glGetIntegerv(GL_VIEWPORT,viewport));if((GLuint)fbo!=t->framebuffer||memcmp(viewport,t->sampling.viewport,sizeof viewport))return 0;
 NFS_CBP_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CBP_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||!name)return 0;
 for(i=0;i<20;++i)if(s->textures[i]&&(GLuint)name==s->textures[i])return 0;
 for(pass=0;pass<2;++pass){uint32_t color=pass?p->foreground565:p->shadow565;unsigned r=(color>>11)&31,g=(color>>5)&63,b=color&31;float red=(float)((r<<3)|(r>>2))/255.f,green=(float)((g<<2)|(g>>4))/255.f,blue=(float)((b<<3)|(b>>2))/255.f;float d=pass?0.f:1.f;
  const NfsSoftwareTextVertex a={p->left+d,p->top+d,0,0,red,green,blue,1},b0={p->left+d,p->bottom+d,0,1,red,green,blue,1},c={p->right+d,p->top+d,1,0,red,green,blue,1},e={p->right+d,p->bottom+d,1,1,red,green,blue,1};q[0]=a;q[1]=b0;q[2]=c;q[3]=c;q[4]=b0;q[5]=e;
  ok=nfs_controls_setup_word_vertices(s->program,s->textures[slot],s->matrix,s->sampler,q,6,t);if(ok<0)goto lost;if(!ok)return 0;
 }return 1;lost:nfs_controls_binding_phrase_forget(s);return -1;
}
#undef NFS_CBP_GL
#endif
