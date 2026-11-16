// SPDX-License-Identifier: MIT
#include "hd/WideTransitionGeometry.h"
#include <stdio.h>
#include <math.h>
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"failed line %d\n",__LINE__);return 1;}}while(0)
int main(void){unsigned shapes[3][2]={{640,480},{1280,720},{2360,1640}};unsigned cases=0;NfsTransitionBand bands[20];
for(unsigned shape=0;shape<3;++shape)for(int d=0;d<2;++d)for(int phase=-1;phase<=255;++phase)for(int step=0;step<=32;++step){
 NfsTransitionMotion n={0,255,d,phase,-37,11,step};REQUIRE(nfs_transition_bands(&n,shapes[shape][0],shapes[shape][1],bands));
 for(unsigned k=0;k<20;++k){NfsTransitionBand*b=&bands[k];if(k)REQUIRE(bands[k-1].right==b->left);REQUIRE(b->top==11&&b->bottom==491);REQUIRE(b->u0==k/20.f&&b->u1==(k+1)/20.f);
 for(int right=0;right<2;++right){int offset=d?(right?228:240)-12*(int)k:(right?12:0)+12*(int)k;int v=phase==-1?255:2*phase-247+offset;if(v<0)v=0;if(v>255)v=255;REQUIRE((right?b->grayRight:b->grayLeft)==v/255.f);}
 if(shape==0){float oldLeft=(float)(-37+(int)k*step);REQUIRE(b->left==oldLeft&&b->right==oldLeft+step);}}
 ++cases;
}
NfsTransitionMotion n={0,255,0,-1,0,0,32};REQUIRE(!nfs_transition_bands(0,640,480,bands));REQUIRE(!nfs_transition_bands(&n,640,0,bands));REQUIRE(!nfs_transition_bands(&n,2000,720,bands));n.phase=256;REQUIRE(!nfs_transition_bands(&n,640,480,bands));printf("PASS %u motion plans, exact native geometry/gray equivalence; no presentation authority\n",cases);return 0;}
