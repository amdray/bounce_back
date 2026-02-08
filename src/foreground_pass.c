/**
 * Foreground overlay pass implementation (front tiles only).
 */

#include "foreground_pass.h"
#include "tile_transform.h"

#include <SDL2/SDL.h>
#include <stdlib.h>

int foreground_pass_build(const Level* level, ForegroundPass* out) {
    if (!level || !out) return -1;

    out->front_tiles = NULL;
    out->front_count = 0;

    int front_count = 0;
    for (int y = 0; y < (int)level->height; y++) {
        for (int x = 0; x < (int)level->width; x++) {
            uint8_t tile_id = (uint8_t)(level_get_tile(level, x, y) & 0x7F);
            if ((tile_id >= 52 && tile_id <= 61) || (tile_id >= 66 && tile_id <= 72)) {
                front_count++;
            }
        }
    }

    if (front_count > 0) {
        out->front_tiles = (TileCoord*)malloc((size_t)front_count * sizeof(TileCoord));
        if (!out->front_tiles) return -1;
    }

    int front_i = 0;
    for (int y = 0; y < (int)level->height; y++) {
        for (int x = 0; x < (int)level->width; x++) {
            uint8_t tile_id = (uint8_t)(level_get_tile(level, x, y) & 0x7F);
            if ((tile_id >= 52 && tile_id <= 61) || (tile_id >= 66 && tile_id <= 72)) {
                out->front_tiles[front_i++] = (TileCoord){ (uint8_t)x, (uint8_t)y };
            }
        }
    }

    out->front_count = front_count;
    return 0;
}

void foreground_pass_draw(SDL_Renderer* renderer,
                          const Level* level,
                          TileMetadata* tile_meta,
                          TileAnimation* tile_anim,
                          const Tileset* tileset,
                          const ForegroundPass* pass,
                          int camera_x,
                          int camera_y) {
    if (!renderer || !level || !tile_meta || !tile_anim || !tileset || !pass) return;

    for (int i = 0; i < pass->front_count; i++) {
        int tile_x = (int)pass->front_tiles[i].tile_x;
        int tile_y = (int)pass->front_tiles[i].tile_y;

        uint8_t tile_id = (uint8_t)(level_get_tile(level, tile_x, tile_y) & 0x7F);
        uint8_t display_tile = animation_get_tile(tile_anim, tile_meta, tile_id);
        TileMetadata meta = tile_meta[display_tile];

        if (meta.render_type == 0) continue;
        if (meta.render_type == 3) continue;

        SDL_Texture* tex = tileset_get(tileset, meta.image_index);
        if (!tex) continue;

        int screen_x = tile_x * 16 - camera_x;
        int screen_y = tile_y * 16 - camera_y;
        SDL_Rect dest = { screen_x, screen_y, 16, 16 };
        if (meta.transform != 0) {
            draw_tile_with_transform(renderer, tex, &dest, meta.transform);
        } else {
            SDL_RenderCopy(renderer, tex, NULL, &dest);
        }
    }
}
