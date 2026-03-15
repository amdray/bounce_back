/**
 * Foreground overlay pass implementation (front tiles + hoop anchors).
 */

#include "foreground_pass.h"
#include "tile_transform.h"
#include "resource_loader.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>

/* ic[3..6] — hoop sprites: x[0]=ic3=16x18, x[1]=ic4=16x18, x[2]=ic5=16x16, x[3]=ic6=16x16 */
static SDL_Texture*      g_hoop_tex[4]  = {NULL, NULL, NULL, NULL};
static ResourceContainer* g_hoop_ic     = NULL;

int foreground_pass_init(SDL_Renderer* renderer) {
    if (!renderer) return -1;

    g_hoop_ic = resource_load("res/ic");
    if (!g_hoop_ic) return -1;

    for (int i = 0; i < 4; i++) {
        size_t sz = 0;
        const uint8_t* data = resource_get_element(g_hoop_ic, 3 + i, &sz);
        if (!data || sz == 0) goto fail;

        SDL_RWops* rw = SDL_RWFromConstMem(data, (int)sz);
        if (!rw) goto fail;

        SDL_Surface* surf = IMG_Load_RW(rw, 1);
        if (!surf) goto fail;

        g_hoop_tex[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        if (!g_hoop_tex[i]) goto fail;
    }
    return 0;

fail:
    foreground_pass_shutdown();
    return -1;
}

void foreground_pass_shutdown(void) {
    for (int i = 0; i < 4; i++) {
        if (g_hoop_tex[i]) {
            SDL_DestroyTexture(g_hoop_tex[i]);
            g_hoop_tex[i] = NULL;
        }
    }
    if (g_hoop_ic) {
        resource_free(g_hoop_ic);
        g_hoop_ic = NULL;
    }
}

int foreground_pass_build(const Level* level, ForegroundPass* out) {
    if (!level || !out) return -1;

    out->front_tiles = NULL;
    out->front_count = 0;
    out->hoop_count  = 0;

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
            /* hoop anchors — hardcoded per-tile transforms in overlay (h.java first loop) */
            if ((tile_id == 93 || tile_id == 94 || tile_id == 95 || tile_id == 96 ||
                 tile_id == 97 || tile_id == 99 || tile_id == 101 || tile_id == 103) &&
                out->hoop_count < 128) {
                out->hoop_tiles[out->hoop_count++] = (TileCoord){ (uint8_t)x, (uint8_t)y };
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

    /* --- hoop anchor overlay ---
     * Source: h.java a(Graphics,DirectGraphics), first loop (tileId 93-103).
     * Transforms are hardcoded per tile_id — NOT from tf b[tileId].
     *
     * Sprites (h.java this.x[] = ic[3..6]):
     *   x[0]=ic[3]=16x18  x[1]=ic[4]=16x18  (small hoops, variant A/B)
     *   x[2]=ic[5]=16x16  x[3]=ic[6]=16x16  (large hoops, variant A/B)
     *
     * Transform bits (tile_transform.c): bit3=flipX, bit2=flipY, bits1-0={1=270,2=180,3=90}
     *   0x00 = none   0x01 = rot270   0x02 = rot180   0x08 = flipX   0x09 = flipX+rot270
     */
    for (int i = 0; i < pass->hoop_count; i++) {
        int tx = (int)pass->hoop_tiles[i].tile_x;
        int ty = (int)pass->hoop_tiles[i].tile_y;
        uint8_t tid = (uint8_t)(level_get_tile(level, tx, ty) & 0x7F);
        int sx = tx * 16 - camera_x;
        int sy = ty * 16 - camera_y;
        SDL_Texture* htex;
        SDL_Rect dest;
        switch (tid) {
            case 93: case 95:
                /* drawImage(x[0/1], sx, sy-1, 20) — no transform, 16x18 */
                htex = g_hoop_tex[(tid == 93) ? 0 : 1];
                dest = (SDL_Rect){ sx, sy - 1, 16, 18 };
                SDL_RenderCopy(renderer, htex, NULL, &dest);
                break;
            case 94: case 96:
                /* drawImage(x[0/1], sx-1, sy, 20, 270) — Nokia 270 => SDL 90cw */
                /* Rotating 16x18 produces an 18x16 footprint. */
                htex = g_hoop_tex[(tid == 94) ? 0 : 1];
                dest = (SDL_Rect){ sx - 1, sy, 18, 16 };
                draw_tile_with_transform(renderer, htex, &dest, 0x03);
                break;
            case 97: case 99:
                /* top half:    drawImage(x[2/3], sx, sy,    20, 8192)  — flipX */
                /* bottom half: drawImage(x[2/3], sx, sy+16, 20, 180)   — rot180 */
                htex = g_hoop_tex[(tid == 97) ? 2 : 3];
                dest = (SDL_Rect){ sx, sy, 16, 16 };
                draw_tile_with_transform(renderer, htex, &dest, 0x08);
                dest.y += 16;
                draw_tile_with_transform(renderer, htex, &dest, 0x02);
                break;
            case 101: case 103:
                /* left half:  Nokia 8462 = R270ccw then H = SDL angle=270cw + flipH = 0x09 */
                /* right half: Nokia 270  = R270ccw       = SDL angle=90cw             = 0x03 */
                htex = g_hoop_tex[(tid == 101) ? 2 : 3];
                dest = (SDL_Rect){ sx, sy, 16, 16 };
                draw_tile_with_transform(renderer, htex, &dest, 0x09);
                dest.x += 16;
                draw_tile_with_transform(renderer, htex, &dest, 0x03);
                break;
        }
    }
}
