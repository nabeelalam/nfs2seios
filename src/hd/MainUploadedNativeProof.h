// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <string.h>
/* Exact native receipt metadata; no GL ids alone or pixel resemblance is authority. */
typedef struct NfsMainUploadedNativeTiles {uint64_t context;uintptr_t thread;uint32_t addresses[300],ids[300],slots[300];uint64_t revisions[300];uint32_t low,high;}NfsMainUploadedNativeTiles;
static int nfs_main_uploaded_overlap(const NfsMainUploadedNativeTiles*t,uint32_t address,uint64_t bytes){
 if(!t||!bytes||bytes>0x200000u||address>0x200000u-bytes)return 1;
 if((uint64_t)address+bytes<=t->low||address>=t->high)return 0;
 for(unsigned i=0;i<300;++i)if((uint64_t)address+bytes>t->addresses[i]&&(uint64_t)t->addresses[i]+2048>address)return 1;
 return 0;
}
static int nfs_main_uploaded_witness_matches(const NfsMainUploadedNativeTiles*t,unsigned i,uint64_t context,uint64_t revision,uint32_t address,uint32_t id,int valid){return t&&i<300&&valid&&context==t->context&&revision&&revision==t->revisions[i]&&address==t->addresses[i]&&id==t->ids[i];}
