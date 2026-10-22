// SPDX-License-Identifier: MIT
#include "HdSceneFixture.h"
#include "hd/AudioSidebarPacket.h"
#include <type_traits>
namespace as=nfs_audio_sidebar;
struct SidebarFixture {
 std::unique_ptr<Fixture>f{new Fixture};as::Binding binding{0xabc123,0x51000,0x200000,0x700000,3,5,29,11};
 std::shared_ptr<const nfs_soft::FramePacket>caption;
 std::shared_ptr<bg::SourceCatalog>catalog;
 std::vector<uint8_t>before,after;uint64_t captionSerial=13,captionLifetime=101;
 SidebarFixture(){f->scene.form=binding.form;catalog=std::make_shared<bg::SourceCatalog>(f->catalog);rebuild();}
 void rebuild(){nfs_soft::FontDescription d;for(auto&role:d.glyphs)for(auto&g:role){g.x=g.y=2;g.w=g.h=1;g.advance=65536;}std::vector<uint8_t>alpha(1024*512);alpha[2*1024+2]=255;auto font=nfs_soft::FontCatalog::create(d,alpha.data(),alpha.size(),17);CHECK(font);nfs_soft::FramePacketBuilder builder(font);auto surface=as::surface(f->group.post.target,captionLifetime);CHECK(builder.begin(surface,captionSerial,f->group.post.pixels.data(),as::Pixels));nfs_soft::TextCommand text;std::memcpy(text.text.data(),"options",7);text.length=7;text.role=1;text.alignment=1;text.sourceSize=18;text.x=122;text.y=364;text.advance=92;text.color=0xffff;text.clip={0,0,343,480};CHECK(builder.beginString(text));auto post=f->group.post.pixels;const unsigned at=369*343+123;CHECK(builder.word(at,post[at],0xffff,0xffff));post[at]=0xffff;CHECK(builder.endString(text));caption=builder.finish(surface,post.data(),as::Pixels);CHECK(caption);before.resize(as::Bytes);for(unsigned k=0;k<4;++k)put32(before.data()+4*k,f->group.post.target.header[k]);for(unsigned k=0;k<as::Pixels;++k)put16(before.data()+16+2*k,post[k]);after=before;}
 std::shared_ptr<const as::Packet>verify(){as::Input i{binding,f->scene,f->group,caption,captionSerial,captionLifetime,before,after};return as::Packet::create(i,binding,f->scene.preSide.target,catalog);}
};
static_assert(!std::is_default_constructible<as::Packet>::value,"packet requires verified construction");
int main(){try{auto x=std::unique_ptr<SidebarFixture>(new SidebarFixture);auto p=x->verify();CHECK(p&&p->wordCount()==1&&p->caption()==x->caption&&p->binding().lifetime!=p->caption()->surface().lifetime);CHECK(p->matches(x->binding,x->f->scene.preSide.target,x->before.data(),x->before.size()));
 for(unsigned k=0;k<as::Pixels;++k)CHECK(logo::le16(p->raw().data()+16+2*k)==(k==369*343+123?0xffff:x->f->group.post.pixels[k]));CHECK(p->logos()[0].left==0&&p->logos()[0].top==0&&p->logos()[0].clip[2]==343);
 unsigned negatives=0;auto reject=[&]{CHECK(!x->verify());++negatives;};
 x->f->scene.postSide.pixels[100]^=1;reject();x->f->scene.postSide.pixels[100]^=1;
 x->f->scene.postWords.pixels[101]^=1;reject();x->f->scene.postWords.pixels[101]^=1;
 x->f->group.pre.pixels[102]^=1;reject();x->f->group.pre.pixels[102]^=1;
 x->f->group.post.pixels[103]^=1;reject();x->f->group.post.pixels[103]^=1;
 x->before[16+2*104]^=1;x->after=x->before;reject();x->before[16+2*104]^=1;x->after=x->before;
 x->after[100]^=1;reject();x->after=x->before;
 x->before[0]^=1;x->after=x->before;reject();x->before[0]^=1;x->after=x->before;
 ++x->captionSerial;reject();--x->captionSerial;++x->captionLifetime;reject();--x->captionLifetime;
 auto cap=x->caption;x->caption.reset();reject();x->caption=cap;
 x->f->scene.form^=1;reject();x->f->scene.form^=1;
 x->f->group.thread++;reject();x->f->group.thread--;
 x->binding.sourceOwner++;reject();x->binding.sourceOwner--;x->binding.bitmap++;reject();x->binding.bitmap--;
 x->binding.archive=0;reject();x->binding.archive=0x700000;x->binding.ownerEpoch=0;reject();x->binding.ownerEpoch=3;x->binding.cacheRevision=0;reject();x->binding.cacheRevision=5;x->binding.lifetime=0;reject();x->binding.lifetime=11;
 x->f->scene.preSide.target.x[123]^=2;reject();x->f->scene.preSide.target.x[123]^=2;
 x->f->scene.preSide.target.ownerWords[8]^=16;reject();x->f->scene.preSide.target.ownerWords[8]^=16;
 x->f->scene.draws[0].paletteAfter[1]^=1;reject();x->f->scene.draws[0].paletteAfter[1]^=1;
 x->f->scene.draws[1].sourceAfter[16]^=1;reject();x->f->scene.draws[1].sourceAfter[16]^=1;
 x->f->group.draw[0].paletteLast[1]^=1;reject();x->f->group.draw[0].paletteLast[1]^=1;
 x->catalog->identities[0].sha[0]^=1;reject();x->catalog->identities[0].sha[0]^=1;
 as::Input input{x->binding,x->f->scene,x->f->group,x->caption,x->captionSerial,x->captionLifetime,x->before,x->after};
 auto current=x->binding;current.archive++;CHECK(!as::Packet::create(input,current,x->f->scene.preSide.target,x->catalog));current=x->binding;current.ownerEpoch++;CHECK(!as::Packet::create(input,current,x->f->scene.preSide.target,x->catalog));current=x->binding;current.cacheRevision++;CHECK(!as::Packet::create(input,current,x->f->scene.preSide.target,x->catalog));current=x->binding;current.lifetime++;CHECK(!as::Packet::create(input,current,x->f->scene.preSide.target,x->catalog));
 auto flipped=x->f->scene.preSide.target;flipped.ownerState[0]^=0x10000;CHECK(p->matches(x->binding,flipped,x->before.data(),x->before.size()));
 for(unsigned k=1;k<5;++k){auto t=flipped;t.ownerState[k]^=4;CHECK(!p->matches(x->binding,t,x->before.data(),x->before.size()));}
 for(unsigned k=0;k<4;++k){auto t=flipped;t.header[k]^=1;CHECK(!p->matches(x->binding,t,x->before.data(),x->before.size()));}
 {auto t=flipped;t.w[8]^=16;CHECK(!p->matches(x->binding,t,x->before.data(),x->before.size()));t=flipped;t.x[123]^=2;CHECK(!p->matches(x->binding,t,x->before.data(),x->before.size()));t=flipped;t.y[234]^=2;CHECK(!p->matches(x->binding,t,x->before.data(),x->before.size()));}
 auto retiredTarget=x->f->scene.preSide.target;retiredTarget.ownerWords[11]^=4;CHECK(!p->matches(x->binding,retiredTarget,x->before.data(),x->before.size()));
 CHECK(x->verify());auto raw=p->raw();x->before[16]^=1;CHECK(p->raw()==raw);x->catalog->roles[0]=12;CHECK(p->catalog()->roles[0]==0);x.reset();CHECK(p->caption()&&p->raw()==raw&&p->catalog()->roles[0]==0);
 auto mismatch=std::unique_ptr<SidebarFixture>(new SidebarFixture);mismatch->f->group.post.pixels[300]^=1;mismatch->rebuild();mismatch->f->group.post.pixels[300]^=1;CHECK(!mismatch->verify());
 // All supported word counts: zero and four need no invented Main event epoch.
 auto z=std::unique_ptr<SidebarFixture>(new SidebarFixture);z->f->scene.draws.resize(1);z->f->scene.postWords=z->f->scene.postSide;z->f->group.pre=z->f->scene.postWords;z->rebuild();CHECK(z->verify()->wordCount()==0);
 auto q=std::unique_ptr<SidebarFixture>(new SidebarFixture);while(q->f->scene.draws.size()<5)q->f->scene.draws.push_back(q->f->scene.draws[1]);CHECK(q->verify()->wordCount()==4);q->f->scene.draws.push_back(q->f->scene.draws[1]);CHECK(!q->verify());
 // Existing API still requires a genuine full640 Copy; group-only verifier does not.
 auto old=std::unique_ptr<Fixture>(new Fixture);CHECK(old->verify()==bg::Failure::None);CHECK(logo::verifyGroup(old->group,old->workspace.logo,old->catalog.logos)==logo::Failure::None);old->copy.rows=0;CHECK(old->verify()==bg::Failure::Logos);CHECK(logo::verifyGroup(old->group,old->workspace.logo,old->catalog.logos)==logo::Failure::None);old->group.post.pixels[200]^=1;CHECK(logo::verifyGroup(old->group,old->workspace.logo,old->catalog.logos)==logo::Failure::GroupPixels);CHECK(old->verify()==bg::Failure::Logos);
 std::printf("PASS Audio immutable343cache full164640pixel oracle, %u mutation cases, zero/fourwords, independent caption lifetime, group-only/legacy Copy regression\n",negatives);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
