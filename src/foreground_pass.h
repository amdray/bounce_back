/**
 * Foreground overlay pass (front tiles only).
 */

#ifndef FOREGROUND_PASS_H
#define FOREGROUND_PASS_H

#include <SDL2/SDL.h>
#include <stdint.h>

#include "level_loader.h"
#include "tile_animation.h"
#include "tile_metadata.h"
#include "tileset_loader.h"

typedef struct {
    uint8_t tile_x;
    uint8_t tile_y;
} TileCoord;

typedef struct {
    TileCoord* front_tiles;
    int front_count;
} ForegroundPass;

int foreground_pass_build(const Level* level, ForegroundPass* out);

void foreground_pass_draw(SDL_Renderer* renderer,
                          const Level* level,
                          TileMetadata* tile_meta,
                          TileAnimation* tile_anim,
                          const Tileset* tileset,
                          const ForegroundPass* pass,
                          int camera_x,
                          int camera_y);

#endif // FOREGROUND_PASS_H
