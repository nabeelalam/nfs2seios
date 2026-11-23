// SPDX-License-Identifier: MIT
// Explicit production and preview fixtures; excluded from default builds/tests.
#include "hd/ClassicScreens/VictoryArtworkLoader.h"
#include "hd/ClassicScreens/VictoryArtProvider.h"
#include "hd/ClassicScreens/VictoryOriginalArtwork.h"
#include <cassert>
#include <cstdio>
int main(int argc,char**argv){assert(argc==3);char error[128]{};nfs_victory_art_clear();assert(!nfs_victory_art_load_bundled_file(argv[2],error,sizeof error));assert(!nfs_victory_art_generation());assert(nfs_victory_art_load_bundled_file(argv[1],error,sizeof error));auto generation=nfs_victory_art_generation();assert(generation);for(unsigned a=0;a<2;++a)for(unsigned i=0;i<16;++i){NfsVictoryArtView view{};assert(nfs_victory_art_acquire(a,i,nfs_victory_original::sources[i].sha,&view));assert(view.generation==generation&&view.aspect==a&&view.image==i);view.release(view.owner);}assert(!nfs_victory_art_load_bundled_file(argv[2],error,sizeof error)&&nfs_victory_art_generation()==generation);assert(!nfs_victory_art_load_bundled_file("/nonexistent-victory.ownedz",error,sizeof error)&&nfs_victory_art_generation()==generation);nfs_victory_art_clear();std::puts("Production32-image startup loaded; preview/missing refused; active generation preserved.");}
