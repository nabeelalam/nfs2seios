// SPDX-License-Identifier: MIT
// Source-only pre-ThinLTO code-generation probe, never a runnable app.
#include "GuestCopyTransaction.h"
extern "C" __attribute__((noinline)) int32_t nfs_probe_accessor(nfs::GuestCopyTransaction*tx,uint32_t address){return tx->load<int32_t>(address);}
extern "C" __attribute__((noinline)) void nfs_probe_transform(uint32_t source,uint32_t destination){
 nfs::GuestCopyTransaction tx;
 #pragma clang loop unroll(full)
 for(unsigned k=0;k<9;++k){int32_t value=tx.load<int32_t>(source+4*k);tx.store<int32_t>(destination+4*k,value);}
}
extern "C" __attribute__((noinline)) void nfs_probe_rep8(uint32_t&source,uint32_t&destination,uint32_t&count){
 nfs::GuestCopyTransaction tx;
 for(unsigned k=0;k<8&&count;++k){int32_t value=tx.load<int32_t>(source);tx.store<int32_t>(destination,value);source+=4;destination+=4;--count;}
}
