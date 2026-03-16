/**
 * Foreground overlay pass implementation (front tiles + hoop anchors).
 */

#include "foreground_pass.h"
#include "tile_transform.h"

#include <SDL2/SDL.h>
#include <stdlib.h>

/* ic[3..6] hoop sprites — borrowed from IcAssets, NOT owned here. */
static SDL_Texture* g_hoop_tex[4] = {NULL, NULL, NULL, NULL};

static int front_tile_uses_raw_orientation(uint8_t tile_id) {
    /* Local visual fix: turbine family tiles in /res/tf carry non-zero transform,
     * but the raw if0 textures already match the intended orientation.
     * Applying tf.transform rotates the vertical 2x6 turbine incorrectly.
     */
    return ((tile_id >= 52 && tile_id <= 61) || (tile_id >= 66 && tile_id <= 72));
}

int foreground_pass_init(const IcAssets* ic) {
    if (!ic) return -1;
    for (int i = 0; i < 4; i++) {
        if (!ic->hoop[i]) return -1;
        g_hoop_tex[i] = ic->hoop[i];
    }
    return 0;
}

void foreground_pass_shutdown(void) {
    /* Textures owned by GameAssets — only null out borrowed pointers. */
    for (int i = 0; i < 4; i++)
        g_hoop_tex[i] = NULL;
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
            /* hoop anchors — h.java:331: if (n == 93 || n == 94 || n == 97 || n == 101)
             * 95/96/99/103 never enter h[]; their switch cases in Java are dead code. */
            if ((tile_id == 93 || tile_id == 94 || tile_id == 97 || tile_id == 101) &&
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
        if (meta.transform != 0 && !front_tile_uses_raw_orientation(tile_id)) {
            /* Convert Nokia tf b-byte to SDL draw_param.
             *
             * Nokia DirectGraphics (Nokia_UI_API_1_1):
             *   - Angles are CCW-positive: ROTATE_90=90°CCW, ROTATE_270=270°CCW.
             *   - Operation order: rotate first, then flipV, then flipH.
             *   - g.java p[] = {0,270,180,90,16384,16654,16564,16474,8192,8462,8372,8282}
             *
             * SDL2 (SDL_render.c SDL_RenderCopyExF):
             *   - Angles are CW-positive in screen coords (y-down): angle=90 → 90°CW.
             *   - Operation order: flip vertex coords first, then apply rotation matrix.
             *
             * Because the orders differ, combined flip+rotation cases are NOT a simple bit
             * re-map. Derived from full pixel-map (x,y→x',y') for all 12 b-values:
             *
             *  b  Nokia p[b]   Nokia meaning       (x,y)→         SDL draw_param
             *  0    0          —                   (x,y)          0x00
             *  1  270          ROT270CCW=90CW       (15-y, x)      0x03 (rot3,90°CW)
             *  2  180          ROT180               (15-x,15-y)    0x02 (rot2,180°)
             *  3   90          ROT90CCW=270CW       (y, 15-x)      0x01 (rot1,270°CW)
             *  4  16384        FLIP_V               (x, 15-y)      0x04 (flipV)
             *  5  16654        FLIP_V|ROT270CCW     (15-y,15-x)    0x05 (flipV+rot1)
             *  6  16564        FLIP_V|ROT180 ≡FLIPH (15-x, y)      0x08 (flipH)
             *  7  16474        FLIP_V|ROT90CCW      (y, x)         0x07 (flipV+rot3)
             *  8   8192        FLIP_H               (15-x, y)      0x08 (flipH)
             *  9   8462        FLIP_H|ROT270CCW     (y, x)         0x09 (flipH+rot1)
             * 10   8372        FLIP_H|ROT180 ≡FLIPV (x, 15-y)      0x04 (flipV)
             * 11   8282        FLIP_H|ROT90CCW      (15-y,15-x)    0x05 (flipV+rot1)
             */
            static const uint8_t nokia_b_to_draw_param[12] = {
                0x00, /* b= 0 */
                0x03, /* b= 1 ROT270CCW */
                0x02, /* b= 2 ROT180    */
                0x01, /* b= 3 ROT90CCW  */
                0x04, /* b= 4 FLIPV              */
                0x05, /* b= 5 FLIPV|ROT270CCW    */
                0x08, /* b= 6 FLIPV|ROT180≡FLIPH */
                0x07, /* b= 7 FLIPV|ROT90CCW     */
                0x08, /* b= 8 FLIPH              */
                0x09, /* b= 9 FLIPH|ROT270CCW    */
                0x04, /* b=10 FLIPH|ROT180≡FLIPV */
                0x05, /* b=11 FLIPH|ROT90CCW     */
            };
            uint8_t b = meta.transform;
            uint8_t draw_param = (b < 12u) ? nokia_b_to_draw_param[b] : 0x00;
            draw_tile_with_transform(renderer, tex, &dest, draw_param);
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
        SDL_Texture* htex = NULL;
        SDL_Rect dest = {0};
        switch (tid) {
            case 93: case 95:
                /* drawImage(x[0/1], sx, sy-1, 20) — no transform, 16x18 */
                htex = g_hoop_tex[(tid == 93) ? 0 : 1];
                if (!htex) break;
                dest = (SDL_Rect){ sx, sy - 1, 16, 18 };
                SDL_RenderCopy(renderer, htex, NULL, &dest);
                break;
            case 94: case 96:
                /* drawImage(x[0/1], sx-1, sy, 20, 270) — Nokia 270 => SDL 90cw */
                /* Rotating 16x18 produces an 18x16 footprint. */
                htex = g_hoop_tex[(tid == 94) ? 0 : 1];
                if (!htex) break;
                dest = (SDL_Rect){ sx - 1, sy, 18, 16 };
                draw_tile_with_transform(renderer, htex, &dest, 0x03);
                break;
            case 97: case 99:
                /* top half:    drawImage(x[2/3], sx, sy,    20, 8192)  — flipX */
                /* bottom half: drawImage(x[2/3], sx, sy+16, 20, 180)   — rot180 */
                htex = g_hoop_tex[(tid == 97) ? 2 : 3];
                if (!htex) break;
                dest = (SDL_Rect){ sx, sy, 16, 16 };
                draw_tile_with_transform(renderer, htex, &dest, 0x08);
                dest.y += 16;
                draw_tile_with_transform(renderer, htex, &dest, 0x02);
                break;
            case 101: case 103:
                /* left half:  Nokia 8462 = R270ccw then H = SDL angle=270cw + flipH = 0x09 */
                /* right half: Nokia 270  = R270ccw       = SDL angle=90cw             = 0x03 */
                htex = g_hoop_tex[(tid == 101) ? 2 : 3];
                if (!htex) break;
                dest = (SDL_Rect){ sx, sy, 16, 16 };
                draw_tile_with_transform(renderer, htex, &dest, 0x09);
                dest.x += 16;
                draw_tile_with_transform(renderer, htex, &dest, 0x03);
                break;
        }
    }
}
