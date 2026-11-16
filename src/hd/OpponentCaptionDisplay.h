// SPDX-License-Identifier: MIT
#pragma once
#include "OpponentCaptionProof.h"
namespace nfs_opponent_caption_display {
struct Binding {uint32_t formGuest=0,archive=0;uintptr_t game=0;uint64_t serial=0,lifetime=0;};
inline bool matches(const nfs_opponent_caption_source::Proof&p,const Binding&b)noexcept{
 const auto&e=p.evidence();const auto&packet=p.packet();
 if(!p.current()||!packet||!b.formGuest||!b.archive||!b.game||!b.serial||!b.lifetime||e.policy!=nfs_opponent_caption_source::Policy::Opponents||e.formGuest!=b.formGuest||e.game!=b.game||e.serial!=b.serial||e.lifetime!=b.lifetime||packet->serial()!=b.serial||packet->surface().lifetime!=b.lifetime||packet->pixels()!=343*480||packet->commands()!=1||!packet->writes()||!packet->font())return false;
 if(e.requestedResource!=4||e.inputCategory!=1||e.inputAnchor!=168||e.inputY!=271||e.measureWidth!=68||e.measureAnchor!=168||e.measureKind!=2)return false;
 const auto&c=e.command;if(c.role!=1||c.sourceSize!=18||c.alignment!=2||c.color!=0xffff||c.x!=100||c.y!=271||c.advance!=68||c.length!=9||std::memcmp(c.text.data(),"opponents",9))return false;
 for(const auto*s:{&e.resolved,&e.measure,&e.stringBefore,&e.stringAfter})if(s->archive!=b.archive||s->selector!=4||s->resource!=4||s->identity.lifetime!=b.lifetime)return false;
 if(e.route==nfs_opponent_caption_source::Route::OrdinaryEbp)return e.incomingEbp==b.formGuest&&!p.origin();
 return e.route==nfs_opponent_caption_source::Route::InitialMain&&p.origin()&&p.origin()->current()&&(e.incomingEbp==p.origin()->formGuest()||e.incomingEbp==0||e.incomingEbp==UINT32_MAX||e.incomingEbp==UINT32_MAX-1);
}
}
