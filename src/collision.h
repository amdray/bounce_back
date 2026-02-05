/**
 * Pixel-perfect collision test (g.java:315-430) - Step 4.
 *
 * Spec: docs/STEP_04_COLLISIONS.md
 */

#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include <stdint.h>

#include "collision_masks.h"
#include "level_loader.h"
#include "tile_metadata.h"

bool collision_test(Level* level, TileMetadata* tile_meta,
                    CollisionMasks* masks,
                    int rect_x, int rect_y, int rect_w, int rect_h,
                    bool* player_mask);

void apply_transform(uint8_t transform, int* x, int* y);

#endif // COLLISION_H

