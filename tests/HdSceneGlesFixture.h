// SPDX-License-Identifier: MIT
#pragma once
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdexcept>
#include <vector>
#include <string>
namespace nfs_hd_test {
inline void require(bool ok,const char*why){if(!ok)throw std::runtime_error(why);}
class Pbuffer {
 public:
 EGLDisplay display=EGL_NO_DISPLAY;EGLConfig config{};EGLSurface surface=EGL_NO_SURFACE;EGLContext context=EGL_NO_CONTEXT;int width,height;unsigned generation=0;
 explicit Pbuffer(int w=256,int h=192):width(w),height(h){
 display=eglGetDisplay(EGL_DEFAULT_DISPLAY);require(display!=EGL_NO_DISPLAY,"EGL display");require(eglInitialize(display,nullptr,nullptr),"EGL initialize");require(eglBindAPI(EGL_OPENGL_ES_API),"EGL bind GLES API");
 const EGLint attrs[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};EGLint count=0;
 require(eglChooseConfig(display,attrs,&config,1,&count)&&count,"RGBA8 EGL config");const EGLint p[]={EGL_WIDTH,w,EGL_HEIGHT,h,EGL_NONE};surface=eglCreatePbufferSurface(display,config,p);require(surface!=EGL_NO_SURFACE,"EGL pbuffer");createContext();
 }
 Pbuffer(const Pbuffer&)=delete;Pbuffer&operator=(const Pbuffer&)=delete;
 void unbind(){require(eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT),"EGL unbind");}
 void destroyUnboundContext(){require(eglGetCurrentContext()==EGL_NO_CONTEXT,"context still bound");if(context!=EGL_NO_CONTEXT){require(eglDestroyContext(display,context),"EGL destroy");context=EGL_NO_CONTEXT;}}
 void createContext(){require(context==EGL_NO_CONTEXT,"existing context");const EGLint ca[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};context=eglCreateContext(display,config,EGL_NO_CONTEXT,ca);require(context!=EGL_NO_CONTEXT,"EGL create");require(eglMakeCurrent(display,surface,surface,context),"EGL make current");++generation;}
 std::vector<unsigned char> rgba()const{std::vector<unsigned char>p(size_t(width)*height*4);glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,p.data());return p;}
 ~Pbuffer(){if(display!=EGL_NO_DISPLAY){eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);if(context!=EGL_NO_CONTEXT)eglDestroyContext(display,context);if(surface!=EGL_NO_SURFACE)eglDestroySurface(display,surface);eglTerminate(display);}}
};
inline GLuint compileShader(GLenum type,const char*source){GLuint id=glCreateShader(type);glShaderSource(id,1,&source,nullptr);glCompileShader(id);GLint ok=0;glGetShaderiv(id,GL_COMPILE_STATUS,&ok);if(!ok){char log[2048];GLsizei n=0;glGetShaderInfoLog(id,sizeof(log),&n,log);glDeleteShader(id);throw std::runtime_error(std::string(log,size_t(n)));}return id;}
}

#pragma once
#include <GLES2/gl2.h>
#include <cstring>
namespace nfs_hd_test {
static const GLenum fields[]={GL_CURRENT_PROGRAM,GL_FRAMEBUFFER_BINDING,GL_TEXTURE_BINDING_2D,GL_ACTIVE_TEXTURE,GL_ARRAY_BUFFER_BINDING,GL_ELEMENT_ARRAY_BUFFER_BINDING,GL_BLEND_SRC_RGB,GL_BLEND_DST_RGB,GL_BLEND_SRC_ALPHA,GL_BLEND_DST_ALPHA,GL_BLEND_EQUATION_RGB,GL_BLEND_EQUATION_ALPHA,GL_UNPACK_ALIGNMENT,GL_PACK_ALIGNMENT,GL_CULL_FACE_MODE,GL_FRONT_FACE,GL_DEPTH_FUNC};
struct State{GLint scalar[sizeof(fields)/sizeof(fields[0])],viewport[4],scissor[4],attr[8][6];void*pointer[8];GLboolean caps[6],color[4],depthMask;GLfloat gamma,blendColor[4],depthRange[2];};
static State capture(GLuint displayProgram=0,GLint gammaLocation=-1){State s{};for(unsigned i=0;i<sizeof(fields)/sizeof(fields[0]);++i)glGetIntegerv(fields[i],s.scalar+i);glGetIntegerv(GL_VIEWPORT,s.viewport);glGetIntegerv(GL_SCISSOR_BOX,s.scissor);GLenum caps[]={GL_BLEND,GL_DEPTH_TEST,GL_SCISSOR_TEST,GL_CULL_FACE,GL_DITHER,GL_STENCIL_TEST};for(int i=0;i<6;++i)s.caps[i]=glIsEnabled(caps[i]);glGetBooleanv(GL_COLOR_WRITEMASK,s.color);glGetBooleanv(GL_DEPTH_WRITEMASK,&s.depthMask);glGetFloatv(GL_BLEND_COLOR,s.blendColor);glGetFloatv(GL_DEPTH_RANGE,s.depthRange);if(displayProgram&&gammaLocation>=0)glGetUniformfv(displayProgram,gammaLocation,&s.gamma);for(int i=0;i<8;++i){GLenum a[]={GL_VERTEX_ATTRIB_ARRAY_ENABLED,GL_VERTEX_ATTRIB_ARRAY_SIZE,GL_VERTEX_ATTRIB_ARRAY_TYPE,GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,GL_VERTEX_ATTRIB_ARRAY_STRIDE,GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING};for(int j=0;j<6;++j)glGetVertexAttribiv(i,a[j],&s.attr[i][j]);glGetVertexAttribPointerv(i,GL_VERTEX_ATTRIB_ARRAY_POINTER,&s.pointer[i]);}return s;}

static bool equal(const State&a,const State&b){
 return !std::memcmp(a.scalar,b.scalar,sizeof(a.scalar))&&!std::memcmp(a.viewport,b.viewport,sizeof(a.viewport))&&!std::memcmp(a.scissor,b.scissor,sizeof(a.scissor))&&!std::memcmp(a.attr,b.attr,sizeof(a.attr))&&!std::memcmp(a.pointer,b.pointer,sizeof(a.pointer))&&!std::memcmp(a.caps,b.caps,sizeof(a.caps))&&!std::memcmp(a.color,b.color,sizeof(a.color))&&a.depthMask==b.depthMask&&a.gamma==b.gamma&&!std::memcmp(a.blendColor,b.blendColor,sizeof(a.blendColor))&&!std::memcmp(a.depthRange,b.depthRange,sizeof(a.depthRange));
}
}
