/**
 * Level renderer implementation (Step 06: camera + visible area).
 */

#include "level_renderer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define TILE_SIZE     16

LevelRenderer* renderer_create(Level* level, TileMetadata* meta, TileAnimation* tile_anim, Tileset* tileset) {
    if (!level || !meta || !tile_anim || !tileset) return NULL;
    LevelRenderer* r = (LevelRenderer*)calloc(1, sizeof(LevelRenderer));
    if (!r) return NULL;
    r->level = level;
    r->tile_meta = meta;
    r->tile_anim = tile_anim;
    r->tileset = tileset;
    return r;
}

void renderer_free(LevelRenderer* renderer) {
    free(renderer);
}

void renderer_draw(LevelRenderer* lr, SDL_Renderer* r, int camera_x, int camera_y) {
    if (!lr || !lr->level || !lr->tile_meta || !lr->tileset || !r) return;

    int startTileX = camera_x / TILE_SIZE;
    int endTileX = (camera_x + SCREEN_WIDTH - 1) / TILE_SIZE;
    int startTileY = camera_y / TILE_SIZE;
    int endTileY = (camera_y + SCREEN_HEIGHT - 1) / TILE_SIZE;

    for (int y = startTileY; y <= endTileY; y++) {
        for (int x = startTileX; x <= endTileX; x++) {
            if (x < 0 || y < 0) continue;
            if (x >= (int)lr->level->width || y >= (int)lr->level->height) continue;

            int screen_x = x * TILE_SIZE - camera_x;
            int screen_y = y * TILE_SIZE - camera_y;

            uint8_t tile_byte = level_get_tile(lr->level, x, y);
            uint8_t tile_id = (uint8_t)(tile_byte & 0x7F);
            bool bg_fill = (tile_byte & 0x80) != 0;
            (void)bg_fill;

            uint8_t display_tile = animation_get_tile(lr->tile_anim, lr->tile_meta, tile_id);
            TileMetadata meta = lr->tile_meta[display_tile];
            if (meta.render_type == 0) continue;
            if (meta.render_type == 3) continue;

            SDL_Texture* tex = tileset_get(lr->tileset, meta.image_index);
            if (!tex) continue;

            SDL_Rect dest = { screen_x, screen_y, TILE_SIZE, TILE_SIZE };
            SDL_RenderCopy(r, tex, NULL, &dest);
        }
    }
}
