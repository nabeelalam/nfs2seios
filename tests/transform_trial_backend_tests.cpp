// SPDX-License-Identifier: MIT
#include "TransformTrial.h"
#include <cassert>
#include <cstring>
extern "C" bool nfs_transform_trial_cached() noexcept;
int main(int argc,char **argv){
    const bool initially_cached=argc==2 && std::strcmp(argv[1],"--expect-cache")==0;
    auto &control=nfs::transform_trial_control();
    assert(control.snapshot().mode()==(initially_cached?1:0));
    assert(nfs_transform_trial_cached()==initially_cached);
    assert(control.set_cached(true));assert(nfs_transform_trial_cached());
    assert(control.set_cached(false));assert(!nfs_transform_trial_cached());
}
