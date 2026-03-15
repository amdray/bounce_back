/**
 * Collision wrappers over Level runtime API.
 */

#include "collision.h"

void collision_hits_clear(CollisionHits* hits) {
    if (!hits) return;
    for (int i = 0; i < COLLISION_HITS_MAX; i++) {
        hits->x[i] = -1;
        hits->y[i] = -1;
    }
    hits->overflow = false;
}

void collision_hits_add(CollisionHits* hits, int tile_x, int tile_y) {
    if (!hits) return;
    for (int i = 0; i < COLLISION_HITS_MAX; i++) {
        if (hits->x[i] == -1) {
            hits->x[i] = tile_x;
            hits->y[i] = tile_y;
            return;
        }
    }
    hits->overflow = true;
}

bool collision_test_collect(Level* level,
                            int rect_x, int rect_y, int rect_w, int rect_h,
                            const bool* player_mask, CollisionHits* hits) {
    if (!level || !hits) return false;
    bool overflow = false;
    int obj = -1;
    bool res = level_test_collision_collect(level, rect_x, rect_y, rect_w, rect_h, player_mask,
                                            hits->x, hits->y, COLLISION_HITS_MAX, &overflow, &obj);
    hits->overflow = overflow;
    return res;
}

bool collision_test(Level* level,
                    int rect_x, int rect_y, int rect_w, int rect_h,
                    const bool* player_mask) {
    return level_test_collision(level, rect_x, rect_y, rect_w, rect_h, player_mask);
}
