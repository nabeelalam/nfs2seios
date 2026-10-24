// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_SETUP_WORD_GPU_IMPLEMENTATION
#define NFS_CONTROLS_SETUP_WORD_GPU_IMPLEMENTATION
#include "ControlsSetupWordGpu.h"
#include "ControlsSetupWordIdentity.h"
/* Mechanical child Controls only: the caller supplies the independently owned Controls
   context/staging guard. No source-pass, FIFO, native-upload, FBO or publication
   authority is created here. Caller must first join genuine current Controls
   source proof, retained packet/art and the actual original native witness;
   validate original software-font vertices; preflight all resource aliases;
   and supply a current unpublished staging guard. The finite28-image view is mandatory; original dynamic text is retained
   between ordered replacement spans. Any refusal or late error requires original fallback/discard of the
   entire staging transaction, never publication of a partial replacement.
   Prerequisites: neutral stage, text/background GPU types, digest implementation
   and guarded nfs_nested_caption_{current,save,restore,blend} helpers included. */
static int nfs_controls_setup_word_empty(const NfsControlsSetupWordsView*v){unsigned i,j;if(!v||v->schema||v->count)return 0;for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i){const NfsControlsSetupWordImageView*a=v->images+i;if(a->word||a->width||a->height||a->rgba||a->bytes)return 0;for(j=0;j<32;++j)if(a->sha256[j])return 0;}for(i=0;i<NFS_CONTROLS_SETUP_WORD_PLACEMENTS;++i){const NfsControlsSetupWordPlacement*a=v->placements+i;if(a->word||a->color565||a->vertexFirst||a->vertexCount||a->left||a->top||a->right||a->bottom)return 0;}return 1;}
static int nfs_controls_setup_word_valid(const NfsControlsSetupWordsView*v,const NfsSoftwareTextLayerView*text){
 unsigned i;size_t cursor=0;NfsSoftwareTextStage stage;float bounds[4];
 /* Schema and count precede all borrowed image/vertex access. The full source
    queue separately proves these placements correspond to current child text. */
 if(!v||v->schema!=NFS_CONTROLS_SETUP_WORD_SCHEMA||!v->count||v->count>NFS_CONTROLS_SETUP_WORD_PLACEMENTS||!text||text->logicalWidth!=640||text->logicalHeight!=480||!text->vertices||!text->vertexCount||text->vertexCount>12288||text->vertexCount%6)return 0;
 memset(&stage,0,sizeof stage);stage.logicalWidth=640;stage.logicalHeight=480;
 if(!nfs_soft_gpu_vertices(&stage,text->vertices,text->vertexCount,bounds))return 0;
 for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i){const NfsControlsSetupWordImageView*a=v->images+i;
  if(a->word!=i+1||a->width!=nfs_controls_setup_word_width[i]||!a->width||a->height!=128||a->bytes!=(size_t)a->width*128u*4u||!a->rgba||memcmp(a->sha256,nfs_controls_setup_word_sha256[i],32))return 0;
 }
 for(i=0;i<v->count;++i){const NfsControlsSetupWordPlacement*p=v->placements+i;
  if(!p->word||p->word>NFS_CONTROLS_SETUP_WORD_IMAGES||p->color565>65535u||!p->vertexCount||p->vertexFirst%6||p->vertexCount%6||p->vertexFirst<cursor||p->vertexFirst>text->vertexCount||p->vertexCount>text->vertexCount-p->vertexFirst||!isfinite(p->left)||!isfinite(p->top)||!isfinite(p->right)||!isfinite(p->bottom)||p->left<0||p->top<0||p->right>640||p->bottom>480||p->left>=p->right||p->top>=p->bottom)return 0;
  cursor=p->vertexFirst+p->vertexCount;
 }
 return 1;
}
/* Borrowed pixels are read only when creating owned textures. The immutable
   image IDs and expected digests above cannot change with text placements.
   Once uploaded, cached draws use those authenticated GPU copies; repeatedly
   hashing the unused CPU buffers does not authenticate any later GL operation. */
