// SPDX-License-Identifier: MIT
#include <assert.h>
#include <stdio.h>
#include "hd/NativeSceneProvenance.h"
static unsigned tests;
#define CHECK(x) do{assert(x);++tests;}while(0)
static uint32_t a[300],ids[300];
static void setup(NfsLogoProvenance*p){memset(p,0,sizeof*p);nfs_logo_p_created(p);nfs_logo_p_upload(p);nfs_logo_p_clear(p,9,1,1,65535);}
static void pass(NfsLogoProvenance*p){unsigned i;for(i=0;i<300;++i){nfs_logo_p_select(p,0,a[i],ids[i],11,32);nfs_logo_p_triangle(p,1);nfs_logo_p_triangle(p,1);}}
int main(void){NfsLogoProvenance p;unsigned i;for(i=0;i<300;++i){a[i]=i*2048;ids[i]=i+1;}setup(&p);CHECK(nfs_logo_p_begin(&p,1,9,a,ids));pass(&p);CHECK(nfs_logo_p_finish(&p,1,9));CHECK(!nfs_logo_p_finish(&p,1,9));CHECK(!nfs_logo_p_begin(&p,2,9,a,ids));
 for(i=0;i<5;++i){setup(&p);nfs_logo_p_clear(&p,9,i!=0,i!=1,i==2?0:65535);if(i==3)nfs_logo_p_triangle(&p,0);if(i==4)nfs_logo_p_retired(&p);CHECK(!nfs_logo_p_begin(&p,1,9,a,ids));}
 setup(&p);CHECK(!nfs_logo_p_begin(&p,1,8,a,ids));a[1]=a[0]+4;CHECK(!nfs_logo_p_begin(&p,1,9,a,ids));a[1]=2048;ids[2]=0;CHECK(!nfs_logo_p_begin(&p,1,9,a,ids));ids[2]=3;ids[1]=ids[0];CHECK(!nfs_logo_p_begin(&p,1,9,a,ids));ids[1]=2;a[299]=0x1ff804;CHECK(!nfs_logo_p_begin(&p,1,9,a,ids));a[299]=299*2048;
 for(i=0;i<9;++i){setup(&p);CHECK(nfs_logo_p_begin(&p,1,9,a,ids));switch(i){case 0:nfs_logo_p_upload(&p);break;case 1:nfs_logo_p_created(&p);break;case 2:nfs_logo_p_retired(&p);break;case 3:nfs_logo_p_select(&p,1,a[0],ids[0],11,32);break;case 4:nfs_logo_p_select(&p,0,a[1],ids[0],11,32);break;case 5:nfs_logo_p_select(&p,0,a[0],ids[0]+1,11,32);break;case 6:nfs_logo_p_select(&p,0,a[0],ids[0],10,32);break;case 7:nfs_logo_p_select(&p,0,a[0],ids[0],11,64);break;case 8:nfs_logo_p_triangle(&p,1);break;}CHECK(!nfs_logo_p_finish(&p,1,9));}
 setup(&p);CHECK(nfs_logo_p_begin(&p,1,9,a,ids));nfs_logo_p_select(&p,0,a[0],ids[0],11,32);nfs_logo_p_triangle(&p,0);CHECK(!nfs_logo_p_finish(&p,1,9));
 setup(&p);CHECK(nfs_logo_p_begin(&p,1,9,a,ids));pass(&p);nfs_logo_p_triangle(&p,1);CHECK(!nfs_logo_p_finish(&p,1,9));
 setup(&p);CHECK(nfs_logo_p_begin(&p,1,9,a,ids));pass(&p);CHECK(!nfs_logo_p_finish(&p,2,9));
 setup(&p);CHECK(nfs_logo_p_begin(&p,1,9,a,ids));nfs_logo_p_contaminate(&p);CHECK(!nfs_logo_p_finish(&p,1,9));
 setup(&p);p.context=UINT64_MAX;nfs_logo_p_created(&p);CHECK(p.disabled&&!p.alive);CHECK(!nfs_logo_p_begin(&p,1,9,a,ids));setup(&p);p.uploads=UINT64_MAX;nfs_logo_p_upload(&p);CHECK(p.disabled);setup(&p);p.clearEvent=UINT64_MAX;nfs_logo_p_clear(&p,9,1,1,65535);CHECK(p.disabled&&!p.clearValid);
 setup(&p);CHECK(nfs_logo_p_begin(&p,1,9,a,ids));pass(&p);CHECK(nfs_logo_p_finish(&p,1,9));nfs_logo_p_clear(&p,9,1,1,65535);CHECK(!nfs_logo_p_begin(&p,1,9,a,ids));CHECK(nfs_logo_p_begin(&p,2,9,a,ids));pass(&p);CHECK(nfs_logo_p_finish(&p,2,9));printf("PASS %u provenance checks; no GL/game claim\n",tests);return 0;}
