// SPDX-License-Identifier: MIT
#include "TransformTrial.h"
extern "C" uint64_t nfs_test_trial_peer_token(){return nfs::transform_trial_control().snapshot().token;}
extern "C" void nfs_test_trial_peer_register(){nfs::transform_trial_control().register_backend();}
extern "C" bool nfs_test_trial_peer_set(bool cache){return nfs::transform_trial_control().set_cached(cache);}
