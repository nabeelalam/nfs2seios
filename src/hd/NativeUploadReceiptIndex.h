// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <string.h>
#define NFS_NATIVE_UPLOAD_RECEIPTS 1024u
#define NFS_NATIVE_UPLOAD_RECEIPT_BYTES 2048u
/* Completed native uploads only. Index choice is never a pixel/source proof. */
typedef struct {uint64_t context,revision;uint32_t address,id,format;unsigned valid;unsigned char bytes[2048];}NfsLogoUploadWitness;
typedef struct {uint64_t context,revision;uint32_t address,id,format,slot;unsigned valid;}NfsNativeUploadChoice;
static void nfs_native_upload_receipt_begin(NfsLogoUploadWitness*w,unsigned*evict,uint64_t context,uint32_t address,uint64_t bytes,int tracked,NfsNativeUploadChoice*out){unsigned same=1024,freeSlot=1024;if(!out)return;memset(out,0,sizeof*out);if(!w||!evict||!context||!bytes||bytes>UINT64_MAX-address)return;const uint64_t end=(uint64_t)address+bytes;
 for(unsigned i=0;i<1024;++i){NfsLogoUploadWitness*r=w+i;if(r->valid&&r->context!=context)r->valid=0;if(r->valid&&(uint64_t)r->address<end&&(uint64_t)address<(uint64_t)r->address+2048){if(r->address==address)same=i;r->valid=0;}if(!r->valid&&freeSlot==1024)freeSlot=i;}
 if(!tracked||bytes!=2048)return;unsigned slot=same!=1024?same:freeSlot!=1024?freeSlot:(*evict)++%1024;w[slot].valid=0;out->context=context;out->address=address;out->slot=slot;out->valid=1;
}
/* InitialID0 is allowed only for the paired native upload which creates its
 * texture AFTER begin; caller proves completed GL upload/current new ID. */
static int nfs_native_upload_receipt_commit(NfsLogoUploadWitness*w,NfsNativeUploadChoice*p,uint64_t context,uint64_t revision,uint32_t address,uint32_t id,uint32_t format,const unsigned char*bytes){if(!w||!p||!p->valid||p->slot>=1024||!context||p->context!=context||!revision||p->revision!=revision||p->address!=address||!id||(p->id&&p->id!=id)||p->format!=format||!bytes)return 0;NfsLogoUploadWitness*r=w+p->slot;r->valid=0;r->context=context;r->revision=revision;r->address=address;r->id=id;r->format=format;memcpy(r->bytes,bytes,2048);r->valid=1;p->valid=0;return 1;}
static int nfs_native_upload_receipt_current(const NfsLogoUploadWitness*w,uint64_t context,uint64_t revision,uint32_t address,uint32_t id,uint32_t format,const unsigned char*bytes){return w&&w->valid&&context&&w->context==context&&revision&&w->revision==revision&&w->address==address&&id&&w->id==id&&w->format==format&&bytes&&!memcmp(w->bytes,bytes,2048);}
