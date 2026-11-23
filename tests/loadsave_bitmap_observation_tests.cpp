// SPDX-License-Identifier: MIT
// Synthetic paint receipts and bitmap observations; no guest/native-pass claim.
#define main existing_loadsave_paint_fixture_main
#include "loadsave_source_policy_tests.cpp"
#undef main
#include "hd/ClassicScreens/LivePacketCore.h"
namespace nfs_ls_live {
class LiveIssuer {
public:
 static std::shared_ptr<const OwnedFrame> issue(const Fixture&f){
  if(!f.valid())return {};
  return std::shared_ptr<const OwnedFrame>(new OwnedFrame(f.packet,f.profile,f.evidence,19));
 }
 static ObservedNow observe(const Fixture&f){
  ObservedNow n;n.authority_=f.current;n.targets_=f.evidence.targetsAfter;n.ticket_=19;n.post_=f.post;
  const auto&t=n.targets_.full;n.surface_.lifetime=t.lifetime;
  std::copy(t.descriptor.begin(),t.descriptor.end(),n.surface_.geometry.begin());
  n.surface_.geometry[14]=t.x[0];n.surface_.geometry[15]=t.x[1];n.surface_.geometry[16]=t.x.back();
  n.surface_.geometry[17]=t.y[0];n.surface_.geometry[18]=t.y[1];n.surface_.geometry[19]=t.y.back();return n;
 }
 static void corrupt(ObservedNow&n,unsigned kind){switch(kind){
  case 0:++n.ticket_;break;case 1:++n.authority_.owner.epoch;break;
  case 2:n.authority_.selected=6;break;case 3:++n.authority_.table;break;
  case 4:n.authority_.rows[3].flags^=1;break;case 5:++n.targets_.full.descriptor[8];break;
  case 6:++n.targets_.selectedOwner;break;case 7:++n.surface_.geometry[8];break;
  case 8:n.post_[0]^=1;break;case 9:n.post_.pop_back();break;
  case 10:++n.targets_.slots[1].lifetime;break;case 11:++n.surface_.lifetime;break;
  default:throw std::runtime_error("Unknown bitmap fault");}}
};
}
int main(){try{
 // The separately registered source-policy test covers its full fault matrix.
 Fixture f;using Issuer=nfs_ls_live::LiveIssuer;
 auto frame=Issuer::issue(f);CHECK(frame);CHECK(frame->matches(Issuer::observe(f)));
 for(unsigned k=0;k<12;++k){auto n=Issuer::observe(f);Issuer::corrupt(n,k);CHECK(!frame->matches(n));}
 // The packet owns its authenticated paint receipt. Later mutable painting
 // inputs cannot rewrite it or become a dependency of unchanged bitmap display.
 f.profile.fontBytes[16]^=1;f.evidence.post.mode=1;f.evidence.post.size=42;
 f.evidence.post.palette=0;f.evidence.post.fontBytes.clear();
 CHECK(frame->matches(Issuer::observe(f)));
 CHECK(!Issuer::issue(f)); // Fresh painting still rejects these invalid inputs.
 std::cout<<"LoadSave bitmap observation: ownership/pixel negatives and sealed paint lifetime PASS\n";return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