static int nfs_controls_setup_word_pixels_valid(const NfsControlsSetupWordsView*v){
 unsigned i;unsigned char sha[32];
 for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i){const NfsControlsSetupWordImageView*a=v->images+i;
  if(!nfs_texture_sha256(a->rgba,a->bytes,sha)||memcmp(sha,nfs_controls_setup_word_sha256[i],32))return 0;
 }
 return 1;
}
static void nfs_controls_setup_word_key(const NfsControlsSetupWordsView*v,const NfsSoftwareTextLayerView*text,unsigned char out[32]){unsigned char b[4096];size_t used=0;unsigned i;
#define NFS_CSW_KEY(x) do{memcpy(b+used,&(x),sizeof(x));used+=sizeof(x);}while(0)
 NFS_CSW_KEY(text->vertexCount);NFS_CSW_KEY(v->count);for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i){const NfsControlsSetupWordImageView*a=v->images+i;NFS_CSW_KEY(a->word);NFS_CSW_KEY(a->width);NFS_CSW_KEY(a->height);memcpy(b+used,a->sha256,32);used+=32;}for(i=0;i<v->count;++i){const NfsControlsSetupWordPlacement*a=v->placements+i;NFS_CSW_KEY(a->word);NFS_CSW_KEY(a->color565);NFS_CSW_KEY(a->vertexFirst);NFS_CSW_KEY(a->vertexCount);NFS_CSW_KEY(a->left);NFS_CSW_KEY(a->top);NFS_CSW_KEY(a->right);NFS_CSW_KEY(a->bottom);}nfs_texture_sha256(b,used,out);
