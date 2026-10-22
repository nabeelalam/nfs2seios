// SPDX-License-Identifier: MIT
#include "hd/SoftwareFontBuiltin.h"
#include "hd/SoftwareTextLayout.h"
#include "hd/ArtworkActivation.h"
#include "hd/BackgroundSceneCatalog.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
using namespace nfs_soft;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
std::shared_ptr<const FontCatalog> acquireFontFromPeer();
static void catalogTests(){
 auto font=createBuiltinFontCatalog();CHECK(font&&font->alpha().size()==1024u*512&&font->description().baseline[0]==23&&font->description().baseline[1]==20);
 CHECK(authenticatedFontMetadata(font->description()));
 const uint8_t word[]={'l','o','c','a','t','i','o','n',0};Layout shape;CHECK(layout(*font,word,sizeof word,0,116,162,66,0,{0,0,343,480},shape)&&shape.length==8&&shape.count==8);
 std::vector<uint8_t>decoded;CHECK(decodeFontCoverage(nfs_soft_authored_data::coverageRle,sizeof nfs_soft_authored_data::coverageRle,decoded));CHECK(decoded==font->alpha());
 // Mutable source buffers and description cannot alter an already copied catalog.
 auto data=builtinFontDescription();auto rle=std::vector<uint8_t>(std::begin(nfs_soft_authored_data::coverageRle),std::end(nfs_soft_authored_data::coverageRle));auto owned=createAuthenticatedSoftwareFont(data,rle.data(),rle.size());CHECK(owned);const auto pristine=owned->alpha();data.glyphs[0][33].advance++;rle[2]^=1;CHECK(owned->alpha()==pristine&&authenticatedFontMetadata(owned->description()));
 CHECK(FontCatalog::create(data,decoded.data(),decoded.size(),nfs_soft_authored_data::identity));CHECK(!createAuthenticatedSoftwareFont(data,nfs_soft_authored_data::coverageRle,sizeof nfs_soft_authored_data::coverageRle));CHECK(!createAuthenticatedSoftwareFont(builtinFontDescription(),rle.data(),rle.size()));
 auto untouched=std::vector<uint8_t>{1,2,3};const auto before=untouched;const uint8_t zero[]={0,0,1},shortRun[]={1,0,0};CHECK(!decodeFontCoverage(nullptr,3,untouched));CHECK(!decodeFontCoverage(zero,3,untouched));CHECK(!decodeFontCoverage(shortRun,3,untouched));CHECK(!decodeFontCoverage(zero,2,untouched));CHECK(!decodeFontCoverage(zero,1024u*512*3+3,untouched));CHECK(untouched==before);
 auto overflow=std::vector<uint8_t>(27);for(size_t i=0;i<overflow.size();i+=3){overflow[i]=255;overflow[i+1]=255;}CHECK(!decodeFontCoverage(overflow.data(),overflow.size(),untouched));CHECK(untouched==before);
 auto bad=builtinFontDescription();bad.baseline[0]++;CHECK(!authenticatedFontMetadata(bad));bad=builtinFontDescription();bad.kern[1][0]++;CHECK(!authenticatedFontMetadata(bad));bad=builtinFontDescription();bad.glyphs[1][95].top++;CHECK(!authenticatedFontMetadata(bad));
}
static void providerTests(){
 CHECK(!acquireFontCatalog());CHECK(!installFontCatalog({}));
 auto first=createBuiltinFontCatalog(),replacement=createBuiltinFontCatalog();CHECK(first&&replacement);std::weak_ptr<const FontCatalog>old=first;
 CHECK(installFontCatalog(first));first.reset();CHECK(!old.expired());CHECK(installFontCatalog(replacement));CHECK(old.expired());
 auto acquired=acquireFontFromPeer();CHECK(acquired==replacement&&acquireFontCatalog()==acquired);CHECK(!installFontCatalog(createBuiltinFontCatalog()));CHECK(!installFontCatalog(acquired));
 std::atomic<unsigned>success{0};std::vector<std::thread>threads;for(unsigned i=0;i<8;++i)threads.emplace_back([&]{for(unsigned k=0;k<64;++k)if(acquireFontFromPeer()==acquired)++success;});for(auto&t:threads)t.join();CHECK(success==512);
 std::weak_ptr<const FontCatalog>retained=acquired;replacement.reset();acquired.reset();CHECK(!retained.expired()&&acquireFontCatalog()->alpha().size()==1024u*512);
}
int main(int argc,char**argv){
 CHECK(argc==2);const std::string mode=argv[1];
 if(mode=="catalog")catalogTests();
 else if(mode=="provider")providerTests();
 else if(mode=="startup"){
  CHECK(!acquireFontCatalog());CHECK(!nfs_hd::activateConfiguredArtwork(""));auto owner=acquireFontCatalog();CHECK(owner&&authenticatedFontMetadata(owner->description()));CHECK(!nfs_hd_art_provider_available()&&!nfs_background_scene_runtime::catalogConfigured()&&!std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_HD"));CHECK(!nfs_hd::activateConfiguredArtwork(""));CHECK(acquireFontCatalog()==owner);
 }else CHECK(false);
 std::printf("PASS authored-font-%s checks=%u\n",mode.c_str(),checks);
}
