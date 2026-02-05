/**
 * Level renderer for Bounce Back (Step 2).
 *
 * Spec: docs/STEP_02_TILE_ENGINE.md
 */

#ifndef LEVEL_RENDERER_H
#define LEVEL_RENDERER_H

#include <SDL2/SDL.h>
#include "level_loader.h"
#include "tile_metadata.h"
#include "tile_animation.h"
#include "tileset_loader.h"

typedef struct {
    Level* level;
    TileMetadata* tile_meta; // array indexed by tileId
    TileAnimation* tile_anim;
    Tileset* tileset;
} LevelRenderer;

LevelRenderer* renderer_create(Level* level, TileMetadata* meta, TileAnimation* tile_anim, Tileset* tileset);
void renderer_free(LevelRenderer* renderer);
void renderer_draw(LevelRenderer* lr, SDL_Renderer* r, int camera_x, int camera_y);

#endif // LEVEL_RENDERER_H
