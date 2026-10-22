// SPDX-License-Identifier: MIT
#ifndef NFS_SOFTWARE_TEXT_LAYER_ABI_H
#define NFS_SOFTWARE_TEXT_LAYER_ABI_H
#include <stdint.h>
#include <stddef.h>
#include "SoftwareTextVertex.h"
/* Coverage is from the sealed startup FontCatalog retained by the queued packet.
   Shape, digest and numeric identity alone do not authenticate that provider. */
typedef struct NfsSoftwareTextFontView {
 uint32_t schema,width,height;
 uint64_t identity;
 const unsigned char*alpha;size_t bytes;unsigned char sha256[32];
} NfsSoftwareTextFontView;
typedef struct NfsSoftwareTextLayerView {
 uint32_t logicalWidth,logicalHeight;
 NfsSoftwareTextFontView font;
 const NfsSoftwareTextVertex*vertices;size_t vertexCount;
} NfsSoftwareTextLayerView;
#endif
