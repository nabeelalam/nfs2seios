// SPDX-License-Identifier: MIT
// Actual Runtime.inc GL-fault isolation. Only the original-font SHA boundary is
// stubbed with a synthetic owned buffer; this test makes no original-hash proof.
#include "TextureDigest.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>
#define CHECK(v) do{if(!(v)){std::fprintf(stderr,"upload check line %d: %s\n",__LINE__,#v);std::abort();}}while(0)
typedef unsigned GLuint;typedef int GLint;typedef unsigned GLenum;typedef float GLfloat;
enum{GL_MAX_TEXTURE_SIZE=1,GL_TEXTURE0=2,GL_TEXTURE1=3,GL_TEXTURE_2D=4,GL_TEXTURE_MIN_FILTER=5,GL_TEXTURE_MAG_FILTER=6,GL_LINEAR=7,GL_TEXTURE_WRAP_S=8,GL_TEXTURE_WRAP_T=9,GL_CLAMP_TO_EDGE=10,GL_RGBA=11,GL_UNSIGNED_BYTE=12,GL_NO_ERROR=0,GR_TEXFMT_ARGB_4444=13};
struct Tmu{float sow,tow,oow;};struct GrVertex{float x,y,z,r,g,b,ooz,a,oow;Tmu tmuvtx[2];};
struct TextureInfo{unsigned fmt,size,hdDigits;void*data;};
static int nfs_hd_builtin_enabled=1,nfs_hd_visible=1,nfs_hd_cell_layout_available=1,nfs_hd_selected=1,nfs_hd_uniform_sent=-1;
static GLuint nfs_hd_aux_texture=77,nfs_hd_bound_aux_texture=77;
static GLint nfs_hd_active_location=1;static unsigned long long nfs_hd_uploads;
static unsigned limitValue=2120,pendingError=0,unit=GL_TEXTURE0,boundAux=77,genCalls=0,bindCalls=0,uploadCalls=0,deleteCalls=0,errorCalls=0,lastHeight=0;
static bool failUpload=false;static unsigned char uploadSha[32];static std::vector<unsigned char>syntheticOriginal(131072);
static void glGetIntegerv(GLenum,GLint*p){*p=int(limitValue);}static GLenum glGetError(){++errorCalls;unsigned value=pendingError;pendingError=0;return value;}
static void glActiveTexture(GLenum v){unit=v;}static void glGenTextures(int,GLuint*p){++genCalls;*p=900;}
static void glBindTexture(GLenum,GLuint value){CHECK(unit==GL_TEXTURE1);++bindCalls;boundAux=value;}
static void glTexParameteri(GLenum,GLenum,GLint){}
static void glTexImage2D(GLenum,int,int,int w,int h,int,GLenum,GLenum,const void*p){CHECK(w==1024&&unit==GL_TEXTURE1);++uploadCalls;lastHeight=unsigned(h);CHECK(nfs_texture_sha256(p,size_t(w)*h*4,uploadSha));if(failUpload)pendingError=0x505;}
static void glDeleteTextures(int,const GLuint*p){++deleteCalls;if(boundAux==*p)boundAux=0;}
static void glUniform1f(GLint,GLfloat){}
static void nfs_front_hit_refresh(void);
static int upload_test_sha(const void*,size_t,unsigned char[32]);
#define nfs_texture_sha256 upload_test_sha
#include "FrontendFontRuntime.inc"
#undef nfs_texture_sha256
static int upload_test_sha(const void*p,size_t n,unsigned char out[32]){if(p==syntheticOriginal.data()&&n==syntheticOriginal.size()){std::memcpy(out,nfs_front_original_digest,32);return 1;}return nfs_texture_sha256(p,n,out);}
static void nfs_front_hit_refresh(void){}
extern "C" uint64_t nfs_front_text_context(void){return 0;}
extern "C" void nfs_front_text_context_set(uint64_t){}
#ifndef NFS_HD_AUX_SYNC_TEST_INCLUDE
#error Generate the exact maintained nfs_hd_sync_uniform body with frontend_font_upload_source_test.py
#endif
#include NFS_HD_AUX_SYNC_TEST_INCLUDE
static void reset(unsigned limit){limitValue=limit;pendingError=0;unit=GL_TEXTURE0;boundAux=77;nfs_hd_bound_aux_texture=77;nfs_front_font_texture=0;nfs_front_font_uploaded_height=0;nfs_hd_selected=1;nfs_hd_uniform_sent=-1;nfs_hd_uploads=0;genCalls=bindCalls=uploadCalls=deleteCalls=errorCalls=lastHeight=0;failUpload=false;for(unsigned p=0;p<2;++p){nfs_front_source_owner[p]=nullptr;nfs_front_source_matched[p]=0;nfs_front_font_valid[p]=0;}nfs_front_extended_ready=0;}
static TextureInfo input(){TextureInfo ti={GR_TEXFMT_ARGB_4444,256,0,syntheticOriginal.data()};return ti;}
static bool digest(const char*s){for(unsigned i=0;i<32;++i){unsigned value;if(std::sscanf(s+2*i,"%2x",&value)!=1||uploadSha[i]!=value)return false;}return true;}
int main(){
 reset(2120);auto ti=input();pendingError=0x502;CHECK(!nfs_front_font_upload(&ti));CHECK(errorCalls==1&&genCalls==0&&bindCalls==0&&uploadCalls==0&&nfs_front_font_texture==0&&nfs_front_font_uploaded_height==0&&boundAux==77&&nfs_hd_bound_aux_texture==77&&unit==GL_TEXTURE0&&ti.hdDigits==0);
 reset(2120);ti=input();failUpload=true;CHECK(!nfs_front_font_upload(&ti));CHECK(genCalls==1&&uploadCalls==1&&deleteCalls==1&&lastHeight==2120&&boundAux==0&&nfs_hd_bound_aux_texture==0&&nfs_front_font_texture==0&&nfs_front_font_uploaded_height==0&&unit==GL_TEXTURE0&&ti.hdDigits==0);unsigned prior=bindCalls;nfs_hd_sync_uniform();CHECK(bindCalls==prior+1&&boundAux==77&&nfs_hd_bound_aux_texture==77&&unit==GL_TEXTURE0);
 reset(2120);ti=input();CHECK(nfs_front_font_upload(&ti));CHECK(lastHeight==2120&&nfs_front_font_uploaded_height==2120&&ti.hdDigits==41&&boundAux==900&&nfs_hd_bound_aux_texture==900&&nfs_hd_uploads==1);CHECK(digest("84b561a78704e7d44469315bfc56321fd7372b3c8d46a54dfe0b3dbf79645c33"));
 reset(2048);ti=input();CHECK(nfs_front_font_upload(&ti));CHECK(lastHeight==2048&&nfs_front_font_uploaded_height==2048&&ti.hdDigits==41);CHECK(digest("3cfbcbe7c3b9f0330b629b9bbfdc75dc6a5ecb25c3a64c8a063d8e798c2340a4"));CHECK(!nfs_front_atlas_row_fits(.5f,256.25f,78,17,nfs_front_font_uploaded_height,2120));
 reset(2047);ti=input();CHECK(!nfs_front_font_upload(&ti));CHECK(!genCalls&&!uploadCalls&&!nfs_front_font_texture&&!nfs_front_font_uploaded_height);
 reset(2120);ti=input();ti.size=128;CHECK(!nfs_front_font_upload(&ti));CHECK(!genCalls&&!errorCalls);
 reset(2120);ti=input();ti.data=nullptr;CHECK(!nfs_front_font_upload(&ti));CHECK(!genCalls&&!errorCalls);
 std::puts("PASS actual Runtime.inc clean-boundary/pending-error/noalloc/upload-fault/cache0/actual-next-aux-rebind and realRGBA2048/full2120 payloads");
}
