// SPDX-License-Identifier: MIT
#include "GameTrace.h"
#include <sstream>
namespace {struct Call {uint32_t id=0,stack=0;};thread_local Call calls[32];thread_local unsigned next=0,count=0;}
extern "C" void nfs_trace_guest_call(uint32_t id,uint32_t stack) noexcept{calls[next]={id,stack};next=(next+1)%32;if(count<32)++count;}
namespace nfs {std::string recent_game_calls(){std::ostringstream text;text<<"recent native game calls (oldest first):";for(unsigned i=0;i<count;++i){const auto &call=calls[(next+32-count+i)%32];text<<" 0x"<<std::hex<<call.id<<"@0x"<<call.stack;}return text.str();}}
