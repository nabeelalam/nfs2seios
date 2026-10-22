// SPDX-License-Identifier: MIT
#pragma once
#include "HdSceneFixture.h"
#include "hd/Main343OriginProof.h"
namespace main343_origin_fixture {
namespace origin=nfs_main_sidebar_origin;
struct Fixture: ::Fixture {
 origin::Binding binding{0x7000,29,0x123456,0x405f61,1};
 // Retarget every original descriptor/LUT/row/copy argument together. Pixel
 // payloads and events remain the existing independently verified Main fixture.
 void retarget343(uint32_t owner,uint32_t bitmap,uint32_t xTable,uint32_t yTable){
  const uint32_t old=copy.sourceOwner.w[8];
  auto target=[&](rec::Target&t){t.ownerState[1]=owner;if(t.w[1]!=343)return;t.owner=owner;t.w[8]=bitmap;t.w[11]=yTable;t.w[12]=xTable;t.ownerWords=t.w;};
  auto plane=[&](rec::Plane&p){target(p.target);};
  plane(base.post);plane(scene.preSide);plane(scene.postSide);plane(scene.postWords);plane(scene.tailPre);plane(scene.tailPost);plane(group.pre);plane(group.post);plane(copy.pre);plane(copy.post);target(copy.sourceOwner);
  for(auto&d:scene.draws){target(d.before);target(d.after);for(auto&r:d.rows)r.args[1]=bitmap+(r.args[1]-old);}
  for(unsigned k=0;k<group.draws;++k){auto&d=group.draw[k];target(d.before);target(d.after);for(unsigned j=0;j<d.rows;++j)d.row[j].args[1]=bitmap+(d.row[j].args[1]-old);}
  copy.args[0]=bitmap;
 }
 Fixture(){retarget343(0x1800,0x40000,0x3000,0x2000);}
 bg::JoinedInput input()const{return {base,scene,group,copy,{1,3,1},{1,4,1},{1,5,1},{1,6,1},copy.post.target,1,1,keys};}
 std::shared_ptr<const bg::SourceCatalog>trusted()const{return std::make_shared<const bg::SourceCatalog>(catalog);}
 std::shared_ptr<const origin::Proof>publish(origin::Store&store){return store.publish(input(),binding,workspace,trusted());}
};
}
