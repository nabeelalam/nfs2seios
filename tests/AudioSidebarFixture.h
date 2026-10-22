#pragma once
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
