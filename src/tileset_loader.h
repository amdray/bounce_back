/**
 * Tileset loader for Bounce Back (/res/if*).
 *
 * Spec: docs/STEP_02_TILE_ENGINE.md
 * Reference: docs/DEOBFUSCATION.md (table /res resources)
 */

#ifndef TILESET_LOADER_H
#define TILESET_LOADER_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Texture** textures; // [image_index]
    int count;              // number of base images
} Tileset;

Tileset* tileset_load(SDL_Renderer* renderer, const char* if0_path, const char* theme_path);
void tileset_free(Tileset* tileset);
SDL_Texture* tileset_get(const Tileset* tileset, int image_index);

#endif // TILESET_LOADER_H
