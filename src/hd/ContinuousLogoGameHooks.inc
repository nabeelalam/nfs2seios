// SPDX-License-Identifier: MIT
#pragma once
#include "ContinuousLogoCollector.h"
#include <new>
#include <cstdlib>
#include <exception>
namespace nfs_logo_publisher {

inline bool enabled()noexcept {static const bool on=[](){const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_HD");return p&&p[0]=='1'&&p[1]==0;}();return on&&nfs_background_scene_runtime::catalogConfigured();}
inline uintptr_t threadToken()noexcept {static thread_local unsigned char token;return reinterpret_cast<uintptr_t>(&token);}
inline Collector*&stateSlot()noexcept {struct Storage{Collector*value=nullptr;~Storage(){delete value;}};static thread_local Storage storage;return storage.value;}
inline std::atomic<uintptr_t>&publisherThread()noexcept {static std::atomic<uintptr_t>value{0};return value;}
inline Collector*store()noexcept {return enabled()?stateSlot():nullptr;}
inline bool owner()noexcept {auto*s=store();return s&&s->inMenu&&s->thread==threadToken();}
inline const nfs_small_logo_live::SourceIdentity& logoIdentity(unsigned role){auto*s=store();if(!s||!s->catalog||role>=5)throw 1;return s->catalog->identities[role];}
inline std::array<unsigned char,16> logoHeader(unsigned role){std::array<unsigned char,16> result{};const auto&i=logoIdentity(role);for(unsigned k=0;k<16;++k)result[k]=uint8_t(i.header[k/4]>>(8*(k%4)));return result;}
inline int exceptions()noexcept {
#if __cplusplus >=201703L
return std::uncaught_exceptions();
#else
return std::uncaught_exception()?1:0;
#endif
}
struct MenuOwnerScope {
 Collector*s=nullptr;uintptr_t game=0;bool active=false,claimed=false;int unwind=0;
 explicit MenuOwnerScope(Game&g)noexcept {if(!enabled()||uint32_t(g.eax)!=uint32_t(guest_address(off_4D3EC4)))return;uintptr_t expected=0,token=threadToken();if(!publisherThread().compare_exchange_strong(expected,token)&&expected!=token)return;claimed=expected==0;auto trusted=nfs_background_scene_runtime::acquireCatalog();if(!trusted){if(claimed)publisherThread().store(0);return;}auto*&slot=stateSlot();if(!slot)slot=new(std::nothrow)Collector{};s=slot;if(!s){if(claimed)publisherThread().store(0);return;}s->catalog=&trusted->logos;game=reinterpret_cast<uintptr_t>(&g);active=s->beginMenu(game,threadToken());unwind=exceptions();}
 ~MenuOwnerScope(){if(active)s->endMenu(game,exceptions()>unwind);if(claimed)publisherThread().store(0);}
};
struct OwnerScope {
 Collector*s=nullptr;bool lexical=false,active=false;int unwind=0;uint32_t kind=0,arg=0;
 OwnerScope(Game&g,uint32_t entryKind)noexcept {
  if(!owner())return;s=store();if(s->gameOwner!=reinterpret_cast<uintptr_t>(&g)){s->invalidate(Collector::Stop::Owner);return;}
  unwind=exceptions();kind=entryKind;arg=uint32_t(g.eax);lexical=true;
  if(s->invocationDepth++){s->invalidate(Collector::Stop::Nested);s->invalid=true;}
 }
 // Called only after the original cached timer-return branch, before any redraw-state/art write.
 void start()noexcept {if(!lexical||active||s->invalid)return;active=s->enterIteration(kind,arg,threadToken());}
 ~OwnerScope(){if(!lexical)return;if(active)s->leaveIteration(exceptions()>unwind);else if(exceptions()>unwind)s->invalidate(Collector::Stop::Unwind);if(s->invocationDepth)--s->invocationDepth;}
};
inline void checked(uint32_t at,size_t n){(void)nfs_host_address(at,n);}
inline void target(nfs::GuestCopyTransaction&tx,Target&t,uint32_t at=uint32_t(guest_address(dword_4DB240))){
 for(unsigned i=0;i<14;++i)t.w[i]=tx.load<uint32_t>(at+i*4);const auto&w=t.w;if(!w[1]||w[1]>640||!w[2]||w[2]>480||(w[7]&255)!=16||!w[8]||!w[11]||!w[12])throw 1;
 checked(w[8],16);checked(w[12],w[1]*4);checked(w[11],w[2]*4);for(unsigned i=0;i<4;++i)t.header[i]=tx.load<uint32_t>(w[8]+i*4);if((t.header[1]&65535)!=w[1]||(t.header[1]>>16)!=w[2])throw 1;
 for(unsigned i=0;i<w[1];++i)t.x[i]=tx.load<uint32_t>(w[12]+i*4);for(unsigned i=0;i<w[2];++i)t.y[i]=tx.load<uint32_t>(w[11]+i*4);
 const uint32_t handles[4]={tx.load<uint32_t>(dword_4D231C),tx.load<uint32_t>(dword_4D2320),tx.load<uint32_t>(dword_4D4D24),tx.load<uint32_t>(dword_4DABE4)};
 t.ownerState[0]=tx.load<uint32_t>(dword_4D2314+2);for(unsigned i=0;i<4;++i)t.ownerState[i+1]=handles[i];
 for(auto h:handles){if(!h)continue;checked(h,56);bool match=true;for(unsigned i:{0u,1u,2u,7u,8u,10u,11u,12u,13u})if(tx.load<uint32_t>(h+i*4)!=w[i])match=false;if(match){t.owner=h;for(unsigned i=0;i<14;++i)t.ownerWords[i]=tx.load<uint32_t>(h+i*4);break;}}
 if(!t.owner)throw 2;
}
inline void plane(Plane&p,uint32_t&error)noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,p.target);auto&w=p.target.w;p.width=w[1];p.height=w[2];bool linear=true;for(unsigned x=0;x<p.width;++x)if(p.target.x[x]!=2*x)linear=false;for(unsigned y=0;y<p.height;++y)if(p.target.y[y]!=16+2*p.width*y)linear=false;
 if(linear){size_t bytes=size_t(p.width)*p.height*2;checked(w[8],16+bytes);__builtin_memcpy(p.pixels.data(),nfs_host_address(w[8]+16,bytes),bytes);}else for(unsigned y=0;y<p.height;++y)for(unsigned x=0;x<p.width;++x){uint64_t at=uint64_t(w[8])+p.target.x[x]+p.target.y[y];if(at>UINT32_MAX-1)throw 1;p.pixels[y*p.width+x]=tx.load<uint16_t>(uint32_t(at));}
 Target after{};nfs_logo_publisher::target(tx,after);if(!same(after,p.target)){error|=Changed;return;}p.valid=1;
 }catch(...){error|=Read;}}
