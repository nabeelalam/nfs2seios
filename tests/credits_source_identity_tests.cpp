// SPDX-License-Identifier: MIT
#include "hd/ClassicScreens/CreditsSourceIdentity.h"
#include "hd/ClassicScreens/CreditsSceneSource.h"
#include "hd/ClassicScreens/CreditsFullFrame.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <thread>
using namespace nfs_credits_ui;
int main(int argc,char**argv){
 assert(!ArchiveIdentity::create({}));
 assert(!ArchiveIdentity::create(std::vector<unsigned char>(NFS_CREDITS_ARCHIVE_BYTES)));
 assert(!ArchiveIdentity::create(std::vector<unsigned char>(NFS_CREDITS_ARCHIVE_BYTES-1)));
 assert(!SourceLease::create(1,1,{}));
 if(argc!=2){std::puts("Credits rejection checks passed; positive original-data checks require a private fixture");return 0;}
 std::ifstream stream(argv[1],std::ios::binary);assert(stream);
 std::vector<unsigned char>bytes((std::istreambuf_iterator<char>(stream)),{});
 auto identity=ArchiveIdentity::create(bytes);assert(identity);
 assert(!SourceLease::create(0,0x10000000,identity));
 assert(!SourceLease::create(1,0,identity));
 assert(!SourceLease::create(1,0xffff0000,identity));
 auto lease=SourceLease::create(0x1234,0x10000000,identity);assert(lease&&lease->current(0x1234,0x10000000));
 std::array<StaticDraw,4>draws{};const unsigned recipe[4]={5,8,0,1};
 for(unsigned i=0;i<4;++i){draws[i].sourceIndex=recipe[i];auto&t=draws[i].target;t[1]=640;t[2]=480;t[5]=640;t[6]=480;t[7]=0x7810;t[8]=0x20000000;t[10]=1280;t[11]=0x21000000;t[12]=0x22000000;}
 draws[0].x=100;draws[0].y=50;draws[1].x=300;draws[1].y=30;draws[3].x=456;draws[3].y=17;
 // Geometry here is synthetic; this does not prove live game placement.
 auto scene=SceneSource::create(lease,0x1234,0x10000000,0,draws);assert(scene&&scene->current()&&scene->locale()==0&&scene->draws()[3].x==456);
 assert(!SceneSource::create(lease,0x1234,0x10000000,1,draws));
 auto changed=draws;changed[0].sourceIndex=8;assert(!SceneSource::create(lease,0x1234,0x10000000,0,changed));
 changed=draws;changed[2].x=1;assert(!SceneSource::create(lease,0x1234,0x10000000,0,changed));
 changed=draws;changed[1].target[10]=640;assert(!SceneSource::create(lease,0x1234,0x10000000,0,changed));
 changed=draws;changed[1].y=4097;assert(!SceneSource::create(lease,0x1234,0x10000000,0,changed));
 FrameAssembly assembly;assert(assembly.begin(scene));auto empty=assembly.seal();assert(empty&&empty->current()&&empty->commands().empty()&&empty->events().empty()&&!empty->partner());
 assert(assembly.begin(scene)&&!empty->current());nfs_soft::TextCommand command{};command.text[0]='A';command.length=1;command.sourceSize=18;command.advance=10;command.clip={0,0,640,480};
 assert(assembly.append(command,0x123456));PartnerPlacement partner;partner.y=200;partner.clip={{100,0,640,450}};assert(assembly.beginPartner(partner));assert(assembly.finishPartner());command.text[0]='B';assert(assembly.append(command,0xabcdef));auto ordered=assembly.seal();
 assert(ordered&&ordered->current()&&ordered->commands().size()==2&&ordered->events().size()==3&&ordered->events()[0].kind==FrameEvent::Text&&ordered->events()[1].kind==FrameEvent::Partners&&ordered->events()[2].kind==FrameEvent::Text&&ordered->events()[2].index==1);
 assert(ordered->colors()[0]==0xff123456&&ordered->commands()[0].text[0]=='A'&&ordered->commands()[1].text[0]=='B'&&ordered->partner()->y==200&&ordered->partner()->clip[0]==100);
 assert(assembly.begin(scene)&&!ordered->current());assert(assembly.beginPartner(partner));assert(!assembly.seal());
 assert(assembly.begin(scene));assert(assembly.beginPartner(partner)&&assembly.finishPartner());assert(!assembly.beginPartner(partner)&&!assembly.seal());
 assert(assembly.begin(scene));partner.y=450;assert(!assembly.beginPartner(partner));partner.y=200;
 assert(assembly.begin(scene));partner.clip[0]=-1;assert(!assembly.beginPartner(partner));partner.clip[0]=100;
 assert(assembly.begin(scene));command.advance=0;assert(!assembly.append(command,0));command.advance=10;
 assert(assembly.begin(scene));assert(assembly.beginPartner(partner));assert(!assembly.append(command,0));
 assert(assembly.begin(scene));assert(assembly.beginPartner(partner)&&assembly.finishPartner());auto noInkPartner=assembly.seal();assert(noInkPartner&&noInkPartner->commands().empty()&&noInkPartner->events().size()==1);assembly.abort();assert(!noInkPartner->current());
 std::shared_ptr<const WholeFrame>escaped;{FrameAssembly temporary;assert(temporary.begin(scene));escaped=temporary.seal();assert(escaped->current());}assert(!escaped->current());
 assert(assembly.begin(scene));auto beforeRetire=assembly.seal();assert(beforeRetire->current());
 assert(!lease->current(0x1235,0x10000000)&&!lease->current(0x1234,0x10000001));
 for(unsigned offset:{0u,4u,8u,12u,16u,20u,88u,100u,614503u,842383u}){
  bytes[offset]^=1;assert(!ArchiveIdentity::create(bytes));bytes[offset]^=1;
 }
 for(unsigned i=0;i<NFS_CREDITS_RESOURCE_COUNT;++i){auto offset=nfs_credits_sources[i].offset+16;bytes[offset]^=1;assert(!ArchiveIdentity::create(bytes));bytes[offset]^=1;}
 // Neither the archive bytes nor the owning snapshot must remain allocated.
 bytes.clear();bytes.shrink_to_fit();assert(lease->current(0x1234,0x10000000));
 std::thread worker([lease]{lease->retire();});worker.join();assert(!scene->current()&&!beforeRetire->current());assert(!lease->current(0x1234,0x10000000));
 auto next=SourceLease::create(0x1234,0x10000000,identity);assert(next&&next->current(0x1234,0x10000000));
 assert(!lease->current(0x1234,0x10000000));
 std::thread destroy([]{nfs_main_sidebar_origin::retireGame(0x1234);});destroy.join();
 assert(!next->current(0x1234,0x10000000));
 auto reused=SourceLease::create(0x1234,0x10000000,identity);assert(reused&&reused->current(0x1234,0x10000000));
 assert(!next->current(0x1234,0x10000000));reused->retire();nfs_main_sidebar_origin::retireGame(0x1234);
 std::puts("Credits original identity, static recipe, ordered complete/empty frames and source/frame retirement passed");
}
