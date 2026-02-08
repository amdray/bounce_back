/**
 * Player masks loader implementation - Step 10.
 */

#include "player_masks.h"

#include "resource_loader.h"

#include <stdlib.h>

static void player_masks_free_partial(PlayerMasks* pm, int loaded) {
    if (!pm) return;
    if (pm->masks) {
        for (int i = 0; i < loaded; i++) {
            free(pm->masks[i].data);
            pm->masks[i].data = NULL;
        }
        free(pm->masks);
        pm->masks = NULL;
    }
    free(pm);
}

PlayerMasks* player_masks_load(const char* path) {
    ResourceContainer* rc = resource_load(path);
    if (!rc) return NULL;

    size_t chunk_size = 0;
    const uint8_t* chunk0 = resource_get_element(rc, 0, &chunk_size);
    if (!chunk0 || chunk_size < 1) {
        resource_free(rc);
        return NULL;
    }

    size_t pos = 0;
    uint8_t mask_count = chunk0[pos++];

    PlayerMasks* pm = (PlayerMasks*)calloc(1, sizeof(PlayerMasks));
    if (!pm) {
        resource_free(rc);
        return NULL;
    }
    pm->count = mask_count;

    pm->masks = (PlayerMask*)calloc(mask_count, sizeof(PlayerMask));
    if (!pm->masks) {
        resource_free(rc);
        free(pm);
        return NULL;
    }

    for (int i = 0; i < (int)mask_count; i++) {
        if (pos + 2 > chunk_size) {
            resource_free(rc);
            player_masks_free_partial(pm, i);
            return NULL;
        }

        uint8_t w = chunk0[pos++];
        uint8_t h = chunk0[pos++];
        pm->masks[i].w = w;
        pm->masks[i].h = h;

        size_t needed = (size_t)w * (size_t)h;
        if (pos + needed > chunk_size) {
            resource_free(rc);
            player_masks_free_partial(pm, i);
            return NULL;
        }

        bool* data = (bool*)calloc(needed, sizeof(bool));
        if (!data) {
            resource_free(rc);
            player_masks_free_partial(pm, i);
            return NULL;
        }

        for (size_t y = 0; y < (size_t)h; y++) {
            for (size_t x = 0; x < (size_t)w; x++) {
                data[x + y * (size_t)w] = (chunk0[pos++] != 0);
            }
        }

        pm->masks[i].data = data;
    }

    resource_free(rc);
    return pm;
}

void player_masks_free(PlayerMasks* pm) {
    if (!pm) return;
    if (pm->masks) {
        for (int i = 0; i < (int)pm->count; i++) {
            free(pm->masks[i].data);
        }
        free(pm->masks);
    }
    free(pm);
}

const bool* player_masks_select(PlayerMasks* pm, int sprite_index, int* out_w, int* out_h) {
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!pm || !pm->masks) return NULL;

    int idx = -1;
    if ((sprite_index >= 0 && sprite_index <= 8) || (sprite_index >= 20 && sprite_index <= 22)) {
        idx = 0;
    } else if (sprite_index >= 9 && sprite_index <= 19) {
        idx = 1;
    }

    if (idx < 0 || idx >= (int)pm->count) return NULL;

    if (out_w) *out_w = pm->masks[idx].w;
    if (out_h) *out_h = pm->masks[idx].h;
    return pm->masks[idx].data;
}

