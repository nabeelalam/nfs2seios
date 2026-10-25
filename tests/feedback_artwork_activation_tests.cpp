// SPDX-License-Identifier: MIT
// Synthetic startup tests; no original data or workstation paths.
#include "FeedbackArtworkPackFixture.h"
#include "hd/ArtworkActivation.h"
#ifdef _WIN32
#include <direct.h>
static void directory(const std::string&p){(void)_mkdir(p.c_str());}
#else
#include <sys/stat.h>
static void directory(const std::string&p){(void)mkdir(p.c_str(),0700);}
#endif
int main(int argc,char**argv){try{
 CHECK(argc==3);const std::string root=argv[1],mode=argv[2];directory(root);directory(root+"/gamedata");
 FeedbackFixture fixture;AudioPackFixture legacy;const Bytes bytes=mode=="legacy-absent"?legacy.bytes:fixture.bytes;
 const auto pin=sha(bytes);{std::ofstream out(root+"/gamedata/remaster.nfshd",std::ios::binary);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());CHECK(bool(out));}
 NfsFeedbackModalBackdropArtView view{};
 if(mode=="sealed-optional")CHECK(!nfs_hd_feedback_modal_backdrop_art(&view));
 if(mode=="wrong-pin"){auto wrong=pin;wrong[0]^=1;CHECK(!nfs_hd::activateArtworkPack(root,wrong));CHECK(!nfs_hd_feedback_modal_backdrop_art_provider_available());return 0;}
 CHECK(nfs_hd::activateArtworkPack(root,pin));CHECK(nfs_hd::activateArtworkPack(root,pin));
 if(mode=="legacy-absent"||mode=="sealed-optional"){
  std::memset(&view,0x55,sizeof view);const auto saved=view;CHECK(!nfs_hd_feedback_modal_backdrop_art(&view));CHECK(!std::memcmp(&saved,&view,sizeof view));
 }else{
  CHECK(mode=="success");CHECK(nfs_hd_feedback_modal_backdrop_art_provider_available());CHECK(nfs_hd_feedback_modal_backdrop_art(&view));CHECK(view.count==2&&view.images[0].straightRgba==1&&view.images[0].rgba[0]==251&&view.images[0].rgba[3]==0);
  const auto saved=view;std::fill(fixture.bytes.begin(),fixture.bytes.end(),0);std::vector<unsigned char>noise(2*1024*1024,0xa5);CHECK(nfs_hd_feedback_modal_backdrop_art(&view));CHECK(view.images[0].rgba==saved.images[0].rgba&&view.images[0].rgba[0]==251);
 }
 std::printf("PASS optional Feedback startup %s\n",mode.c_str());return 0;
}catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
