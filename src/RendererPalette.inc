// SPDX-License-Identifier: MIT
// Included by the GLES owner after its palette and texture state declarations.
// Keys identify submissions; exact bytes decide whether table contents changed.
typedef struct {
    uint64_t identity;
    uint64_t generation;
    uint32_t colours[256];
} NfsOwnedPalette;
static NfsOwnedPalette nfs_palette_cache[128];
static uint32_t nfs_palette_replacement;
static uint64_t nfs_palette_sequence, g_paletteGeneration;
static uint64_t nfs_palette_uploads;

uint64_t nfs_glide_palette_upload_count(void) { return nfs_palette_uploads; }

void nfs_glide_palette_table_owned(GrChipID_t tmu, GrTexTable_t type,
                                  uint64_t identity, const GuTexPalette *data)
{
    (void)tmu;
    if (type != GR_TEXTABLE_PALETTE) return;
    NfsOwnedPalette *record = NULL;
    for (unsigned i = 0; i < 128; ++i) {
        if (nfs_palette_cache[i].generation && nfs_palette_cache[i].identity == identity) {
            record = &nfs_palette_cache[i];
            break;
        }
    }
    if (!record) {
        record = &nfs_palette_cache[nfs_palette_replacement++ % 128];
        record->generation = 0;
        record->identity = identity;
    }
    if (!record->generation || memcmp(record->colours, data->data, sizeof(record->colours))) {
        memcpy(record->colours, data->data, sizeof(record->colours));
        record->generation = ++nfs_palette_sequence;
    }
    g_palette = record->colours;
    g_paletteGeneration = record->generation;
}
