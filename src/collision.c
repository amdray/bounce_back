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

void apply_transform(uint8_t transform, int* x, int* y) {
    int k = 15;
    int m = 15;
    int i10 = *x;
    int i11 = *y;
    if (transform & 0x8) i10 = k - i10;
    if (transform & 0x4) i11 = m - i11;
    switch (transform & 0x3) {
        case 0:
            break;
        case 1: {
            int t = i10;
            i10 = i11;
            i11 = m - t;
            break;
        }
        case 2:
            i10 = k - i10;
            i11 = m - i11;
            break;
        case 3: {
            int t = i11;
            i11 = i10;
            i10 = m - t;
            break;
        }
    }
    *x = i10;
    *y = i11;
}

bool collision_test_collect(Level* level, TileMetadata* tile_meta,
                            CollisionMasks* masks,
                            int rect_x, int rect_y, int rect_w, int rect_h,
                            const bool* player_mask, CollisionHits* hits) {
    (void)tile_meta;
    (void)masks;
    if (!level || !hits) return false;
    bool overflow = false;
    int obj = -1;
    bool res = level_test_collision_collect(level, rect_x, rect_y, rect_w, rect_h, player_mask,
                                            hits->x, hits->y, COLLISION_HITS_MAX, &overflow, &obj);
    hits->overflow = overflow;
    return res;
}

bool collision_test(Level* level, TileMetadata* tile_meta,
                    CollisionMasks* masks,
                    int rect_x, int rect_y, int rect_w, int rect_h,
                    const bool* player_mask) {
    (void)tile_meta;
    (void)masks;
    return level_test_collision(level, rect_x, rect_y, rect_w, rect_h, player_mask);
}
