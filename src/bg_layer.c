#include "bg_layer.h"

#include "endian_utils.h"
#include "resource_loader.h"
#include "texture_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int wrap_mod(int v, int mod) {
    if (mod <= 0) return 0;
    int r = v % mod;
    if (r < 0) r += mod;
    return r;
}

BgLayer* bg_layer_load(SDL_Renderer* renderer,
                       const char* bg_path,
                       const char* ib0_path,
                       const char* ib_theme_path) {
    if (!renderer || !bg_path || !ib0_path) return NULL;

    ResourceContainer* bg_res = resource_load(bg_path);
    if (!bg_res) return NULL;
    if (bg_res->count < 3) {
        resource_free(bg_res);
        return NULL;
    }

    size_t s0 = 0, s1 = 0, s2 = 0;
    const uint8_t* ch0 = resource_get_element(bg_res, 0, &s0);
    const uint8_t* ch1 = resource_get_element(bg_res, 1, &s1);
    const uint8_t* ch2 = resource_get_element(bg_res, 2, &s2);
    if (!ch0 || !ch1 || !ch2 || s0 < 4 + 15 || s2 < 4 + 2) {
        resource_free(bg_res);
        return NULL;
    }

    const uint8_t* p0 = ch0 + 4; // Java g(..., paramInt1=4) for /res/bg
    const uint8_t* end0 = ch0 + s0;
    const uint8_t* p1 = (s1 >= 4) ? (ch1 + 4) : ch1;
    const uint8_t* end1 = ch1 + s1;
    const uint8_t* p2 = ch2 + 4; // Java g(..., paramInt3=4) for map chunk
    const uint8_t* end2 = ch2 + s2;

    uint8_t images_total = p0[0];
    uint8_t images_base = p0[1];
    uint8_t clamp_x = p0[2];
    uint8_t clamp_y = p0[3];
    uint8_t tile_w = p0[4];
    uint8_t tile_h = p0[5];
    uint8_t tile_count = p0[6];
    uint8_t split_index = p0[7];
    uint8_t tile_id_mask = p0[8];
    uint8_t tile_flag_mask = p0[9];
    (void)endian_read_be32_i32(p0 + 10);
    p0 += 14;
    if (p0 >= end0) {
        resource_free(bg_res);
        return NULL;
    }

    uint8_t anim_count = *p0++;
    for (uint8_t i = 0; i < anim_count; i++) {
        if (p0 + 3 > end0) {
            resource_free(bg_res);
            return NULL;
        }
        uint8_t frame_count = p0[2];
        p0 += 3;
        if (p0 + frame_count > end0) {
            resource_free(bg_res);
            return NULL;
        }
        p0 += frame_count;
    }

    BgLayer* bg = (BgLayer*)calloc(1, sizeof(BgLayer));
    if (!bg) {
        resource_free(bg_res);
        return NULL;
    }

    bg->tile_w = tile_w;
    bg->tile_h = tile_h;
    bg->tile_count = tile_count;
    bg->clamp_x = (clamp_x != 0);
    bg->clamp_y = (clamp_y != 0);
    bg->tile_id_mask = tile_id_mask;
    bg->tile_flag_mask = tile_flag_mask;

    bg->meta = (BgTileMeta*)calloc((size_t)tile_count, sizeof(BgTileMeta));
    if (!bg->meta) {
        bg_layer_free(bg);
        resource_free(bg_res);
        return NULL;
    }

    for (uint8_t tile_id = 0; tile_id < tile_count; tile_id++) {
        const uint8_t** p = (tile_id < split_index) ? &p0 : &p1;
        const uint8_t* end = (tile_id < split_index) ? end0 : end1;
        if (*p + 9 > end) {
            bg_layer_free(bg);
            resource_free(bg_res);
            return NULL;
        }
        uint8_t render_type = (*p)[1];
        uint8_t image_index = (*p)[2];
        uint8_t transform = (*p)[3];
        uint8_t collision_type = (*p)[4];
        *p += 5;
        if (collision_type == 1) {
            size_t mask_bytes = (size_t)tile_w * (size_t)tile_h;
            if (*p + mask_bytes > end) {
                bg_layer_free(bg);
                resource_free(bg_res);
                return NULL;
            }
            *p += mask_bytes;
        }
        if (*p + 4 > end) {
            bg_layer_free(bg);
            resource_free(bg_res);
            return NULL;
        }
        int32_t aux = endian_read_be32_i32(*p);
        *p += 4;

        bg->meta[tile_id].render_type = render_type;
        bg->meta[tile_id].image_index = image_index;
        bg->meta[tile_id].transform = transform;
        bg->meta[tile_id].aux = aux;
    }

    if (p2 + 2 > end2) {
        bg_layer_free(bg);
        resource_free(bg_res);
        return NULL;
    }
    bg->height_tiles = p2[0];
    bg->width_tiles = p2[1];
    p2 += 2;
    size_t map_len = (size_t)bg->width_tiles * (size_t)bg->height_tiles;
    if (p2 + map_len > end2) {
        bg_layer_free(bg);
        resource_free(bg_res);
        return NULL;
    }
    bg->tile_map = (uint8_t*)malloc(map_len);
    if (!bg->tile_map) {
        bg_layer_free(bg);
        resource_free(bg_res);
        return NULL;
    }
    memcpy(bg->tile_map, p2, map_len);

    ResourceContainer* ib0 = resource_load(ib0_path);
    if (!ib0) {
        bg_layer_free(bg);
        resource_free(bg_res);
        return NULL;
    }
    ResourceContainer* ibt = NULL;
    if (ib_theme_path) {
        ibt = resource_load(ib_theme_path);
    }

    bg->texture_count = images_total;
    bg->textures = (SDL_Texture**)calloc((size_t)bg->texture_count, sizeof(SDL_Texture*));
    if (!bg->textures) {
        if (ibt) resource_free(ibt);
        resource_free(ib0);
        bg_layer_free(bg);
        resource_free(bg_res);
        return NULL;
    }

    for (int i = 0; i < (int)images_base && i < bg->texture_count; i++) {
        size_t sz = 0;
        const uint8_t* data = resource_get_element(ib0, i, &sz);
        bg->textures[i] = texture_loader_png_from_memory(renderer, data, sz);
    }

    if (ibt) {
        int theme_idx = 0;
        for (int i = (int)images_base; i < bg->texture_count; i++) {
            size_t sz = 0;
            const uint8_t* data = resource_get_element(ibt, theme_idx++, &sz);
            bg->textures[i] = texture_loader_png_from_memory(renderer, data, sz);
        }
        resource_free(ibt);
    }

    resource_free(ib0);
    resource_free(bg_res);
    return bg;
}

