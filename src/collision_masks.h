/**
 * Collision masks storage (g.java:207) - Step 4.
 *
 * Spec: docs/STEP_04_COLLISIONS.md
 * Reference: docs/DEOBFUSCATION.md (/res/tf masks)
 */

#ifndef COLLISION_MASKS_H
#define COLLISION_MASKS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool*** masks;     // [tileId][x][y] - аналог g.s (g.java:207)
    uint8_t tile_count;
    uint8_t mask_w;    // обычно 16 после нормализации (g.java:167)
    uint8_t mask_h;
} CollisionMasks;

CollisionMasks* collision_masks_load(const char* tf_path);
void collision_masks_free(CollisionMasks* masks);

#endif // COLLISION_MASKS_H

