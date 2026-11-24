// SPDX-License-Identifier: MIT
// Link only with the instrumented Game dispatcher; never part of normal sources.
#include "TransformTrial.h"
#include <cstdlib>
#include <cstring>
namespace {const bool registered=[]{
    auto &control=nfs::transform_trial_control();control.register_backend();
    const auto *requested=std::getenv("NFS_PRIVATE_TRANSFORM_TRIAL");
    if(requested && std::strcmp(requested,"1")==0)control.set_cached(true);
    return true;
}();}
extern "C" bool nfs_transform_trial_cached() noexcept {
    return nfs::transform_trial_control().snapshot().mode()==1;
}
