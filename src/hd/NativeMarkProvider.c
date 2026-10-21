// SPDX-License-Identifier: MIT
#include "NativeMarkProvider.h"
#include <stdatomic.h>
static NfsNativeMarkProvider installed;
static atomic_int sealed;
static int ready;
int nfs_hd_native_mark_provider_available(void){return ready;}
int nfs_hd_install_native_mark_provider(const NfsNativeMarkProvider*p){
 if(atomic_load_explicit(&sealed,memory_order_relaxed)||ready||!p||
    p->schema!=NFS_NATIVE_MARK_PROVIDER_SCHEMA||!p->lookup)return 0;
 installed=*p;ready=1;return 1;
}
int nfs_hd_native_mark_art(uint32_t encoding,uint32_t edge,
                          const uint8_t digest[32],NfsNativeMarkArt*out){
 NfsNativeMarkArt value={0};NfsNativeMarkProfile profile;unsigned nonzero=0;
 atomic_store_explicit(&sealed,1,memory_order_relaxed);
 if(!ready||!out||!digest||!nfs_native_mark_profile(encoding,edge,&profile))return 0;
 for(unsigned k=0;k<32;++k)nonzero|=digest[k];
 if(!nonzero||!installed.lookup(installed.user,encoding,edge,digest,&value))return 0;
 *out=value;return 1;
}
