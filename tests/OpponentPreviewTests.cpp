// SPDX-License-Identifier: MIT
#include "HdSceneFixture.h"
#include "OpponentPreviewFixture.h"
int main(){try{auto f=std::unique_ptr<Fixture>(new Fixture);using namespace nfs_opponent_preview_test;unsigned cases=0;
 auto original=std::unique_ptr<nfs_opponent_preview_test::Fixture>(new nfs_opponent_preview_test::Fixture(f->base.post));
 for(unsigned i=0;i<15;++i){original->input.selection=i;std::memcpy(original->input.draws[0].name.data(),nfs_opponent_preview_source::nameFor(i),4);auto packet=original->packet(f->base.post);CHECK(packet);CHECK(packet->post()==original->oracle);CHECK(packet->selection()==i);CHECK(!packet->sourceAuthenticated());++cases;}
 auto valid=original->input;
 for(unsigned i=0;i<8;++i){original->input=valid;auto&v=original->input;if(i==0)v.form=0x404abd;if(i==1)v.selection=15;if(i==2)v.draws[0].name[3]='z';if(i==3)v.draws[0].args[2]++;if(i==4)v.draws[0].rows[0].after[0]^=1;if(i==5)v.draws[0].sourceAfter[16]^=1;if(i==6)v.draws[0].paletteAfter[0]^=1;if(i==7)v.draws.push_back(v.draws[0]);CHECK(!original->packet(f->base.post));++cases;}
 original->input=valid;original->input.draws.clear();original->input.drawCalls=original->input.fillCalls=0;original->input.post=original->input.pre;auto empty=nfs_opponent_preview::Packet::createEmpty(original->input,f->base.post);CHECK(empty&&empty->selection()==15&&empty->sources().empty());CHECK(empty->view().coverageCount==1&&empty->view().coverage[0]==0);CHECK(empty->post()==std::vector<uint16_t>(f->base.post.pixels.begin(),f->base.post.pixels.end()));original->input.post.pixels[123]^=1;CHECK(!nfs_opponent_preview::Packet::createEmpty(original->input,f->base.post));++cases;
 std::printf("PASS Opponents original preview: %u selector/replay/negative cases; optional art absent preserves replay\n",cases);return 0;}catch(const std::exception&e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
