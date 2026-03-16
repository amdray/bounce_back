/**
 * Tile metadata loader implementation (/res/tf).
 */

#include "tile_metadata.h"
#include "endian_utils.h"
#include "resource_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t g_tile_flag_mask = 0;
static uint32_t g_bg_color_rgb = 0;

static int require_bytes(const char* ctx, const uint8_t* p, const uint8_t* end, size_t need) {
    size_t have = (size_t)(end - p);
    if (have < need) {
        fprintf(stderr, "%s: need %zu bytes, have %zu\n", ctx, need, have);
        return 0;
    }
    return 1;
}

TileMetadata* tilemetadata_load(const char* tf_path) {
    if (!tf_path) {
        fprintf(stderr, "tilemetadata_load: NULL tf_path\n");
        return NULL;
    }

    ResourceContainer* tf = resource_load(tf_path);
    if (!tf) return NULL;
    if (tf->count < 2) {
        fprintf(stderr, "tilemetadata_load: expected 2 objects in '%s', got %u\n", tf_path, tf->count);
        resource_free(tf);
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

    // Header (object 0)
    if (!require_bytes("tilemetadata_load(header)", p0, end0, 14)) {
        resource_free(tf);
        return NULL;
    }
    uint8_t images_total = p0[0];
    uint8_t images_base = p0[1];
    uint8_t clampX = p0[2];
    uint8_t clampY = p0[3];
    uint8_t tileW = p0[4];
    uint8_t tileH = p0[5];
    uint8_t tileCount = p0[6];
    uint8_t splitIndex = p0[7];
    uint8_t tileIdMask = p0[8];
    uint8_t tileFlagMask = p0[9];
    uint32_t bgColor = endian_read_be32_u32(p0 + 10);
    (void)images_total;
    (void)images_base;
    (void)clampX;
    (void)clampY;
    (void)tileIdMask;
    g_tile_flag_mask = tileFlagMask;
    g_bg_color_rgb = bgColor & 0x00FFFFFFu;
    p0 += 14;

    // Normalize tile size (g.java normalizes 12 -> 16)
    uint8_t maskW = (tileW == 12) ? 16 : tileW;
    uint8_t maskH = (tileH == 12) ? 16 : tileH;

    // Animation table (object 0, immediately after header)
    if (!require_bytes("tilemetadata_load(anim hdr)", p0, end0, 1)) {
        resource_free(tf);
        return NULL;
    }
    uint8_t animCount = p0[0];
    p0 += 1;  // Only animCount, NO global unused (g.java:186)
    
    for (uint8_t i = 0; i < animCount; i++) {
        if (!require_bytes("tilemetadata_load(anim entry)", p0, end0, 3)) {
            resource_free(tf);
            return NULL;
        }
        uint8_t unused = p0[0];
        uint8_t period = p0[1];
        uint8_t frameCount = p0[2];
        (void)unused;
        (void)period;
        p0 += 3;
        if (!require_bytes("tilemetadata_load(anim frames)", p0, end0, frameCount)) {
            resource_free(tf);
            return NULL;
        }
        p0 += frameCount;
    }

    TileMetadata* meta = (TileMetadata*)calloc(128, sizeof(TileMetadata));
    if (!meta) {
        fprintf(stderr, "tilemetadata_load: calloc failed\n");
        resource_free(tf);
        return NULL;
    }

    for (uint8_t tileId = 0; tileId < tileCount; tileId++) {
        const uint8_t** p = (tileId < splitIndex) ? &p0 : &p1;
        const uint8_t* end = (tileId < splitIndex) ? end0 : end1;

        if (!require_bytes("tilemetadata_load(tile fixed)", *p, end, 5)) {
            free(meta);
            resource_free(tf);
            return NULL;
        }

        int8_t tileIdEcho = (int8_t)(*p)[0];
        (void)tileIdEcho;
        uint8_t renderType = (*p)[1];
        uint8_t imageIndex = (*p)[2];
        uint8_t transform = (*p)[3];
        uint8_t collisionType = (*p)[4];
        *p += 5;

        if (collisionType == 1) {
            size_t maskBytes = (size_t)maskW * (size_t)maskH;
            if (!require_bytes("tilemetadata_load(mask)", *p, end, maskBytes)) {
                free(meta);
                resource_free(tf);
                return NULL;
            }
            *p += maskBytes;
        }

        if (!require_bytes("tilemetadata_load(aux)", *p, end, 4)) {
            free(meta);
            resource_free(tf);
            return NULL;
        }
        int32_t aux_i32 = endian_read_be32_i32(*p);
        *p += 4;

        meta[tileId].render_type = renderType;
        meta[tileId].image_index = imageIndex;
        meta[tileId].transform = transform;
        meta[tileId].collision_type = collisionType;
        meta[tileId].aux = aux_i32;
    }

    resource_free(tf);
    return meta;
}

void tilemetadata_free(TileMetadata* meta) {
    free(meta);
}

uint8_t tilemetadata_flag_mask(void) {
    return g_tile_flag_mask;
}

uint32_t tilemetadata_bg_color_rgb(void) {
    return g_bg_color_rgb;
}
