/**
 * Tile metadata loader for Bounce Back (/res/tf).
 *
 * Spec: docs/STEP_02_TILE_ENGINE.md
 * Reference: docs/DEOBFUSCATION.md ("Формат метаданных тайлов (/res/tf)")
 */

#ifndef TILE_METADATA_H
#define TILE_METADATA_H

#include <stdint.h>

typedef struct {
    uint8_t render_type;    // 0=skip, 1=static, 3=animated
    uint8_t image_index;    // Index in tileset images
    uint8_t transform;      // rotate/flip bitfield
    uint8_t collision_type; // parsed but not used on Step 2
    int32_t aux;            // g.java uses int (af[tile]); keep full 32-bit value
} TileMetadata;

TileMetadata* tilemetadata_load(const char* tf_path);
void tilemetadata_free(TileMetadata* meta);
uint8_t tilemetadata_flag_mask(void);
uint32_t tilemetadata_bg_color_rgb(void);

#endif // TILE_METADATA_H
