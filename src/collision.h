/**
 * Pixel-perfect collision test (g.java:315-430) - Step 4.
 *
 * Spec: docs/STEP_04_COLLISIONS.md
 */

#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include <stdint.h>

#include "level_loader.h"

#define COLLISION_HITS_MAX 5

typedef struct {
    int x[COLLISION_HITS_MAX];
    int y[COLLISION_HITS_MAX];
    bool overflow;
} CollisionHits;

void collision_hits_clear(CollisionHits* hits);
void collision_hits_add(CollisionHits* hits, int tile_x, int tile_y);

bool collision_test(Level* level,
                    int rect_x, int rect_y, int rect_w, int rect_h,
                    const bool* player_mask);

bool collision_test_collect(Level* level,
                            int rect_x, int rect_y, int rect_w, int rect_h,
                            const bool* player_mask, CollisionHits* hits);

#endif // COLLISION_H
