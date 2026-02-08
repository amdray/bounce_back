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

#define COLLISION_HITS_MAX 5

typedef struct {
    int x[COLLISION_HITS_MAX];
    int y[COLLISION_HITS_MAX];
    bool overflow;
} CollisionHits;

void collision_hits_clear(CollisionHits* hits);
void collision_hits_add(CollisionHits* hits, int tile_x, int tile_y);

bool collision_test(Level* level, TileMetadata* tile_meta,
                    CollisionMasks* masks,
                    int rect_x, int rect_y, int rect_w, int rect_h,
                    const bool* player_mask);

bool collision_test_collect(Level* level, TileMetadata* tile_meta,
                            CollisionMasks* masks,
                            int rect_x, int rect_y, int rect_w, int rect_h,
                            const bool* player_mask, CollisionHits* hits);

void apply_transform(uint8_t transform, int* x, int* y);

#endif // COLLISION_H
