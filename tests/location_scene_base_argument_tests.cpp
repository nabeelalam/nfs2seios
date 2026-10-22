// SPDX-License-Identifier: MIT
// Reuse the existing synthetic guest-memory/actual-header fixture; no engine assets.
#define main existing_scene_runtime_fixture_main
#include "hd_scene_runtime_tests.cpp"
#undef main
int main(){try{
 setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD","1",1);auto f=std::unique_ptr<Fixture>(new Fixture);CHECK(nfs_background_scene_runtime::installCatalog(std::make_shared<const bg::SourceCatalog>(f->catalog)));prepareMemory(*f);
 m32(off_4D2A20+4,0x7800);std::memcpy(memory.data()+aBlk4,"blk4",5);CHECK(aBlk4!=0x7800);
 using Mode=nfs_location_scene_live::BaseArgument;unsigned cases=0;
 auto check=[&](Mode mode,uint32_t argument,bool expected){
  Game g;g.eax=off_4D2A20;nfs_logo_publisher::LocationMenuOwnerScope menu(g);nfs_location_scene_live::LocationScope owner(g);auto*s=nfs_location_scene_live::current(g);CHECK(s&&!s->invalid);
  g.eax=argument;g.edx=0x400;{nfs_location_scene_live::RoleScope base(g,nfs_location_scene_live::Phase::Base,mode);CHECK(bool(!s->invalid)==expected);CHECK(leaseDepth==0);}++cases;
 };
 check(Mode::Descriptor,0x7800,true);check(Mode::Blk4Direct,aBlk4,true);
 check(Mode::Descriptor,aBlk4,false);check(Mode::Blk4Direct,0x7800,false);
 memory[0x7800]^=1;check(Mode::Blk4Direct,aBlk4,false);memory[0x7800]^=1;
 memory[aBlk4]^=1;check(Mode::Blk4Direct,aBlk4,false);memory[aBlk4]^=1;
 CHECK(allocationInLease==0);std::printf("PASS %u actual Location base argument cases, two distinct pointers, no lease allocations\n",cases);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