void bg_layer_free(BgLayer* bg) {
    if (!bg) return;
    if (bg->textures) {
        for (int i = 0; i < bg->texture_count; i++) {
            if (bg->textures[i]) SDL_DestroyTexture(bg->textures[i]);
        }
        free(bg->textures);
    }
    free(bg->meta);
    free(bg->tile_map);
    free(bg);
}

void bg_layer_draw(const BgLayer* bg,
                   SDL_Renderer* renderer,
                   int camera_x,
                   int camera_y,
                   int screen_w,
                   int screen_h) {
    if (!bg || !renderer || !bg->tile_map || !bg->meta) return;
    if (bg->tile_w <= 0 || bg->tile_h <= 0 || bg->width_tiles <= 0 || bg->height_tiles <= 0) return;

    int layer_w_px = bg->width_tiles * bg->tile_w;
    int layer_h_px = bg->height_tiles * bg->tile_h;

    int cam_x = camera_x;
    int cam_y = camera_y;

    if (bg->clamp_x) {
        int max_x = layer_w_px - screen_w;
        if (max_x < 0) max_x = 0;
        if (cam_x < 0) cam_x = 0;
        if (cam_x > max_x) cam_x = max_x;
    } else {
        cam_x = wrap_mod(cam_x, layer_w_px);
    }

    if (bg->clamp_y) {
        int max_y = layer_h_px - screen_h;
        if (max_y < 0) max_y = 0;
        if (cam_y < 0) cam_y = 0;
        if (cam_y > max_y) cam_y = max_y;
    } else {
        cam_y = wrap_mod(cam_y, layer_h_px);
    }

    int start_tile_x = cam_x / bg->tile_w;
    int start_tile_y = cam_y / bg->tile_h;
    int offset_x = -(cam_x % bg->tile_w);
    int offset_y = -(cam_y % bg->tile_h);
    int tiles_x = screen_w / bg->tile_w + 2;
    int tiles_y = screen_h / bg->tile_h + 2;

    const uint8_t r = 0x00;
    const uint8_t g = 0x71;
    const uint8_t b = 0xEF;

    for (int yy = 0; yy < tiles_y; yy++) {
        int map_y = start_tile_y + yy;
        if (bg->clamp_y) {
            if (map_y < 0 || map_y >= bg->height_tiles) continue;
        } else {
            map_y = wrap_mod(map_y, bg->height_tiles);
        }

        for (int xx = 0; xx < tiles_x; xx++) {
            int map_x = start_tile_x + xx;
            if (bg->clamp_x) {
                if (map_x < 0 || map_x >= bg->width_tiles) continue;
            } else {
                map_x = wrap_mod(map_x, bg->width_tiles);
            }

            int screen_x = offset_x + xx * bg->tile_w;
            int screen_y = offset_y + yy * bg->tile_h;
            SDL_Rect dst = { screen_x, screen_y, bg->tile_w, bg->tile_h };

            uint8_t tile_byte = bg->tile_map[(size_t)map_y * (size_t)bg->width_tiles + (size_t)map_x];
            uint8_t tile_id = (uint8_t)(tile_byte & bg->tile_id_mask);
            if (tile_id >= bg->tile_count) continue;

            if (bg->tile_flag_mask && (tile_byte & bg->tile_flag_mask)) {
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &dst);
            }

            BgTileMeta tm = bg->meta[tile_id];
            if (tm.render_type == 0) continue;
            if (tm.render_type == 2 || tm.render_type == 5) {
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &dst);
                continue;
            }
            if (tm.render_type == 1) {
                if (tm.image_index < bg->texture_count && bg->textures[tm.image_index]) {
                    SDL_RenderCopy(renderer, bg->textures[tm.image_index], NULL, &dst);
                }
            }
        }
    }
}
