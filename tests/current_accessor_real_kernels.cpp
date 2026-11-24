// SPDX-License-Identifier: MIT
// Synthetic finite accessor probe, not game code or a gameplay/energy claim.
#include "GuestCopyTransaction.h"
#include <cstdint>
#include <cstdlib>
extern "C" __attribute__((noinline)) uint32_t nfs_current_heap(uint32_t source,uint32_t destination,unsigned words){
 if(words>32)std::abort();nfs::GuestCopyTransaction tx;uint32_t checksum=0;
 for(unsigned i=0;i<words;++i){auto value=tx.load<uint32_t>(source+4*i);tx.store<uint32_t>(destination+4*i,value+1u);checksum^=value+i*0x9e3779b9u;}
 return checksum;
}
extern "C" __attribute__((noinline)) uint32_t nfs_current_native(void*source,void*destination,unsigned words){
 if(words>32)std::abort();nfs::GuestCopyTransaction tx;uint32_t checksum=0;auto s=static_cast<unsigned char*>(source),d=static_cast<unsigned char*>(destination);
 for(unsigned i=0;i<words;++i){auto value=tx.load<uint32_t>(s+4*i);tx.store<uint32_t>(d+4*i,value+1u);checksum^=value+i*0x9e3779b9u;}
 return checksum;
}
extern "C" __attribute__((noinline)) uint32_t nfs_current_mixed(uint32_t source,uint32_t destination,void*native_source,void*native_destination,unsigned words){
 if(words>32)std::abort();nfs::GuestCopyTransaction tx;uint32_t checksum=0;auto s=static_cast<unsigned char*>(native_source),d=static_cast<unsigned char*>(native_destination);
 for(unsigned i=0;i<words;++i){uint32_t value;if(i&1){value=tx.load<uint32_t>(s+4*i);tx.store<uint32_t>(destination+4*i,value+1u);}else{value=tx.load<uint32_t>(source+4*i);tx.store<uint32_t>(d+4*i,value+1u);}checksum^=value+i*0x9e3779b9u;}
 return checksum;
}
