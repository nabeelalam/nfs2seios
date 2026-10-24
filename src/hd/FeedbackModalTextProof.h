// SPDX-License-Identifier: MIT
#pragma once
#include "FeedbackModalTextRecords.h"
#include "FeedbackModalBackdropReplay.h"
namespace nfs_feedback_modal_text_live {struct ModalScope;}
namespace nfs_feedback_modal_text {
inline bool snapshotValid(const Snapshot&s,const Evidence&e)noexcept{
 if(!(s.identity==e.binding.surface)||!nfs_feedback_backdrop::targetValid(s.target,e.binding)||!nfs_logo_capture::same(s.target,e.post.target)||s.archive!=e.controlsArchive||!s.font||!s.palette||s.callback!=e.source.glyph||s.mode||s.flags||s.format!=16||s.size!=18||s.originX||s.originY||s.color>65535)return false;
 return s.font==e.post.font&&s.fontHeader==e.post.fontHeader;
}
inline bool authorityValid(const Evidence&e)noexcept{
 nfs_controls_dialog_word::Evidence parent;parent.source=e.source.controls;parent.form=e.source.controls.form;parent.authority=e.authority;
 return e.source.valid()&&e.binding.valid()&&e.binding.form==e.source.controls.form&&e.binding.parent==e.source.controls.parentMenu&&e.controlsArchive&&e.authority.archive==e.controlsArchive&&e.authority.selectedIndex==2&&nfs_controls_dialog_word::authorityValid(parent)&&e.game&&e.thread&&e.serial&&e.caller==e.source.caller&&e.modal==e.source.modal&&e.count==2&&!e.feedbackAvailable;
}
inline bool resourceValid(const Resource&r,unsigned i,const Evidence&e)noexcept{
 if(i>1||r.id!=resourceId(i)||r.table!=e.authority.table||!r.headerAddress||uint64_t(r.table)+12*r.id!=r.headerAddress||uint64_t(r.headerAddress)+12>0x100000000ULL||r.headerBefore!=r.headerAfter||r.textBefore!=r.textAfter||!exactBytes(r.textBefore,i)||r.textAddress!=(i?e.binding.messageResource:e.binding.titleResource)||uint64_t(r.table)+nfs_controls_dialog_word::le32(r.headerBefore.data()+8)!=r.textAddress||r.headerBefore[0]!=18||r.alignment!=2||r.colorIndex!=(i?2u:0u)||r.anchor!=anchor(i)||r.y!=top(i)||r.measuredWidth!=int(width(i))||r.resolves!=1||r.measures!=1||r.strings!=1)return false;
 if(i==0&&(r.textAddress!=e.authority.texts[2]||r.headerBefore!=e.authority.headers[2]))return false;
 // Header positions/policy belong to the resource's original menu use. Only the
 // explicitly observed modal draw policy above determines popup placement.
 for(const auto*s:{&r.before,&r.resolved,&r.measured,&r.stringBefore,&r.after})if(!snapshotValid(*s,e))return false;
 // Font selection and colour selection are genuine original state changes.
 // All phases retain target/context/font identity; only the actual string
 // interval must retain its selected palette and colour exactly.
 return nfs_controls_dialog_word::sameSnapshot(r.stringBefore,r.after)&&r.after.color==color(i);
}
inline bool consistent(const std::shared_ptr<const nfs_soft::FramePacket>&p,const Evidence&e,const std::shared_ptr<const nfs_feedback_backdrop::VerifiedBackdrop>&b)noexcept{
 if(!p||!b||!authorityValid(e)||!(e.binding==b->binding())||p->serial()!=e.serial||!(p->surface()==e.binding.surface)||p->pixels()!=nfs_feedback_backdrop::Pixels||p->clean()!=b->pretext()||!p->font()||!e.fontIdentity||p->font()->identity()!=e.fontIdentity||!p->writes()||p->commands()!=2||p->records().size()!=2||!snapshotValid(e.post,e)||e.post.color!=65535||!nfs_controls_dialog_word::sameSnapshot(e.post,e.resources[1].after))return false;
 size_t end=0;for(unsigned i=0;i<2;++i){const auto&r=p->records()[i];if(!resourceValid(e.resources[i],i,e)||!command(r.command,i)||r.vertexFirst!=end||!r.vertexCount||r.vertexCount%6)return false;end+=r.vertexCount;if(end>p->vertices().size())return false;}
 return end==p->vertices().size();
}
inline bool sameResource(const Resource&a,const Resource&b)noexcept{return a.id==b.id&&a.table==b.table&&a.headerAddress==b.headerAddress&&a.textAddress==b.textAddress&&a.headerBefore==b.headerBefore&&a.headerAfter==b.headerAfter&&a.textBefore==b.textBefore&&a.textAfter==b.textAfter&&a.alignment==b.alignment&&a.colorIndex==b.colorIndex&&a.anchor==b.anchor&&a.y==b.y&&a.measuredWidth==b.measuredWidth&&a.resolves==b.resolves&&a.measures==b.measures&&a.strings==b.strings&&nfs_controls_dialog_word::sameSnapshot(a.before,b.before)&&nfs_controls_dialog_word::sameSnapshot(a.resolved,b.resolved)&&nfs_controls_dialog_word::sameSnapshot(a.measured,b.measured)&&nfs_controls_dialog_word::sameSnapshot(a.stringBefore,b.stringBefore)&&nfs_controls_dialog_word::sameSnapshot(a.after,b.after);}
inline bool matchesCurrent(const std::shared_ptr<const nfs_soft::FramePacket>&p,const Evidence&e,const std::shared_ptr<const nfs_feedback_backdrop::VerifiedBackdrop>&b,const Current&c)noexcept{
 const auto&o=c.observed;if(!consistent(p,e,b)||!consistent(c.packet,o,b)||c.packet!=p||!(o.source==e.source)||!(o.binding==e.binding)||o.game!=e.game||o.thread!=e.thread||o.serial!=e.serial||o.fontIdentity!=e.fontIdentity||o.caller!=e.caller||o.modal!=e.modal||o.count!=e.count||o.feedbackAvailable!=e.feedbackAvailable||o.controlsArchive!=e.controlsArchive||!(o.authority==e.authority)||!nfs_controls_dialog_word::sameSnapshot(o.post,e.post))return false;
 for(unsigned i=0;i<2;++i)if(!sameResource(o.resources[i],e.resources[i]))return false;
 return p->matches(o.binding.surface,o.serial,c.original565,c.planeWords);
}
// Pure consistency result. Deliberately cannot be converted into a source Proof.
class CheckedText final {
 std::shared_ptr<const nfs_soft::FramePacket>packet_;std::shared_ptr<const nfs_feedback_backdrop::VerifiedBackdrop>backdrop_;Evidence evidence_;
 CheckedText(std::shared_ptr<const nfs_soft::FramePacket>p,std::shared_ptr<const nfs_feedback_backdrop::VerifiedBackdrop>b,const Evidence&e):packet_(std::move(p)),backdrop_(std::move(b)),evidence_(e){}
public:
 static std::shared_ptr<const CheckedText>create(const std::shared_ptr<const nfs_soft::FramePacket>&p,const Evidence&e,const std::shared_ptr<const nfs_feedback_backdrop::VerifiedBackdrop>&b)noexcept{if(!consistent(p,e,b))return {};try{return std::shared_ptr<const CheckedText>(new CheckedText(p,b,e));}catch(...){return {};}}
 const auto&packet()const noexcept{return packet_;}const auto&backdrop()const noexcept{return backdrop_;}const Evidence&evidence()const noexcept{return evidence_;}
 bool matches(const Current&c)const noexcept{return matchesCurrent(packet_,evidence_,backdrop_,c);}
};
class Proof final {
 std::shared_ptr<const CheckedText>checked_;
 explicit Proof(std::shared_ptr<const CheckedText>c):checked_(std::move(c)){}
 friend struct ::nfs_feedback_modal_text_live::ModalScope;
public:
 const std::shared_ptr<const CheckedText>&checked()const noexcept{return checked_;}
 // Historical consistency alone is never live/native admission. The producer
 // must retain an independently checked lexical token through outer-caller swap.
 bool current()const noexcept{return checked_&&consistent(checked_->packet(),checked_->evidence(),checked_->backdrop());}
 bool matches(const Current&c)const noexcept{return checked_&&checked_->matches(c);}
};
}
