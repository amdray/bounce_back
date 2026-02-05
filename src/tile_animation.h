/**
 * Tile animation (renderType=3) - Step 07.
 *
 * Spec: docs/STEP_07_TILE_ANIMATION.md
 */

#ifndef TILE_ANIMATION_H
#define TILE_ANIMATION_H

#include <stdint.h>

#include "tile_metadata.h"

typedef struct {
    uint8_t count;         // animCount (U)
    uint8_t** frames;      // animFrames[group][frame] (m)
    uint8_t* period;       // animPeriod[group] (O)
    uint8_t* frame_index;  // animFrameIndex[group] (ai)
    uint8_t* timer;        // animTimer[group] (aa)
    uint8_t* frame_counts; // frameCount[group]
} TileAnimation;

TileAnimation* animation_load(const char* tf_path);
void animation_free(TileAnimation* anim);
void animation_tick(TileAnimation* anim);
uint8_t animation_get_tile(TileAnimation* anim, TileMetadata* tile_meta, uint8_t tile_id);

#endif // TILE_ANIMATION_H

