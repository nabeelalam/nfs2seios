// SPDX-License-Identifier: MIT
#include "SceneGpuSupport.h"
#include <string.h>
static GLuint nfs_text_gpu_shader(GLenum type,const char*source){
 GLuint shader=glCreateShader(type);GLint ok=0;if(!shader)return 0;glShaderSource(shader,1,&source,NULL);glCompileShader(shader);glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);if(!ok){glDeleteShader(shader);return 0;}return shader;
}
static void nfs_text_gpu_capture_attribute(GLuint location,NfsSoftwareTextGpuAttrib*s){
 glGetVertexAttribiv(location,GL_VERTEX_ATTRIB_ARRAY_ENABLED,&s->enabled);glGetVertexAttribiv(location,GL_VERTEX_ATTRIB_ARRAY_SIZE,&s->size);glGetVertexAttribiv(location,GL_VERTEX_ATTRIB_ARRAY_TYPE,&s->type);glGetVertexAttribiv(location,GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,&s->normalized);glGetVertexAttribiv(location,GL_VERTEX_ATTRIB_ARRAY_STRIDE,&s->stride);glGetVertexAttribiv(location,GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,&s->buffer);glGetVertexAttribPointerv(location,GL_VERTEX_ATTRIB_ARRAY_POINTER,&s->pointer);glGetVertexAttribfv(location,GL_CURRENT_VERTEX_ATTRIB,s->current);
}
static void nfs_text_gpu_restore_attribute(GLuint location,const NfsSoftwareTextGpuAttrib*s){glBindBuffer(GL_ARRAY_BUFFER,s->buffer);glVertexAttribPointer(location,s->size,s->type,s->normalized,s->stride,s->pointer);glVertexAttrib4fv(location,s->current);if(s->enabled)glEnableVertexAttribArray(location);else glDisableVertexAttribArray(location);}
static const GLenum nfs_frame_caps[]={GL_BLEND,GL_DEPTH_TEST,GL_CULL_FACE,GL_STENCIL_TEST,GL_SCISSOR_TEST,GL_SAMPLE_ALPHA_TO_COVERAGE,GL_SAMPLE_COVERAGE,GL_DITHER};
static void nfs_frame_save(NfsFrameSaved*s){
 glGetIntegerv(GL_CURRENT_PROGRAM,&s->program);glGetIntegerv(GL_ACTIVE_TEXTURE,&s->active);glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_TEXTURE_BINDING_2D,&s->texture);glActiveTexture(s->active);
 glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&s->buffer);glGetIntegerv(GL_FRAMEBUFFER_BINDING,&s->fbo);glGetIntegerv(GL_RENDERBUFFER_BINDING,&s->rbo);glGetIntegerv(GL_VIEWPORT,s->viewport);glGetIntegerv(GL_PACK_ALIGNMENT,&s->pack);glGetIntegerv(GL_UNPACK_ALIGNMENT,&s->unpack);
 glGetIntegerv(GL_BLEND_SRC_RGB,&s->sr);glGetIntegerv(GL_BLEND_DST_RGB,&s->dr);glGetIntegerv(GL_BLEND_SRC_ALPHA,&s->sa);glGetIntegerv(GL_BLEND_DST_ALPHA,&s->da);glGetIntegerv(GL_BLEND_EQUATION_RGB,&s->eqr);glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&s->eqa);glGetBooleanv(GL_COLOR_WRITEMASK,s->mask);
 for(unsigned i=0;i<8;++i)s->caps[i]=glIsEnabled(nfs_frame_caps[i]);for(unsigned i=0;i<4;++i)nfs_text_gpu_capture_attribute(i,s->attributes+i);
}
static void nfs_frame_restore(const NfsFrameSaved*s){
 for(unsigned i=0;i<4;++i)nfs_text_gpu_restore_attribute(i,s->attributes+i);glBindBuffer(GL_ARRAY_BUFFER,s->buffer);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,s->texture);glActiveTexture(s->active);glUseProgram(s->program);
 glBindFramebuffer(GL_FRAMEBUFFER,s->fbo);glBindRenderbuffer(GL_RENDERBUFFER,s->rbo);glViewport(s->viewport[0],s->viewport[1],s->viewport[2],s->viewport[3]);glPixelStorei(GL_PACK_ALIGNMENT,s->pack);glPixelStorei(GL_UNPACK_ALIGNMENT,s->unpack);glColorMask(s->mask[0],s->mask[1],s->mask[2],s->mask[3]);
 glBlendEquationSeparate(s->eqr,s->eqa);glBlendFuncSeparate(s->sr,s->dr,s->sa,s->da);for(unsigned i=0;i<8;++i){if(s->caps[i])glEnable(nfs_frame_caps[i]);else glDisable(nfs_frame_caps[i]);}
}
static void nfs_frame_draw_state(void){for(unsigned i=0;i<8;++i)glDisable(nfs_frame_caps[i]);glColorMask(1,1,1,1);glActiveTexture(GL_TEXTURE0);glBindBuffer(GL_ARRAY_BUFFER,0);}
static void nfs_frame_texture_params(void){glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);}
static int nfs_frame_dimensions(unsigned w,unsigned h){GLint texture=0,viewport[2];if(!w||!h||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)w*h>NFS_FRAME_MAX_PIXELS)return 0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&texture);glGetIntegerv(GL_MAX_VIEWPORT_DIMS,viewport);return w<=(unsigned)texture&&h<=(unsigned)texture&&w<=(unsigned)viewport[0]&&h<=(unsigned)viewport[1];}
int nfs_frame_gpu_legacy_sampling(unsigned w,unsigned h,GLenum mag,NfsFrameLegacySampling*out){
 if(!out||!h||h>NFS_FRAME_MAX_DIMENSION||w!=h*4u/3u||(mag!=GL_LINEAR&&mag!=GL_NEAREST))return 0;NfsFrameLegacySampling p;memset(&p,0,sizeof p);const float ratio=h/480.0f;const GLint vw=(GLint)(640.0f*ratio+0.5f),vh=(GLint)(480.0f*ratio+0.5f);if(vw<=0||vh<=0)return 0;p.magFilter=mag;p.viewport[2]=vw;p.viewport[3]=vh;p.projection[0]=(2.0f/vw)*ratio;p.projection[5]=(-2.0f/vh)*ratio;p.projection[10]=-2;p.projection[12]=-1;p.projection[13]=1;p.projection[14]=1;p.projection[15]=1;*out=p;return 1;
}
static int nfs_frame_sampling_valid(unsigned w,unsigned h,const NfsFrameLegacySampling*p){NfsFrameLegacySampling expected;if(!p||!nfs_frame_gpu_legacy_sampling(w,h,p->magFilter,&expected))return 0;return memcmp(expected.viewport,p->viewport,sizeof p->viewport)==0&&memcmp(expected.projection,p->projection,sizeof p->projection)==0;}
