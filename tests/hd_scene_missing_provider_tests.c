// SPDX-License-Identifier: MIT
#include <GLES2/gl2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TextureMem 0x200000
#define VertexSnap 0xc0000
#define GR_BUFFER_BACKBUFFER 1
#define GR_TEXFMT_ARGB_1555 11
typedef struct{unsigned smallLod,largeLod,aspectRatio,format;}GrTexInfo;
typedef struct{void*data;unsigned fmt,size,id,hdDigits;}TextureInfo;
typedef struct{float sow,tow,oow;}Tmu;
typedef struct{float x,y,z,r,g,b,ooz,a,oow;Tmu tmuvtx[2];}GrVertex;
static TextureInfo g_textures[TextureMem>>2];static uint8_t g_textureMem[TextureMem];static void*g_glCtx;
static int g_framebufferHeight,linearFiltering,g_renderBuffer,g_depthMask,g_textureEnabled,g_fogEnabled;
static unsigned g_framebuffer,g_shaderProgram,g_framebufferTexture,g_framebufferDepth;static float g_matrix[16];
static unsigned optionalQueries,flushes;
static void*SDL_GL_GetCurrentContext(void){++optionalQueries;return g_glCtx;}
static uintptr_t SDL_GetCurrentThreadID(void){++optionalQueries;return 1;}
static void drawTriangles(void){++flushes;}
static void countedGet(GLenum key,GLint*out){++optionalQueries;glGetIntegerv(key,out);}
#define glGetIntegerv countedGet
#include "hd/BackgroundSceneRenderer.inc"
#undef glGetIntegerv
int main(int argc,char**argv){if(argc!=2)return 1;if(strcmp(argv[1],"unset"))setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD",argv[1],1);else unsetenv("NFS_PRIVATE_BACKGROUND_SCENE_HD");
 if(nfs_private_background_scene_begin(NULL)||nfs_private_background_scene_finish(1))return 2;
 nfs_logo_native_created();nfs_logo_native_retired(0);nfs_logo_native_other();nfs_logo_native_state();nfs_logo_native_upload_begin(NULL);nfs_logo_native_upload_done(NULL);nfs_logo_native_triangle(NULL,NULL,NULL);nfs_logo_native_clear(65535,0,0,0);
 if(optionalQueries||flushes||nfs_bg_compositor.generation)return 3;
 printf("PASS %s missing provider zero optional queries/flush/context ownership\n",argv[1]);return 0;}
