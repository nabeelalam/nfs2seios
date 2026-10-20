// SPDX-License-Identifier: MIT
#include "FrontendTextContext.h"
namespace {
thread_local uint64_t current_text = 0;
}
extern "C" uint64_t nfs_front_text_context(void) { return current_text; }
extern "C" void nfs_front_text_context_set(uint64_t identity) { current_text = identity; }
