/**
 * Tile animation implementation - Step 07.
 */

#include "tile_animation.h"
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

TileAnimation* animation_load(const char* tf_path) {
    ResourceContainer* tf = resource_load(tf_path);
    if (!tf || tf->count < 1) {
        if (tf) resource_free(tf);
        return NULL;
    }

    size_t obj0_size = 0;
    const uint8_t* obj0 = resource_get_element(tf, 0, &obj0_size);
    if (!obj0) {
        resource_free(tf);
        return NULL;
    }

    const uint8_t* p = obj0;
    const uint8_t* end = obj0 + obj0_size;

    // Skip /res/tf header (14 bytes)
    if (!require_bytes("animation_load(header)", p, end, 14)) {
        resource_free(tf);
        return NULL;
    }
    p += 14;

    // animCount (g.java:186 - НЕТ глобального unused!)
    if (!require_bytes("animation_load(animCount)", p, end, 1)) {
        resource_free(tf);
        return NULL;
    }
    uint8_t animCount = p[0];
    p += 1;

    TileAnimation* anim = (TileAnimation*)calloc(1, sizeof(TileAnimation));
    if (!anim) {
        resource_free(tf);
        return NULL;
    }
    anim->count = animCount;

    if (animCount == 0) {
        resource_free(tf);
        return anim;
    }

    anim->frames = (uint8_t**)calloc(animCount, sizeof(uint8_t*));
    anim->period = (uint8_t*)calloc(animCount, sizeof(uint8_t));
    anim->frame_index = (uint8_t*)calloc(animCount, sizeof(uint8_t));
    anim->timer = (uint8_t*)calloc(animCount, sizeof(uint8_t));
    anim->frame_counts = (uint8_t*)calloc(animCount, sizeof(uint8_t));
    if (!anim->frames || !anim->period || !anim->frame_index || !anim->timer || !anim->frame_counts) {
        animation_free(anim);
        resource_free(tf);
        return NULL;
    }

    for (uint8_t group = 0; group < animCount; group++) {
        if (!require_bytes("animation_load(group hdr)", p, end, 3)) {
            animation_free(anim);
            resource_free(tf);
            return NULL;
        }

        uint8_t reserved = p[0];
        (void)reserved;
        uint8_t period = p[1];
        uint8_t frameCount = p[2];
        p += 3;

        if (!require_bytes("animation_load(frames)", p, end, frameCount)) {
            animation_free(anim);
            resource_free(tf);
            return NULL;
        }

        uint8_t* frames = (uint8_t*)malloc(frameCount);
        if (!frames) {
            animation_free(anim);
            resource_free(tf);
            return NULL;
        }
        for (uint8_t i = 0; i < frameCount; i++) {
            frames[i] = p[i];
        }
        p += frameCount;

        anim->frames[group] = frames;
        anim->period[group] = period;
        anim->frame_counts[group] = frameCount;
        anim->frame_index[group] = 0;
        anim->timer[group] = period;
    }

    resource_free(tf);
    return anim;
}

void animation_free(TileAnimation* anim) {
    if (!anim) return;
    if (anim->frames) {
        for (uint8_t i = 0; i < anim->count; i++) {
            free(anim->frames[i]);
        }
        free(anim->frames);
    }
    free(anim->period);
    free(anim->frame_index);
    free(anim->timer);
    free(anim->frame_counts);
    free(anim);
}

void animation_tick(TileAnimation* anim) {
    if (!anim || anim->count == 0) return;

    for (int i = 0; i < anim->count; i++) {
        anim->timer[i]--;
        if (anim->timer[i] == 0) {
            anim->timer[i] = anim->period[i];
            anim->frame_index[i]++;
            if (anim->frame_index[i] >= anim->frame_counts[i]) {
                anim->frame_index[i] = 0;
            }
        }
    }
}

uint8_t animation_get_tile(TileAnimation* anim, TileMetadata* tile_meta, uint8_t tile_id) {
    if (!anim || !tile_meta) return tile_id;

    TileMetadata* tm = &tile_meta[tile_id];
    if (tm->render_type != 3) return tile_id;

    int group = tm->aux;
    if (group < 0 || group >= anim->count) return tile_id;

    int frame = anim->frame_index[group];
    if (frame < 0 || frame >= anim->frame_counts[group]) return tile_id;

    return anim->frames[group][frame];
}

