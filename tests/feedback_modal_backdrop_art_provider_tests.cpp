// SPDX-License-Identifier: MIT
#include "hd/FeedbackModalBackdropArtProvider.h"
#include <array>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(0)
static std::array<unsigned char,NFS_FEEDBACK_MODAL_BACKDROP_BANNER_BYTES>banner{};
static std::array<unsigned char,NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM_BYTES>emblem{};
static unsigned variant=0;
static int supplied(void*user,NfsFeedbackModalBackdropArtView*out){
 if(user!=banner.data())return 0;NfsFeedbackModalBackdropArtView v{};v.schema=NFS_FEEDBACK_MODAL_BACKDROP_ART_SCHEMA;v.count=2;
 for(unsigned i=0;i<2;++i){auto&p=v.images[i];p.schema=NFS_FEEDBACK_MODAL_BACKDROP_ART_SCHEMA;p.role=i+1;p.width=i?NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM_WIDTH:NFS_FEEDBACK_MODAL_BACKDROP_BANNER_WIDTH;p.height=i?NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM_HEIGHT:NFS_FEEDBACK_MODAL_BACKDROP_BANNER_HEIGHT;p.straightRgba=1;p.rgba=i?emblem.data():banner.data();p.bytes=i?emblem.size():banner.size();p.sha256[31]=1;}
 if(variant==1)v.schema=0;if(variant==2)v.count=1;
 if(variant>=3&&variant<=22){auto&p=v.images[(variant-3)/10];switch((variant-3)%10){case 0:p.schema=0;break;case 1:p.role=3;break;case 2:p.width--;break;case 3:p.height++;break;case 4:p.straightRgba=0;break;case 5:p.rgba=nullptr;break;case 6:p.bytes=0;break;case 7:--p.bytes;break;case 8:++p.bytes;break;case 9:std::memset(p.sha256,0,32);break;}}
 if(variant==23){const auto image=v.images[0];v.images[0]=v.images[1];v.images[1]=image;}
 *out=v;return variant!=24;
}
static int failed(void*,NfsFeedbackModalBackdropArtView*out){std::memset(out,0x7b,sizeof *out);return 0;}
static NfsFeedbackModalBackdropArtProvider provider(){return {NFS_FEEDBACK_MODAL_BACKDROP_ART_PROVIDER_SCHEMA,banner.data(),supplied};}
static bool valid(const NfsFeedbackModalBackdropArtView&v){return v.schema==1&&v.count==2&&v.images[0].role==1&&v.images[1].role==2&&v.images[0].rgba==banner.data()&&v.images[1].rgba==emblem.data()&&v.images[0].bytes==banner.size()&&v.images[1].bytes==emblem.size();}
int main(int argc,char**argv){try{
 CHECK(argc==2);NfsFeedbackModalBackdropArtView out;std::memset(&out,0x5a,sizeof out);const auto saved=out;
 if(!std::strcmp(argv[1],"missing")){CHECK(!nfs_hd_feedback_modal_backdrop_art_provider_available());CHECK(!nfs_hd_feedback_modal_backdrop_art(&out));CHECK(!std::memcmp(&out,&saved,sizeof out));auto p=provider();CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(&p));}
 else if(!std::strcmp(argv[1],"null-first")){CHECK(!nfs_hd_feedback_modal_backdrop_art(nullptr));auto p=provider();CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(&p));CHECK(!nfs_hd_feedback_modal_backdrop_art_provider_available());}
 else if(!std::strcmp(argv[1],"failed-callback")){auto p=provider();p.view=failed;CHECK(nfs_hd_install_feedback_modal_backdrop_art_provider(&p));CHECK(!nfs_hd_feedback_modal_backdrop_art(&out));CHECK(!std::memcmp(&out,&saved,sizeof out));CHECK(nfs_hd_feedback_modal_backdrop_art_provider_available());CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(&p));}
 else if(!std::strcmp(argv[1],"metadata")){auto p=provider();CHECK(nfs_hd_install_feedback_modal_backdrop_art_provider(&p));for(variant=1;variant<=24;++variant){CHECK(!nfs_hd_feedback_modal_backdrop_art(&out));CHECK(!std::memcmp(&out,&saved,sizeof out));}variant=0;CHECK(nfs_hd_feedback_modal_backdrop_art(&out));CHECK(valid(out));CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(&p));}
 else if(!std::strcmp(argv[1],"descriptor-copy")){auto p=provider();CHECK(nfs_hd_install_feedback_modal_backdrop_art_provider(&p));CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(&p));p.schema=0;p.user=nullptr;p.view=failed;CHECK(nfs_hd_feedback_modal_backdrop_art(&out));CHECK(valid(out));CHECK(!nfs_hd_feedback_modal_backdrop_art(nullptr));CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(&p));}
 else if(!std::strcmp(argv[1],"invalid-install")){auto p=provider(),bad=p;bad.schema=0;CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(&bad));bad=p;bad.view=nullptr;CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(&bad));CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(nullptr));CHECK(!nfs_hd_feedback_modal_backdrop_art_provider_available());CHECK(nfs_hd_install_feedback_modal_backdrop_art_provider(&p));CHECK(nfs_hd_feedback_modal_backdrop_art(&out));CHECK(valid(out));}
 else {CHECK(!std::strcmp(argv[1],"concurrent"));auto p=provider();CHECK(nfs_hd_install_feedback_modal_backdrop_art_provider(&p));std::atomic<bool>okay{true};std::atomic<unsigned>calls{0};std::vector<std::thread>threads;for(unsigned k=0;k<8;++k)threads.emplace_back([&]{for(unsigned n=0;n<1000;++n){NfsFeedbackModalBackdropArtView v{};if(!nfs_hd_feedback_modal_backdrop_art(&v)||!valid(v)||nfs_hd_feedback_modal_backdrop_art(nullptr)||!nfs_hd_feedback_modal_backdrop_art_provider_available())okay=false;calls+=2;}});for(auto&t:threads)t.join();CHECK(okay&&calls==16000);CHECK(!nfs_hd_install_feedback_modal_backdrop_art_provider(&p));}
 std::printf("PASS immutable Feedback backdrop provider %s\n",argv[1]);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
