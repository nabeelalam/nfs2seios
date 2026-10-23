// SPDX-License-Identifier: MIT
// Synthetic packet/evidence only. This TU deliberately defines the collector's
// friend name as a test-only sealer; it never includes or executes guest hooks.
#define main controlsPureProofFixtureUnusedMain
#include "controls_dialog_word_proof_tests.cpp"
#undef main
#include "hd/ControlsDialogWordDisplay.h"
namespace nfs_controls_text {
struct TailScope {static std::shared_ptr<const word::Proof>synthetic(Packet p,const word::Evidence&e){return std::shared_ptr<const word::Proof>(new word::Proof(std::move(p),e));}};
}
namespace display=nfs_controls_dialog_display;
static display::Binding binding(const word::Evidence&e){return {e.form,e.authority.archive,e.game,e.serial,e.lifetime};}
static std::shared_ptr<const word::Proof>seal(Packet p,const word::Evidence&e){return nfs_controls_text::TailScope::synthetic(std::move(p),e);}
static void positive(unsigned selected){
 auto e=evidence(selected);auto p=packet(commands(selected));auto proof=seal(p,e);const auto original=p->original();const auto clean=p->clean();const auto vertices=p->vertices();const auto records=p->records();const auto b=binding(e);
 auto d=display::Prepared::create(proof,p,b);CHECK(d&&d->current()&&d->packet()==p&&d->proof()==proof);CHECK(d->binding().form==e.form&&d->binding().archive==e.authority.archive&&d->binding().game==e.game&&d->binding().serial==e.serial&&d->binding().lifetime==e.lifetime);
 const auto v=d->view();CHECK(v.schema==1&&v.count==10);const unsigned widths[]={272,144,492,384,140};
 for(unsigned i=0;i<5;++i){const auto&image=v.images[i];auto art=display::artwork(i+1);CHECK(art&&image.word==i+1&&image.width==widths[i]&&image.height==128&&image.bytes==size_t(widths[i])*128*4&&image.rgba==art->rgba.data()&&!std::memcmp(image.sha256,art->sha.data(),32));}
 const unsigned words[]={2,2,3,3,4,4,5,5,1,1},sourceRecords[]={0,1,4,5,8,9,12,13,14,15};const int widthsSource[]={36,123,96,35,68};
 for(unsigned n=0;n<10;++n){const auto&r=records[sourceRecords[n]];const auto&a=v.placements[n];CHECK(a.word==words[n]&&a.color565==r.command.color&&a.vertexFirst==r.vertexFirst&&a.vertexCount==r.vertexCount&&a.left==r.command.x&&a.top==r.command.y&&a.right==r.command.x+widthsSource[n/2]&&a.bottom==r.command.y+32);}
 // Ellipses are six separate original spans, excluded from all substitutions.
 for(unsigned index:{2u,3u,6u,7u,10u,11u}){const auto&r=records[index];CHECK(word::exactText(r.command,"...")&&r.vertexCount==18);for(const auto&a:v.placements)CHECK(a.vertexFirst+a.vertexCount<=r.vertexFirst||r.vertexFirst+r.vertexCount<=a.vertexFirst);}
 CHECK(p->original()==original&&p->clean()==clean&&p->vertices().size()==vertices.size()&&!std::memcmp(p->vertices().data(),vertices.data(),vertices.size()*sizeof(vertices[0])));
 // External evidence and handles can change without mutating the retained view.
 e.authority.selectedIndex=100;e.post.fontHeader[0]^=1;proof.reset();p.reset();CHECK(d->current());const auto again=d->view();CHECK(again.count==10&&again.images[0].rgba==v.images[0].rgba&&again.placements[0].color565==v.placements[0].color565);
 const unsigned char titleSha[32]={23,118,242,193,91,89,54,181,159,68,165,52,110,208,82,65,73,2,107,74,31,143,139,102,106,180,0,188,109,179,31,83};CHECK(!std::memcmp(again.images[0].sha256,titleSha,32));
}
static void negatives(){auto e=evidence(0);auto p=packet(commands(0));auto proof=seal(p,e);const auto b=binding(e);CHECK(!display::Prepared::create({},p,b)&&!display::Prepared::create(proof,{},b));
 for(unsigned n=0;n<10;++n){auto bad=b;switch(n){case 0:bad.form=0;break;case 1:bad.archive=0;break;case 2:bad.game=0;break;case 3:bad.serial=0;break;case 4:bad.lifetime=0;break;case 5:++bad.form;break;case 6:++bad.archive;break;case 7:++bad.game;break;case 8:++bad.serial;break;case 9:++bad.lifetime;break;}CHECK(!display::Prepared::create(proof,p,bad));}
 CHECK(!display::Prepared::create(proof,packet(commands(0)),b));CHECK(!display::Prepared::create(proof,packet(commands(0),2),b));
 for(unsigned n=0;n<8;++n){auto bad=e;switch(n){case 0:bad.serial++;break;case 1:bad.lifetime++;break;case 2:bad.authority.descriptorWords[1][0]^=1;break;case 3:bad.authority.selectedIndex=100;break;case 4:bad.post.fontHeader[0]++;break;case 5:bad.resources[4].measuredWidth++;break;case 6:bad.resources[0].after.palette++;break;case 7:bad.form++;break;}CHECK(!display::Prepared::create(seal(p,bad),p,b));}
 for(unsigned n=0;n<6;++n){auto c=commands(0);if(n==0)std::swap(c[0],c[4]);if(n==1)c[0].x++;if(n==2)c[15].advance++;if(n==3)c[2].text[0]=':';if(n==4)c[3].x++;if(n==5)c[14].color=65535;auto bad=packet(c);CHECK(bad&&!display::Prepared::create(seal(bad,e),bad,b));}
 CHECK(display::Prepared::create(proof,p,b));
}
int main(){for(unsigned i=0;i<4;++i)positive(i);negatives();std::printf("PASS %u synthetic Controls display binding/ordering/ellipsis/retention checks; no genuine source or native consumer execution claimed\n",checks);return 0;}