#undef NFS_CSW_KEY
}
static void nfs_controls_setup_word_forget(NfsControlsSetupWordGpu*s){if(s)memset(s,0,sizeof *s);}
#define NFS_CSW_GL(call) do{if(!nfs_nested_caption_current(t))goto lost;call;if(!nfs_nested_caption_current(t))goto lost;}while(0)
static int nfs_controls_setup_word_retire(NfsControlsSetupWordGpu*s,const NfsBackgroundSceneStage*t,int owner){unsigned i;if(!s)return 0;if(owner&&t&&s->generation==t->generation&&nfs_nested_caption_current(t)){if(s->program){NFS_CSW_GL(glDeleteProgram(s->program));}for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i)if(s->textures[i]){NFS_CSW_GL(glDeleteTextures(1,s->textures+i));}}nfs_controls_setup_word_forget(s);return 1;lost:nfs_controls_setup_word_forget(s);return 0;}
static int nfs_controls_setup_word_ready(NfsControlsSetupWordGpu*s,const NfsControlsSetupWordsView*v,const NfsSoftwareTextLayerView*text,const NfsBackgroundSceneStage*t){unsigned i;GLboolean alive;unsigned char key[32];if(!s||!nfs_nested_caption_current(t)||!nfs_controls_setup_word_valid(v,text)||nfs_controls_setup_word_empty(v)||s->generation!=t->generation||!s->program||s->matrix<0||s->sampler<0)return 0;nfs_controls_setup_word_key(v,text,key);if(memcmp(key,s->key,32))return 0;NFS_CSW_GL(alive=glIsProgram(s->program));if(!alive)return 0;for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i){if(!s->textures[i])return 0;NFS_CSW_GL(alive=glIsTexture(s->textures[i]));if(!alive)return 0;}return 1;lost:nfs_controls_setup_word_forget(s);return 0;}
static int nfs_controls_setup_word_alias(const NfsControlsSetupWordGpu*s,const NfsBackgroundSceneCompositor*b,const NfsSoftwareTextGpu*c,const NfsSoftwareTextGpu*f,const NfsNestedCaptionGpu*side,GLuint color,GLuint original){unsigned i,j;if(!s||!b||!c||!f)return 1;if(s->program&&(s->program==b->program||s->program==b->copyProgram||s->program==b->logo.program||s->program==c->program||s->program==f->program||(side&&s->program==side->program)))return 1;for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i)if(s->textures[i]){GLuint a=s->textures[i];if(a==color||a==original||a==b->logo.texture||a==b->previewTexture||a==c->texture||a==f->texture||(side&&a==side->texture))return 1;for(j=0;j<5;++j)if(a==b->textures[j])return 1;for(j=0;j<2;++j)if(a==b->color[j])return 1;for(j=0;j<i;++j)if(a==s->textures[j])return 1;}return 0;}
static int nfs_controls_setup_word_prepare(NfsControlsSetupWordGpu*s,const NfsControlsSetupWordsView*v,const NfsSoftwareTextLayerView*text,const NfsBackgroundSceneStage*t){GLuint vs=0,fs=0,program=0,textures[NFS_CONTROLS_SETUP_WORD_IMAGES]={0};GLint linked=0,matrix=-1,sampler=-1;GLboolean alive;GLenum error;NfsBgSaved saved;unsigned i;int ok=0;
 static const char vertex[]="attribute vec4 vertex;attribute vec4 color;uniform mat4 matrix;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;tint=color;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_FragColor=tint*texture2D(image,uv).a;}";const char*vp=vertex,*fp=fragment;
 if(!s||!nfs_controls_setup_word_valid(v,text)||!nfs_nested_caption_current(t))return 0;if(s->generation&&s->generation!=t->generation)return 0;
 if(!s->program){for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i)if(s->textures[i])return 0;}
 if(s->program){if(s->matrix<0||s->sampler<0)return 0;NFS_CSW_GL(alive=glIsProgram(s->program));if(!alive)return 0;for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i){if(!s->textures[i])return 0;NFS_CSW_GL(alive=glIsTexture(s->textures[i]));if(!alive)return 0;}nfs_controls_setup_word_key(v,text,s->key);return 1;}
 if(!nfs_controls_setup_word_pixels_valid(v))return 0;
 if(!nfs_nested_caption_save(&saved,t))goto lost;
 NFS_CSW_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_CSW_GL(glShaderSource(vs,1,&vp,NULL));NFS_CSW_GL(glCompileShader(vs));NFS_CSW_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&linked));if(!linked)goto done;
 NFS_CSW_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_CSW_GL(glShaderSource(fs,1,&fp,NULL));NFS_CSW_GL(glCompileShader(fs));NFS_CSW_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&linked));if(!linked)goto done;
 NFS_CSW_GL(program=glCreateProgram());if(!program)goto done;NFS_CSW_GL(glAttachShader(program,vs));NFS_CSW_GL(glAttachShader(program,fs));NFS_CSW_GL(glBindAttribLocation(program,0,"vertex"));NFS_CSW_GL(glBindAttribLocation(program,1,"color"));NFS_CSW_GL(glLinkProgram(program));NFS_CSW_GL(glGetProgramiv(program,GL_LINK_STATUS,&linked));if(!linked)goto done;NFS_CSW_GL(matrix=glGetUniformLocation(program,"matrix"));NFS_CSW_GL(sampler=glGetUniformLocation(program,"image"));if(matrix<0||sampler<0)goto done;
 NFS_CSW_GL(glActiveTexture(GL_TEXTURE0));NFS_CSW_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i){NFS_CSW_GL(glGenTextures(1,textures+i));if(!textures[i])goto done;NFS_CSW_GL(glBindTexture(GL_TEXTURE_2D,textures[i]));NFS_CSW_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_CSW_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_CSW_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_CSW_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_CSW_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,v->images[i].width,128,0,GL_RGBA,GL_UNSIGNED_BYTE,v->images[i].rgba));NFS_CSW_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;}ok=1;
 done:if(vs){NFS_CSW_GL(glDeleteShader(vs));}if(fs){NFS_CSW_GL(glDeleteShader(fs));}if(!ok){if(program){NFS_CSW_GL(glDeleteProgram(program));}for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i)if(textures[i]){NFS_CSW_GL(glDeleteTextures(1,textures+i));}}if(!nfs_nested_caption_restore(&saved,t))goto lost;if(!ok)return 0;s->program=program;memcpy(s->textures,textures,sizeof textures);s->matrix=matrix;s->sampler=sampler;s->generation=t->generation;nfs_controls_setup_word_key(v,text,s->key);return 1;
 lost:nfs_controls_setup_word_forget(s);return 0;
}
/* Original font slices and authored words use the same premultiplied tint math. */
static int nfs_controls_setup_word_vertices(GLuint program,GLuint texture,GLint matrix,GLint sampler,const NfsSoftwareTextVertex*vertices,size_t count,const NfsBackgroundSceneStage*t){unsigned i;GLenum error;if(!count)return 1;for(i=0;i<8;++i){NFS_CSW_GL(glDisable(nfs_frame_caps[i]));}NFS_CSW_GL(glColorMask(1,1,1,1));NFS_CSW_GL(glDepthMask(GL_FALSE));NFS_CSW_GL(glActiveTexture(GL_TEXTURE0));NFS_CSW_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_CSW_GL(glUseProgram(program));NFS_CSW_GL(glUniformMatrix4fv(matrix,1,GL_FALSE,t->sampling.projection));NFS_CSW_GL(glUniform1i(sampler,0));NFS_CSW_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_CSW_GL(glEnableVertexAttribArray(0));NFS_CSW_GL(glEnableVertexAttribArray(1));NFS_CSW_GL(glDisableVertexAttribArray(2));NFS_CSW_GL(glDisableVertexAttribArray(3));NFS_CSW_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),vertices));NFS_CSW_GL(glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),&vertices[0].r));if(!nfs_nested_caption_blend(t,1))return -1;NFS_CSW_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count));NFS_CSW_GL(error=glGetError());return error==GL_NO_ERROR;lost:return -1;}
static int nfs_controls_setup_word_draw(NfsControlsSetupWordGpu*s,const NfsControlsSetupWordsView*v,NfsSoftwareTextGpu*font,const NfsSoftwareTextLayerView*text,const NfsBackgroundSceneStage*t){NfsBgSaved saved;size_t cursor=0;unsigned i;int ok=1;GLint fbo=0,type=0,name=0,viewport[4];if(!font||!font->program||!font->texture||font->matrix<0||font->sampler<0||!nfs_controls_setup_word_ready(s,v,text,t)||font->generation!=t->generation||!t->framebuffer||!nfs_frame_sampling_valid(t->width,t->height,&t->sampling))return 0;NFS_CSW_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_CSW_GL(glGetIntegerv(GL_VIEWPORT,viewport));if((GLuint)fbo!=t->framebuffer||memcmp(viewport,t->sampling.viewport,sizeof viewport))return 0;NFS_CSW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CSW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||!name||(GLuint)name==font->texture)return 0;for(i=0;i<NFS_CONTROLS_SETUP_WORD_IMAGES;++i)if((GLuint)name==s->textures[i])return 0;if(!nfs_nested_caption_save(&saved,t))goto lost;for(i=0;i<v->count&&ok;++i){const NfsControlsSetupWordPlacement*p=v->placements+i;NfsSoftwareTextVertex q[6];float r=(float)(((p->color565>>11)<<3)|((p->color565>>11)>>2))/255.f;unsigned g=(p->color565>>5)&63,b=p->color565&31;float green=(float)((g<<2)|(g>>4))/255.f,blue=(float)((b<<3)|(b>>2))/255.f;
 if(p->vertexFirst>cursor)ok=nfs_controls_setup_word_vertices(font->program,font->texture,font->matrix,font->sampler,text->vertices+cursor,p->vertexFirst-cursor,t);if(ok<0)goto lost;if(!ok)break;
 {const NfsSoftwareTextVertex a={p->left,p->top,0,0,r,green,blue,1},b0={p->left,p->bottom,0,1,r,green,blue,1},c={p->right,p->top,1,0,r,green,blue,1},d={p->right,p->bottom,1,1,r,green,blue,1};q[0]=a;q[1]=b0;q[2]=c;q[3]=c;q[4]=b0;q[5]=d;}
 ok=nfs_controls_setup_word_vertices(s->program,s->textures[p->word-1],s->matrix,s->sampler,q,6,t);if(ok<0)goto lost;cursor=p->vertexFirst+p->vertexCount;}if(ok&&cursor<text->vertexCount)ok=nfs_controls_setup_word_vertices(font->program,font->texture,font->matrix,font->sampler,text->vertices+cursor,text->vertexCount-cursor,t);if(ok<0)goto lost;if(!nfs_nested_caption_restore(&saved,t))goto lost;return ok;lost:nfs_controls_setup_word_forget(s);return 0;}
#undef NFS_CSW_GL
#endif