inline void table(nfs::GuestCopyTransaction&tx,std::array<uint32_t,9>&out,bool full){uint32_t at=uint32_t(guest_address(full?off_4DAD88:off_4DAD64));for(unsigned i=0;i<9;++i)out[i]=tx.load<uint32_t>(at+i*4);}
inline void preGroup(Game&)noexcept {if(!owner())return;auto*s=store();if(auto*g=s->beginGroup())nfs_logo_publisher::plane(g->pre,g->error);}
inline void postGroup(Game&)noexcept {if(!owner())return;auto*s=store();if(auto*g=s->current()){nfs_logo_publisher::plane(g->post,g->error);s->finishGroup();}}
inline void armCopy(Game&)noexcept {if(owner())store()->armCopy();}
inline void digest(Draw&d,bool after)noexcept{try{if(d.role>=5)throw 1;const auto&e=logoIdentity(d.role);auto&out=after?d.sourceAfter:d.sourceBefore;auto&hash=after?d.shaAfter:d.shaBefore;nfs::GuestCopyTransaction tx;checked(d.args[0],e.span);__builtin_memcpy(out.data(),nfs_host_address(d.args[0],e.span),e.span);for(unsigned k=0;k<4;++k)if(nfs_small_logo_live::le32(out.data()+4*k)!=e.header[k])throw 1;if(!nfs_texture_sha256(out.data(),e.span,hash.data())||hash!=e.sha)throw 1;if(after&&hash!=d.shaBefore)throw 1;}catch(...){d.error|=Identity;}}
struct ResolvedScope{
 Draw*d=nullptr;Copy*c=nullptr;int unwind=0;
 explicit ResolvedScope(Game&g)noexcept{
  if(!owner())return;auto*s=store();if(!s||s->depth!=1)return;unwind=exceptions();
  if(s->copyActive){c=&s->copy;c->args={uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)};nfs_logo_publisher::plane(c->pre,c->error);try{nfs::GuestCopyTransaction tx;uint32_t wh=tx.load<uint32_t>(c->args[0]+4);if((tx.load<uint32_t>(c->args[0])&127)!=120||(wh&65535)!=343||!(wh>>16)||(wh>>16)>480||c->args[1]!=0x2800||c->args[2]||c->args[3])throw 1;c->span=16+343*(wh>>16)*2;checked(c->args[0],c->span);__builtin_memcpy(c->sourceBefore.data(),nfs_host_address(c->args[0],c->span),c->span);nfs_texture_sha256(c->sourceBefore.data(),c->span,c->shaBefore.data());table(tx,c->tableBefore,true);{const auto&z=s->group;if(!z.complete||z.serial!=c->linkedGroup||z.post.target.w[8]!=c->args[0])throw 2;const uint32_t handles[2]={tx.load<uint32_t>(dword_4D231C),tx.load<uint32_t>(dword_4D2320)};for(auto h:handles)if(h&&tx.load<uint32_t>(h+0x20)==c->args[0]){nfs_logo_publisher::target(tx,c->sourceOwner,h);break;}if(c->sourceOwner.w[8]!=z.post.target.w[8]||c->sourceOwner.x!=z.post.target.x||c->sourceOwner.y!=z.post.target.y||c->sourceOwner.owner!=z.post.target.owner)throw 2;}if(!c->sourceOwner.owner)throw 2;const auto&linked=s->group;for(unsigned j=0;j<linked.draws;++j){const auto&d=linked.draw[j];int x=int(d.sourceHeader[3]<<20)>>20,y=int(d.sourceHeader[3]<<4)>>20;int l=std::max(x,int(d.before.w[3])),t=std::max(y,int(d.before.w[4])),right=std::min(x+int(d.sourceHeader[1]&65535),int(d.before.w[5])),bottom=std::min(y+int(d.sourceHeader[1]>>16),int(d.before.w[6]));if(l<0||t<0||right>343||bottom>int(wh>>16))throw 2;for(int yy=t;yy<bottom;++yy)for(int xx=l;xx<right;++xx){uint64_t off=uint64_t(c->sourceOwner.x[unsigned(xx)])+c->sourceOwner.y[unsigned(yy)];if(off+2>c->span)throw 2;uint16_t v=uint16_t(c->sourceBefore[size_t(off)])|(uint16_t(c->sourceBefore[size_t(off)+1])<<8);if(v!=linked.post.pixels[unsigned(yy)*343+unsigned(xx)])throw 2;}}}catch(...){c->error|=Identity;}return;}
  auto*p=s->current();if(!p)return;if(s->pending>=0||p->draws==MaxDraws){p->error|=Order;return;}s->pending=int32_t(p->draws++);d=&p->draw[uint32_t(s->pending)];d->args={uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)};d->resolves=1;if(d->args[2]||d->args[3])d->error|=Unsupported;
  try{nfs::GuestCopyTransaction tx;std::array<unsigned char,16>head{};checked(d->args[0],16);__builtin_memcpy(head.data(),nfs_host_address(d->args[0],16),16);for(unsigned i=0;i<5;++i)if(head==logoHeader(i)){std::array<unsigned char,32>hash{};checked(d->args[0],logoIdentity(i).span);if(nfs_texture_sha256(nfs_host_address(d->args[0],logoIdentity(i).span),logoIdentity(i).span,hash.data())&&hash==logoIdentity(i).sha){d->role=i;break;}}if(d->role>=5)throw 1;const auto&e=logoIdentity(d->role);d->span=e.span;d->paletteCount=e.paletteCount;d->actualMaxIndex=e.maxIndex;for(unsigned i=0;i<4;++i)d->sourceHeader[i]=tx.load<uint32_t>(d->args[0]+4*i);nfs_logo_publisher::target(tx,d->before);table(tx,d->tableBefore,(d->args[1]&~0x1ffu)==0x2800);if((d->args[1]&~0x1ffu)!=0x2800&&(d->args[1]&~0x1ffu)!=0x1c00)throw 1;
  }catch(...){d->error|=Identity;}nfs_logo_publisher::digest(*d,false);
 }
 ~ResolvedScope(){if(!d&&!c)return;auto*s=store();if(d){try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,d->after);table(tx,d->tableAfter,(d->args[1]&~0x1ffu)==0x2800);}catch(...){d->error|=Read;}nfs_logo_publisher::digest(*d,true);if(!same(d->before,d->after))d->error|=Changed;for(unsigned i=0;i<6;++i)if(d->tableBefore[i]!=d->tableAfter[i])d->error|=Changed;if(d->tableBefore[7]!=d->tableAfter[7]||d->tableAfter[6]!=d->callback||d->tableAfter[8]!=d->callbackFormat)d->error|=Changed;if(!d->rows||!d->paletteValid)d->error|=Rows;if(exceptions()>unwind)d->error|=Unwind;d->complete=!d->error;s->pending=-1;}
 if(c){nfs_logo_publisher::plane(c->post,c->error);try{nfs::GuestCopyTransaction tx;if(!c->span)throw 1;__builtin_memcpy(c->sourceAfter.data(),nfs_host_address(c->args[0],c->span),c->span);nfs_texture_sha256(c->sourceAfter.data(),c->span,c->shaAfter.data());table(tx,c->tableAfter,true);}catch(...){c->error|=Read;}if(!same(c->pre.target,c->post.target)||c->shaBefore!=c->shaAfter)c->error|=Changed;for(unsigned i=0;i<6;++i)if(c->tableBefore[i]!=c->tableAfter[i])c->error|=Changed;if(c->tableBefore[7]!=c->tableAfter[7])c->error|=Changed;if(!c->rows)c->error|=Rows;if(exceptions()>unwind)c->error|=Unwind;c->complete=!c->error;s->copyActive=false;s->finishCopy();}
 }
};
inline void row(Game&g,uint32_t callback)noexcept{if(!owner())return;auto*s=store();if(!s||s->depth!=1)return;const std::array<uint32_t,4>args{{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};
 if(s->copyActive){auto&c=s->copy;if(c.rows==MaxRows){c.error|=Rows;return;}auto&r=c.row[c.rows++];r.row={callback,args};try{uint32_t bpp=0;if(callback==uint32_t(game_id_sub_483288)||callback==uint32_t(game_id_sub_482BD4))bpp=1;for(auto id:{game_id_sub_482ED4,game_id_sub_482FD4,game_id_sub_483070,game_id_sub_483100,game_id_sub_4831A8})if(callback==uint32_t(id))bpp=4;if(!bpp||!args[2]||args[2]>640)throw 1;r.bytes=args[2]*bpp;nfs::GuestCopyTransaction tx;r.converter=tx.load<uint32_t>(dword_571264);r.finalCallback=tx.load<uint32_t>(dword_571260);r.colorKey=tx.load<uint32_t>(dword_4DD03C);checked(args[0],r.bytes);checked(args[1],args[2]*2);__builtin_memcpy(r.source.data(),nfs_host_address(args[0],r.bytes),r.bytes);Target now{};nfs_logo_publisher::target(tx,now);if(!same(now,c.pre.target))throw 1;}catch(...){c.error|=Rows;}return;}
 auto*p=s->current();if(!p||s->pending<0)return;auto&d=p->draw[uint32_t(s->pending)];if(d.rows==MaxRows){d.error|=Rows;return;}d.row[d.rows++]={callback,args};
 try{if(!args[2]||args[2]>640||uint64_t(args[0])<uint64_t(d.args[0])+16||uint64_t(args[0])+args[2]>uint64_t(d.args[0])+16+uint64_t(d.sourceHeader[1]&65535)*(d.sourceHeader[1]>>16))throw 1;checked(args[1],size_t(args[2])*2);const bool full=(d.args[1]&~0x1ffu)==0x2800;const uint32_t expected=full?uint32_t(game_id_sub_483288):uint32_t(game_id_sub_482BD4);if(callback!=expected||d.role>=5)throw 1;nfs::GuestCopyTransaction tx;Target now{};nfs_logo_publisher::target(tx,now);if(!same(now,d.before)){d.error|=Changed;return;}uint32_t global=tx.load<uint32_t>(full?dword_4DADA0:dword_4DAD7C),format=tx.load<uint32_t>(full?dword_4DADA8:dword_4DAD84),pal=tx.load<uint32_t>(full?dword_5636EC:dword_5636E4);if(global!=callback||format!=0x7b||!pal)throw 1;checked(pal,d.paletteCount*4);if(d.rows==1){d.callback=callback;d.paletteGuest=pal;d.callbackGlobal=global;d.callbackFormat=format;}else if(d.callback!=callback||d.paletteGuest!=pal||d.callbackGlobal!=global||d.callbackFormat!=format)throw 1;for(unsigned i=0;i<d.paletteCount;++i){auto x=tx.load<uint32_t>(pal+i*4);if(d.rows==1)d.paletteFirst[i]=x;else if(d.paletteFirst[i]!=x)throw 1;d.paletteLast[i]=x;}if(full&&d.paletteCount==251){if(tx.load<uint32_t>(pal+255*4)!=0)throw 1;d.paletteFirst[255]=d.paletteLast[255]=0;}d.paletteValid=1;
 }catch(...){d.error|=Palette;}
}
}
