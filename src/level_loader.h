/**
 * Level loader for Bounce Back (/res/lf)
 *
 * Parses a single level's metadata + tileMap from the J2ME container resource.
 *
 * Spec: docs/STEP_02_TILE_ENGINE.md
 * Reference: docs/DEOBFUSCATION.md ("Формат данных уровня (/res/lf)")
 */

#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <stdint.h>

typedef struct {
    uint8_t theme_id;
    uint8_t spawn_x;
    uint8_t spawn_y;
    uint8_t ball_type;
    uint8_t width;
    uint8_t height;
    uint8_t* tile_map; // [height * width] bytes, row-major
} Level;

Level* level_load(const char* lf_path, int level_index);
void level_free(Level* level);
uint8_t level_get_tile(const Level* level, int tile_x, int tile_y);

#endif // LEVEL_LOADER_H

