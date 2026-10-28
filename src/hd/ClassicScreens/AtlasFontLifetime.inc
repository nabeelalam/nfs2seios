// PRIVATE prototype: source-scoped font lifetime coherence, not retail behavior attribution.
#pragma once
#include "GuestCopyTransaction.h"
#include <array>
#include <atomic>
#include <vector>
#include <cstring>
#include <cerrno>
namespace nfs_atlas_lifetime {
enum Outcome {Idle,Restored,Unsupported,OwnerChanged,ReadOrStopFailure,RefreshFailed,Nested,IncompleteUnwind};
struct Report {Outcome outcome=Idle;bool prepared=false,contextWritten=false,refreshCompleted=false,cacheWritten=false;};
inline Report&last(){static thread_local Report r;return r;}
inline std::atomic<uintptr_t>&lease(){static std::atomic<uintptr_t> v{0};return v;}
inline uintptr_t token(){static thread_local unsigned char t;return reinterpret_cast<uintptr_t>(&t);}
struct CpuPreserve {Game&g;std::array<unsigned char,sizeof(CPU)>bytes;int savedErrno;explicit CpuPreserve(Game&v):g(v),savedErrno(errno){std::memcpy(bytes.data(),static_cast<CPU*>(&g),bytes.size());}~CpuPreserve()noexcept{std::memcpy(static_cast<CPU*>(&g),bytes.data(),bytes.size());errno=savedErrno;}};
struct Identity {uint32_t descriptor=0;std::array<uint32_t,10>w{};std::array<uint32_t,8>header{};std::array<unsigned char,27536>font{};};
inline uint32_t address(uint32_t p){return p;}
inline bool sameName(const std::array<uint32_t,10>&w){const unsigned char*p=reinterpret_cast<const unsigned char*>(w.data())+4;const char*n="swiss18.ffn";for(unsigned i=0;i<11;++i){unsigned c=p[i];if(c>='A'&&c<='Z')c+=32;if(c!=unsigned(n[i]))return false;}return p[11]==0;}
inline Identity identify(nfs::GuestCopyTransaction&tx,const std::array<uint32_t,45>&c){
 Identity id;const auto pool=tx.load<std::array<uint32_t,6>>(guest_address(dword_563D80));if(!pool[0]||!pool[1]||pool[0]==pool[1])throw Unsupported;
 uint32_t node=pool[0],previous=0;std::vector<uint32_t>seen;bool ended=false;unsigned matches=0;
 for(unsigned i=0;i<4096;++i){for(auto p:seen)if(p==node)throw Unsupported;seen.push_back(node);auto w=tx.load<std::array<uint32_t,10>>(node);if(w[9]!=previous)throw Unsupported;
  if(node!=pool[0]&&node!=pool[1]&&w[0]==c[32]){if(++matches!=1||!sameName(w)||w[4]!=27536||w[5]!=27536||w[6]!=0x20||!w[7])throw Unsupported;id.descriptor=node;id.w=w;}
  if(node==pool[1]){if(w[8])throw Unsupported;ended=true;break;}previous=node;node=w[8];if(!node)throw Unsupported;
 }
 if(!ended||matches!=1||uint64_t(c[32])+27536>UINT64_C(0x100000000)||c[33]!=c[32]+32||c[34]!=c[32]+2496||c[29]!=224)throw Unsupported;
 id.header=tx.load<std::array<uint32_t,8>>(c[32]);if(id.header[0]!=0x46544e46||id.header[1]!=27536||id.header[5]!=32||id.header[7]!=2496)throw Unsupported;
 id.font=tx.load<std::array<unsigned char,27536>>(c[32]);return id;
}
inline bool equalOwner(const Identity&a,const Identity&b){return a.descriptor==b.descriptor&&a.w[0]==b.w[0]&&a.w[4]==b.w[4]&&a.w[5]==b.w[5]&&a.w[6]==b.w[6]&&a.w[7]==b.w[7]&&sameName(b.w)&&a.header==b.header&&a.font==b.font;}
struct Scope {
 Game&g;Report report;std::array<uint32_t,45>saved{};uint32_t cached=0;Identity before;bool owns=false,prepared=false;
 explicit Scope(Game&v)noexcept:g(v){CpuPreserve preserve(g);uintptr_t zero=0;if(!lease().compare_exchange_strong(zero,token())){last().outcome=Nested;return;}owns=true;
  try{{nfs::GuestCopyTransaction tx;saved=tx.load<std::array<uint32_t,45>>(guest_address(dword_4DD6E8));cached=tx.load<uint32_t>(guest_address(dword_4E54AC));
   const auto persistent=tx.load<std::array<uint32_t,45>>(guest_address(dword_4E533C));if((cached&0xffff)!=18||saved[32]!=persistent[32]||saved[33]!=persistent[33]||saved[34]!=persistent[34])throw Unsupported;before=identify(tx,saved);if(!saved[35]||saved[36]!=0x1040)throw Unsupported;
   (void)tx.load<std::array<unsigned char,0x1040>>(saved[35]);}
   prepared=true;report.prepared=true;
  }catch(Outcome why){report.outcome=why;}catch(...){report.outcome=ReadOrStopFailure;}last()=report;
 }
 void finish(){if(!owns)return;CpuPreserve preserve(g);
  if(prepared){try{
   {nfs::GuestCopyTransaction tx;const auto after=identify(tx,saved);if(!equalOwner(before,after))throw OwnerChanged;
    const uint32_t stack=uint32_t(g.esp);if(stack<0x400)throw Unsupported;
    (void)tx.load<std::array<unsigned char,0x400>>(stack-0x400);
    const auto current=tx.load<std::array<uint32_t,45>>(guest_address(dword_4DD6E8));if(current[35]!=saved[35]||current[36]!=saved[36])throw OwnerChanged;
    (void)tx.load<std::array<unsigned char,0x1040>>(saved[35]);
    // One checked complete180-byte store; no transaction crosses original callback.
    tx.store<std::array<uint32_t,45>>(guest_address(dword_4DD6E8),saved);report.contextWritten=true;
   }
   try{g.esp=int32_t(uint32_t(g.esp)-4u);g._sub_4A6354();g.esp=int32_t(uint32_t(g.esp)+4u);report.refreshCompleted=true;}catch(...){report.outcome=RefreshFailed;throw;}
   {nfs::GuestCopyTransaction tx;if(tx.load<std::array<uint32_t,45>>(guest_address(dword_4DD6E8))!=saved)throw OwnerChanged;
    const auto after=identify(tx,saved);if(!equalOwner(before,after))throw OwnerChanged;
    tx.store<uint32_t>(guest_address(dword_4E54AC),cached);report.cacheWritten=true;
   }report.outcome=Restored;
  }catch(Outcome why){report.outcome=why;}catch(const nfs::WorkerStopped&){report.outcome=ReadOrStopFailure;last()=report;lease().store(0);owns=false;throw;}catch(...){if(report.outcome==RefreshFailed){last()=report;lease().store(0);owns=false;throw;}report.outcome=ReadOrStopFailure;}}
  last()=report;lease().store(0);owns=false;
 }
 ~Scope()noexcept{if(owns){if(prepared)report.outcome=IncompleteUnwind;last()=report;lease().store(0);owns=false;}}
 Scope(const Scope&)=delete;Scope&operator=(const Scope&)=delete;
};
} // namespace nfs_atlas_lifetime
