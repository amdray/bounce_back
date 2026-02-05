/**
 * Collision masks loader (/res/tf) - Step 4.
 *
 * Loads boolean masks for tiles:
 * - mask present only when collisionType==1 (g.java:226-230)
 * - for collisionType==3 creates alias: s[tileId] = s[aux] (g.java:234)
 */

#include "collision_masks.h"
#include "resource_loader.h"

#include <stdio.h>
#include <stdlib.h>

static int require_bytes(const char* ctx, const uint8_t* p, const uint8_t* end, size_t need) {
    size_t have = (size_t)(end - p);
    if (have < need) {
        fprintf(stderr, "%s: need %zu bytes, have %zu\n", ctx, need, have);
        return 0;
    }
    return 1;
}

static uint32_t read_be32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int ptr_seen(bool*** seen, size_t seen_count, bool** ptr) {
    for (size_t i = 0; i < seen_count; i++) {
        if (seen[i] == ptr) return 1;
    }
    return 0;
}

CollisionMasks* collision_masks_load(const char* tf_path) {
    ResourceContainer* tf = resource_load(tf_path);
    if (!tf || tf->count < 2) {
        if (tf) resource_free(tf);
        return NULL;
    }

    size_t obj0_size = 0;
    const uint8_t* obj0 = resource_get_element(tf, 0, &obj0_size);
    size_t obj1_size = 0;
    const uint8_t* obj1 = resource_get_element(tf, 1, &obj1_size);
    if (!obj0 || !obj1) {
        resource_free(tf);
        return NULL;
    }

    const uint8_t* p0 = obj0;
    const uint8_t* end0 = obj0 + obj0_size;
    const uint8_t* p1 = obj1;
    const uint8_t* end1 = obj1 + obj1_size;

    // Header (object 0, 14 bytes)
    if (!require_bytes("collision_masks_load(header)", p0, end0, 14)) {
        resource_free(tf);
        return NULL;
    }
    uint8_t tileW = p0[4];
    uint8_t tileH = p0[5];
    uint8_t tileCount = p0[6];
    uint8_t splitIndex = p0[7];
    p0 += 14;

    // Normalize size (g.java:167)
    uint8_t maskW = (tileW == 12) ? 16 : tileW;
    uint8_t maskH = (tileH == 12) ? 16 : tileH;

    // Skip animation table (g.java:172-180)
    if (!require_bytes("collision_masks_load(anim hdr)", p0, end0, 2)) {
        resource_free(tf);
        return NULL;
    }
    uint8_t animCount = p0[0];
    p0 += 2;
    for (uint8_t i = 0; i < animCount; i++) {
        if (!require_bytes("collision_masks_load(anim entry)", p0, end0, 3)) {
            resource_free(tf);
            return NULL;
        }
        uint8_t frameCount = p0[2];
        p0 += 3;
        if (!require_bytes("collision_masks_load(anim frames)", p0, end0, frameCount)) {
            resource_free(tf);
            return NULL;
        }
        p0 += frameCount;
    }

    CollisionMasks* masks = (CollisionMasks*)calloc(1, sizeof(CollisionMasks));
    if (!masks) {
        resource_free(tf);
        return NULL;
    }
    masks->tile_count = tileCount;
    masks->mask_w = maskW;
    masks->mask_h = maskH;
    masks->masks = (bool***)calloc(tileCount, sizeof(bool**));
    if (!masks->masks) {
        free(masks);
        resource_free(tf);
        return NULL;
    }

    for (uint8_t tileId = 0; tileId < tileCount; tileId++) {
        const uint8_t** p = (tileId < splitIndex) ? &p0 : &p1;
        const uint8_t* end = (tileId < splitIndex) ? end0 : end1;

        if (!require_bytes("collision_masks_load(tile fixed)", *p, end, 5)) {
            collision_masks_free(masks);
            resource_free(tf);
            return NULL;
        }
        uint8_t collisionType = (*p)[4];
        *p += 5;

        if (collisionType == 1) {
            bool** tile_mask = (bool**)malloc(maskW * sizeof(bool*));
            if (!tile_mask) {
                collision_masks_free(masks);
                resource_free(tf);
                return NULL;
            }
            for (int x = 0; x < maskW; x++) {
                tile_mask[x] = (bool*)malloc(maskH * sizeof(bool));
                if (!tile_mask[x]) {
                    for (int j = 0; j < x; j++) free(tile_mask[j]);
                    free(tile_mask);
                    collision_masks_free(masks);
                    resource_free(tf);
                    return NULL;
                }
            }

            if (!require_bytes("collision_masks_load(mask bytes)", *p, end, (size_t)maskW * (size_t)maskH)) {
                for (int x = 0; x < maskW; x++) free(tile_mask[x]);
                free(tile_mask);
                collision_masks_free(masks);
                resource_free(tf);
                return NULL;
            }

            for (int y = 0; y < maskH; y++) {
                for (int x = 0; x < maskW; x++) {
                    tile_mask[x][y] = ((*p)[0] != 0);
                    (*p)++;
                }
            }

            masks->masks[tileId] = tile_mask;
        }

        if (!require_bytes("collision_masks_load(aux)", *p, end, 4)) {
            collision_masks_free(masks);
            resource_free(tf);
            return NULL;
        }
        uint32_t aux = read_be32(*p);
        *p += 4;

        if (collisionType == 3) {
            uint8_t base = (uint8_t)(aux & 0xFF);
            if (base >= tileCount) {
                collision_masks_free(masks);
                resource_free(tf);
                return NULL;
            }
            masks->masks[tileId] = masks->masks[base];
        }
    }

    resource_free(tf);
    return masks;
}

void collision_masks_free(CollisionMasks* masks) {
    if (!masks) return;

    if (masks->masks) {
        bool*** seen = (bool***)calloc(masks->tile_count, sizeof(bool**));
        size_t seen_count = 0;

        for (uint8_t tid = 0; tid < masks->tile_count; tid++) {
            bool** tile_mask = masks->masks[tid];
            if (!tile_mask) continue;
            if (seen && ptr_seen(seen, seen_count, tile_mask)) continue;

            if (seen) seen[seen_count++] = tile_mask;

            for (int x = 0; x < masks->mask_w; x++) {
                free(tile_mask[x]);
            }
            free(tile_mask);
        }

        free(seen);
        free(masks->masks);
    }
    free(masks);
}

