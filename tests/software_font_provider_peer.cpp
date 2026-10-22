// SPDX-License-Identifier: MIT
#include "hd/SoftwareFontProvider.h"
std::shared_ptr<const nfs_soft::FontCatalog> acquireFontFromPeer(){return nfs_soft::acquireFontCatalog();}
