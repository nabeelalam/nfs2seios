// SPDX-License-Identifier: MIT
#include "hd/ClassicScreens/CreditsArtProvider.h"
#include "hd/ClassicScreens/CreditsArtworkLoader.h"
#include "TextureDigest.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>
#include <thread>
int main(int argc,char**argv){
 nfs_credits_art_clear();assert(!nfs_credits_art_generation());assert(!nfs_credits_art_install(nullptr));assert(!nfs_credits_art_acquire(nullptr));
 char error[128];assert(!nfs_credits_art_load_file(nullptr,error,sizeof error)&&error[0]);
 if(argc!=2){std::puts("Credits provider absent-input checks passed; positive pack checks require private artwork");return 0;}
 assert(nfs_credits_art_load_file(argv[1],error,sizeof error));NfsCreditsArtView view{};assert(nfs_credits_art_acquire(&view)&&view.generation&&view.schema==1);
 for(const auto&i:view.images){assert(i.pixels&&i.bytes==size_t(i.width)*i.height*4);unsigned char hash[32];assert(nfs_texture_sha256(i.pixels,i.bytes,hash)&&!std::memcmp(hash,i.sha256,32));}
 NfsCreditsArtInput input{};input.schema=1;std::vector<unsigned char>owned[NFS_CREDITS_ART_COUNT];
 for(unsigned n=0;n<NFS_CREDITS_ART_COUNT;++n){input.images[n]=view.images[n];owned[n].assign(view.images[n].pixels,view.images[n].pixels+view.images[n].bytes);input.images[n].pixels=owned[n].data();}
 assert(!nfs_credits_art_install(&input));uint64_t generation=view.generation;nfs_credits_art_clear();assert(!nfs_credits_art_generation());
 // Acquired bytes outlive clear. Bad alpha, sizes, digests and roles are rejected.
 for(unsigned n=0;n<NFS_CREDITS_ART_COUNT;++n){auto&i=input.images[n];auto saved=i;i.sha256[0]^=1;assert(!nfs_credits_art_install(&input));i=saved;i.bytes--;assert(!nfs_credits_art_install(&input));i=saved;i.height=0;assert(!nfs_credits_art_install(&input));i=saved;}
 auto&background=input.images[0];auto saved=owned[0][3];owned[0][3]=254;assert(nfs_texture_sha256(owned[0].data(),owned[0].size(),background.sha256));assert(!nfs_credits_art_install(&input));owned[0][3]=saved;assert(nfs_texture_sha256(owned[0].data(),owned[0].size(),background.sha256));
 auto&word=input.images[3];const auto old=owned[3][0];owned[3][0]=255;owned[3][3]=0;assert(nfs_texture_sha256(owned[3].data(),owned[3].size(),word.sha256));assert(!nfs_credits_art_install(&input));owned[3].assign(view.images[3].pixels,view.images[3].pixels+view.images[3].bytes);word=view.images[3];word.pixels=owned[3].data();(void)old;
 assert(nfs_credits_art_install(&input));for(auto&v:owned){v.clear();v.shrink_to_fit();}
 NfsCreditsArtView next{};assert(nfs_credits_art_acquire(&next)&&next.generation>generation);view.retain(view.owner);view.release(view.owner);
 std::thread worker([]{nfs_credits_art_clear();});worker.join();assert(!nfs_credits_art_generation());
 for(const auto&i:next.images){unsigned char hash[32];assert(nfs_texture_sha256(i.pixels,i.bytes,hash)&&!std::memcmp(hash,i.sha256,32));}
 next.release(next.owner);view.release(view.owner);std::puts("Credits pinned pack, eleven roles, owned bytes, digest/shape/PMA rejection and retained lifetime passed");
}
