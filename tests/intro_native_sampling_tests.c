// SPDX-License-Identifier: MIT
#include <GLES2/gl2.h>
#include "hd/ClassicScreens/IntroSampling.h"
#include "hd/SceneGpuSupport.inc"
#include <assert.h>
#include <stdio.h>
int main(void){
 const unsigned heights[]={480,720,820,1080,1640,2048};unsigned priorRejected=0;
 for(unsigned i=0;i<sizeof heights/sizeof heights[0];++i){
  NfsNativeLogoTargetProof t={0};t.height=heights[i];t.width=t.height*4/3;
  assert(nfs_frame_gpu_legacy_sampling(t.width,t.height,GL_LINEAR,&t.sampling));
  assert(nfs_intro_sampling_supported(&t));
  if(t.sampling.viewport[2]!=(int)t.width){++priorRejected;assert(t.sampling.viewport[2]==(int)t.width+1);}
  NfsNativeLogoTargetProof bad=t;bad.sampling.viewport[0]=1;assert(!nfs_intro_sampling_supported(&bad));
  bad=t;bad.sampling.viewport[2]++;assert(!nfs_intro_sampling_supported(&bad));
  bad=t;bad.sampling.projection[0]*=2;assert(!nfs_intro_sampling_supported(&bad));
  bad=t;bad.width++;assert(!nfs_intro_sampling_supported(&bad));
  bad=t;bad.sampling.magFilter=0;assert(!nfs_intro_sampling_supported(&bad));
 }
 assert(priorRejected==2);assert(!nfs_intro_sampling_supported(0));
 puts("PASS exact native sampling at6resolutions; oldcheck rejects1640/2048;30invalidmutations rejected");return 0;
}
