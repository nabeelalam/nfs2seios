// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
/* Exact original buffered-pointer ABI, not a projected cursor checkpoint. */
typedef struct NfsMenuPointerEvent { uint32_t offset,data,time,sequence; } NfsMenuPointerEvent;
#ifdef __cplusplus
extern "C" {
#endif
uint64_t nfs_menu_pointer_read_begin(const int32_t state[8],uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
void nfs_menu_pointer_read_receipt(const NfsMenuPointerEvent*,uint32_t,int32_t,uint32_t,uint32_t,uint32_t);
void nfs_menu_pointer_read_finish(uint64_t,const int32_t state[8],uint32_t complete);
uint64_t nfs_menu_pointer_warp_begin(void);
void nfs_menu_pointer_warp_finish(uint64_t,const int32_t state[8],uint32_t complete);
void nfs_menu_pointer_queue_reset(void);
#ifdef __cplusplus
void nfs_menu_pointer_metadata_failure(void) noexcept;
#else
void nfs_menu_pointer_metadata_failure(void);
#endif
#ifdef __cplusplus
}
#include <array>
#include <cerrno>
#include <exception>
namespace nfs {
inline void menu_pointer_finish_noexcept(uint64_t token,const int32_t* state,uint32_t complete,bool warp)noexcept{
 try{if(warp)nfs_menu_pointer_warp_finish(token,state,complete);else nfs_menu_pointer_read_finish(token,state,complete);}
 catch(...){nfs_menu_pointer_metadata_failure();}
}
/* Read() owns/releases its checked guest-copy transaction before returning.
   These scopes never hold guest/input locks across host metadata calls. */
template<class Read> class MenuPointerReadScope {
 Read read;uint64_t token=0;bool exceptions=false;
public:
 explicit MenuPointerReadScope(Read r,uint32_t outX,uint32_t outY,uint32_t menuX,uint32_t menuY,uint32_t outButtons):read(r),exceptions(std::uncaught_exception()){
  const int saved=errno;std::array<int32_t,8> state;
  try{state=read();}catch(...){nfs_menu_pointer_metadata_failure();errno=saved;throw;}
  try{token=nfs_menu_pointer_read_begin(state.data(),outX,outY,menuX,menuY,outButtons);}catch(...){nfs_menu_pointer_metadata_failure();}
  errno=saved;
 }
 ~MenuPointerReadScope()noexcept{
  const int saved=errno;
  if(token)try{if(std::uncaught_exception()!=exceptions)menu_pointer_finish_noexcept(token,nullptr,0,false);else{const auto s=read();nfs_menu_pointer_read_finish(token,s.data(),1);}}catch(...){menu_pointer_finish_noexcept(token,nullptr,0,false);}
  errno=saved;
 }
};
template<class Read> class MenuPointerWarpScope {
 Read read;uint64_t token=0;bool exceptions=false;
public:
 explicit MenuPointerWarpScope(Read r):read(r),exceptions(std::uncaught_exception()) {const int saved=errno;try{token=nfs_menu_pointer_warp_begin();}catch(...){nfs_menu_pointer_metadata_failure();}errno=saved;}
 ~MenuPointerWarpScope()noexcept{
  const int saved=errno;
  if(token)try{if(std::uncaught_exception()!=exceptions)menu_pointer_finish_noexcept(token,nullptr,0,true);else{const auto s=read();nfs_menu_pointer_warp_finish(token,s.data(),1);}}catch(...){menu_pointer_finish_noexcept(token,nullptr,0,true);}
  errno=saved;
 }
};
}
#endif
