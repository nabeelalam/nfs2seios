// SPDX-License-Identifier: MIT
#include "FrontendTextContext.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>
#define CHECK(v) do { if (!(v)) std::abort(); } while (0)
static uint64_t identity(const std::string &text, unsigned &reads) {
    return nfs::frontend_text_identity(1,[&](uint32_t p) {
        ++reads;
        if (p>text.size()+1) throw std::out_of_range("Bounded fixture");
        return uint8_t(text[p-1]);
    });
}
int main() {
    CHECK(nfs_front_text_context()==0);
    unsigned reads=0;
    const auto first=identity("North Country",reads);
    CHECK(first && reads==14);
    reads=0; CHECK(identity("North Country",reads)==first);
    reads=0; CHECK(identity("North Count",reads)!=first);
    reads=0; CHECK(identity("north Country",reads)!=first);
    reads=0; CHECK(identity("",reads)==0 && reads==1);
    reads=0; CHECK(identity(std::string(95,'A'),reads)!=0 && reads==96);
    reads=0; CHECK(identity(std::string(96,'A'),reads)==0 && reads==96);
    for (const auto &text:{std::string("A\nB"),std::string("A\x80"),std::string("A\x7f")}) {
        reads=0; CHECK(identity(text,reads)==0);
    }
    CHECK(nfs::frontend_text_identity(1,[](uint32_t)->uint8_t { throw std::out_of_range("Invalid guest span"); })==0);
    reads=0;
    CHECK(nfs::frontend_text_identity(0,[&](uint32_t)->uint8_t { ++reads; return 'A'; })==0 && reads==0);
    CHECK(nfs::frontend_text_identity(UINT32_MAX,[&](uint32_t)->uint8_t { ++reads; return 'A'; })==0 && reads==1);
    {
        nfs::FrontendTextScope outer(first); CHECK(nfs_front_text_context()==first);
        std::thread worker([&] {
            CHECK(nfs_front_text_context()==0);
            nfs::FrontendTextScope own(7); CHECK(nfs_front_text_context()==7);
        });
        worker.join(); CHECK(nfs_front_text_context()==first);
        try { nfs::FrontendTextScope inner(9); CHECK(nfs_front_text_context()==9); throw std::runtime_error("Unwind"); }
        catch (const std::runtime_error &) { CHECK(nfs_front_text_context()==first); }
    }
    CHECK(nfs_front_text_context()==0);
    std::puts("PASS complete ASCII text identity, bounded/invalid/overflow reads, thread isolation and nested normal/exception restoration");
}
