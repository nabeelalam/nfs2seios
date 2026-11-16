// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstdint>
namespace nfs_movie_postflush {
// Observation only. Original flush/decode/render/poll calls are unchanged.
struct Scope {
 uint64_t invocation=0;bool active=false;
 Scope()noexcept{const int saved=errno;const char*p=std::getenv("NFS_PRIVATE_MOVIE_POSTFLUSH_TRACE");if(p&&p[0]=='1'&&!p[1]){static std::atomic<unsigned>next{0};unsigned n=next.fetch_add(1,std::memory_order_relaxed);if(n<4)invocation=uint64_t(n)+1;}errno=saved;}
 void presented()noexcept{if(!invocation||active)return;const int saved=errno;active=true;std::fprintf(stderr,"MOVIE POSTFLUSH event=active invocation=%llu source=4242f0 firstDecodedPresentationReturned=1 initialKeyboardFlushComplete=1 nextInputPoll=424de0\n",(unsigned long long)invocation);std::fflush(stderr);errno=saved;}
 ~Scope()noexcept{if(!active)return;const int saved=errno;std::fprintf(stderr,"MOVIE POSTFLUSH event=end invocation=%llu\n",(unsigned long long)invocation);std::fflush(stderr);errno=saved;}
};
}
