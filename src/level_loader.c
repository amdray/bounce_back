/**
 * Level loader implementation (/res/lf).
 */

#include "level_loader.h"
#include "resource_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int require_size_at_least(const char* ctx, size_t size, size_t need) {
    if (size < need) {
        fprintf(stderr, "%s: buffer too small: have=%zu need=%zu\n", ctx, size, need);
        return 0;
    }
    return 1;
}

Level* level_load(const char* lf_path, int level_index) {
    if (!lf_path) {
        fprintf(stderr, "level_load: NULL lf_path\n");
        return NULL;
    }
    if (level_index < 0) {
        fprintf(stderr, "level_load: invalid level_index=%d\n", level_index);
        return NULL;
    }

    ResourceContainer* lf = resource_load(lf_path);
    if (!lf) return NULL;

    int meta_index = 2 * level_index;
    int map_index = 2 * level_index + 1;
    if (meta_index < 0 || map_index < 0 || meta_index >= lf->count || map_index >= lf->count) {
        fprintf(stderr, "level_load: level_index=%d out of range for lf count=%u\n",
                level_index, lf->count);
        resource_free(lf);
        return NULL;
    }

    size_t meta_size = 0;
    const uint8_t* meta = resource_get_element(lf, meta_index, &meta_size);
    if (!meta) {
        resource_free(lf);
        return NULL;
    }
    if (!require_size_at_least("level_load(meta)", meta_size, 7)) {
        resource_free(lf);
        return NULL;
    }

    uint8_t theme_id = meta[0];
    uint8_t spawn_y = meta[1];
    uint8_t spawn_x = meta[2];
    uint8_t ball_type = meta[3];
    uint8_t enemy_count = meta[6];

    size_t enemies_bytes = (size_t)enemy_count * 9;
    size_t meta_need = 7 + enemies_bytes;
    if (!require_size_at_least("level_load(meta+enemies)", meta_size, meta_need)) {
        resource_free(lf);
        return NULL;
    }

    size_t map_size = 0;
    const uint8_t* map = resource_get_element(lf, map_index, &map_size);
    if (!map) {
        resource_free(lf);
        return NULL;
    }
    if (!require_size_at_least("level_load(tilemap)", map_size, 2)) {
        resource_free(lf);
        return NULL;
    }

    uint8_t height = map[0];
    uint8_t width = map[1];
    size_t tiles_count = (size_t)height * (size_t)width;
    if (!require_size_at_least("level_load(tilemap bytes)", map_size, 2 + tiles_count)) {
        resource_free(lf);
        return NULL;
    }

    Level* level = (Level*)calloc(1, sizeof(Level));
    if (!level) {
        fprintf(stderr, "level_load: calloc failed\n");
        resource_free(lf);
        return NULL;
    }

    level->theme_id = theme_id;
    level->spawn_x = spawn_x;
    level->spawn_y = spawn_y;
    level->ball_type = ball_type;
    level->width = width;
    level->height = height;

    level->tile_map = (uint8_t*)malloc(tiles_count);
    if (!level->tile_map) {
        fprintf(stderr, "level_load: malloc failed for tile_map (%zu bytes)\n", tiles_count);
        free(level);
        resource_free(lf);
        return NULL;
    }
    memcpy(level->tile_map, map + 2, tiles_count);

    resource_free(lf);
    return level;
}

void level_free(Level* level) {
    if (!level) return;
    free(level->tile_map);
    free(level);
}

uint8_t level_get_tile(const Level* level, int tile_x, int tile_y) {
    if (!level || !level->tile_map) return 0;
    if (tile_x < 0 || tile_y < 0) return 0;
    if (tile_x >= (int)level->width || tile_y >= (int)level->height) return 0;
    return level->tile_map[(size_t)tile_y * (size_t)level->width + (size_t)tile_x];
}

