/**
 * Player masks loader for /res/b chunk[0] - Step 10.
 *
 * Spec: docs/STEP_10_PLAYER_MASKS_RES_B.md
 * Reference: docs/DEOBFUSCATION.md ("Формат /res/b")
 */

#ifndef PLAYER_MASKS_H
#define PLAYER_MASKS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t w;
    uint8_t h;
    bool* data; // flattened: data[x + y*w]
} PlayerMask;

typedef struct {
    uint8_t count;
    PlayerMask* masks;
} PlayerMasks;

PlayerMasks* player_masks_load(const char* path);
void player_masks_free(PlayerMasks* pm);

const bool* player_masks_select(PlayerMasks* pm, int sprite_index, int* out_w, int* out_h);

#endif // PLAYER_MASKS_H

