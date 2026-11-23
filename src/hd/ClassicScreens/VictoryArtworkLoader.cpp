// SPDX-License-Identifier: MIT
#include "VictoryArtworkLoader.h"
#include "VictoryArtworkPin.h"
#include "VictoryArtProvider.h"
#include <cstring>
int nfs_victory_art_load_bundled_file(const char*path,char*error,size_t capacity){
 const char*text=NFS_VICTORY_ARTWORK_METADATA_SHA256;unsigned char pin[32]{};
 bool valid=std::strlen(text)==64;unsigned nonzero=0;
 if(valid)for(unsigned k=0;k<64;++k){const char c=text[k];unsigned digit=0;if(c>='0'&&c<='9')digit=c-'0';else if(c>='a'&&c<='f')digit=c-'a'+10;else if(c>='A'&&c<='F')digit=c-'A'+10;else{valid=false;break;}pin[k/2]|=static_cast<unsigned char>(digit<<(k%2?0:4));nonzero|=digit;}
 if(!valid||!nonzero||!NFS_VICTORY_ARTWORK_FILE_BYTES){const char*why="Victory production artwork pin is not configured";if(error&&capacity){const size_t n=std::strlen(why)<capacity-1?std::strlen(why):capacity-1;std::memcpy(error,why,n);error[n]=0;}return 0;}
 return nfs_victory_art_load_file(path,NFS_VICTORY_ARTWORK_FILE_BYTES,pin,0,error,capacity);
}
