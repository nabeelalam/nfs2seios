// SPDX-License-Identifier: MIT
#include "hd/ClassicScreens/CreditsWholePrepare.h"
#include "hd/ClassicScreens/CreditsArtworkLoader.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>
int main(int argc,char**argv){
 assert(!nfs_credits_ui::PreparedWhole::create({}));
 if(argc!=3){std::puts("Credits whole-frame preparation absent-input check passed; positive cases require private source/font/artwork fixtures");return 0;}
 using namespace nfs_credits_ui;std::ifstream stream(argv[1],std::ios::binary);assert(stream);std::vector<unsigned char>bytes((std::istreambuf_iterator<char>(stream)),{});auto identity=ArchiveIdentity::create(bytes);assert(identity);auto source=SourceLease::create(0x1234,0x10000000,identity);assert(source);
 std::array<StaticDraw,4>draws{};const unsigned recipe[4]={5,8,0,1};for(unsigned i=0;i<4;++i){auto&d=draws[i];d.sourceIndex=recipe[i];d.target[1]=640;d.target[2]=480;d.target[5]=640;d.target[6]=480;d.target[7]=0x7810;d.target[8]=1;d.target[10]=1280;d.target[11]=2;d.target[12]=3;}
 draws[0].x=100;draws[0].y=50;draws[1].x=300;draws[1].y=30;draws[3].x=456;draws[3].y=17;
 auto scene=SceneSource::create(source,0x1234,0x10000000,0,draws);assert(scene);FrameAssembly assembly;assert(assembly.begin(scene));auto empty=assembly.seal();assert(empty);assert(!PreparedWhole::create(empty));
 char error[128];assert(nfs_credits_art_load_file(argv[2],error,sizeof error));auto preparedEmpty=PreparedWhole::create(empty);assert(preparedEmpty);auto emptyView=preparedEmpty->view(1);assert(emptyView.schema==1&&emptyView.request==1&&emptyView.current(emptyView.owner)&&!emptyView.text.vertexCount&&!emptyView.text.vertices&&!emptyView.eventCount&&!emptyView.hasPartner&&emptyView.heading.left==456&&emptyView.heading.top==17);
 // A real original command can be wholly clipped during initial scrolling.
 // It must keep the complete background/heading frame rather than fall back.
 assert(assembly.begin(scene));nfs_soft::TextCommand clipped{};clipped.text[0]='A';clipped.length=1;clipped.sourceSize=18;clipped.advance=12;clipped.x=200;clipped.y=500;clipped.clip={100,0,640,450};assert(assembly.append(clipped,0xffffff));auto clippedFrame=assembly.seal();assert(clippedFrame);auto clippedPrepared=PreparedWhole::create(clippedFrame);assert(clippedPrepared);auto clippedView=clippedPrepared->view(9);assert(clippedView.text.logicalWidth==640&&clippedView.text.logicalHeight==480&&!clippedView.text.vertexCount&&clippedView.eventCount==1&&clippedView.events[0].count==0&&clippedView.current(clippedView.owner));
 assert(assembly.begin(scene));assert(!emptyView.current(emptyView.owner));nfs_soft::TextCommand text{};text.text[0]='A';text.length=1;text.sourceSize=18;text.advance=12;text.x=200;text.y=100;text.clip={100,0,640,450};assert(assembly.append(text,0x11aa33));PartnerPlacement partner;partner.y=200;partner.clip={{100,0,640,450}};assert(assembly.beginPartner(partner)&&assembly.finishPartner());text.text[0]='B';text.y=200;assert(assembly.append(text,0xee4477));auto whole=assembly.seal();assert(whole);auto prepared=PreparedWhole::create(whole);assert(prepared);auto view=prepared->view(2);assert(view.schema==1&&view.locale==0&&view.request==2&&view.eventCount==3&&view.hasPartner);
 assert(view.events[0].kind==1&&view.events[0].first==0&&view.events[0].count>0&&view.events[1].kind==2&&view.events[2].kind==1&&view.events[2].first==view.events[0].count&&view.events[0].count+view.events[2].count==view.text.vertexCount);
 assert(view.partner[0].left==100&&view.partner[0].top==250&&view.partner[0].right==296&&view.partner[0].bottom==288&&view.partner[1].left==300&&view.partner[1].top==230&&view.partner[1].right==543&&view.partner[1].bottom==360&&view.partnerClip.left==100&&view.partnerClip.bottom==450);
 const auto&a=view.text.vertices[0];const auto&b=view.text.vertices[view.events[2].first];assert(a.r==17.f/255&&a.g==170.f/255&&a.b==51.f/255&&a.a==1&&b.r==238.f/255&&b.g==68.f/255&&b.b==119.f/255&&b.a==1);
 // Keep a C renderer lease after the C++ shared handle is dropped.
 view.retain(view.owner);prepared.reset();assert(view.current(view.owner));nfs_credits_art_clear();assert(!view.current(view.owner));const auto&i=view.art.images[0];unsigned char sha[32];assert(nfs_texture_sha256(i.pixels,i.bytes,sha)&&!std::memcmp(sha,i.sha256,32));view.release(view.owner);
 assert(nfs_credits_art_load_file(argv[2],error,sizeof error));auto replacement=PreparedWhole::create(whole);assert(replacement&&replacement->view(3).current(replacement->view(3).owner));assembly.abort();assert(!replacement->view(3).request);
 source->retire();nfs_main_sidebar_origin::retireGame(0x1234);nfs_credits_art_clear();std::puts("Credits whole-frame empty/mixed layers, colors, actual-source geometry, C leases and generation/frame invalidation passed");
}
