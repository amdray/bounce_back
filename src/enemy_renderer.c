#include "enemy_renderer.h"

#include "resource_loader.h"
#include "game_constants.h"

#include <SDL2/SDL_image.h>

static ResourceContainer* g_ic = NULL;
static SDL_Texture* g_enemy_tex[4] = {0};

static SDL_Texture* load_png_from_mem(SDL_Renderer* renderer, const uint8_t* data, size_t size) {
    if (!renderer || !data || size == 0) return NULL;

    SDL_RWops* rw = SDL_RWFromConstMem(data, (int)size);
    if (!rw) return NULL;

    SDL_Surface* surf = IMG_Load_RW(rw, 1);
    if (!surf) return NULL;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

static int enemy_width_for_type(uint8_t type) {
    if (type == 0) return 32;
    if (type == 1) return 16;
    return 24;
}

static int enemy_height_for_type(uint8_t type) {
    if (type == 0) return 32;
    if (type == 1) return 16;
    return 11;
}

int enemy_renderer_init(SDL_Renderer* renderer) {
    if (!renderer) return -1;

    g_ic = resource_load("res/ic");
    if (!g_ic) return -1;

    for (int i = 0; i < 4; i++) {
        size_t elem_size = 0;
        const uint8_t* elem = resource_get_element(g_ic, 8 + i, &elem_size);
        if (!elem || elem_size == 0) goto fail;

        g_enemy_tex[i] = load_png_from_mem(renderer, elem, elem_size);
        if (!g_enemy_tex[i]) goto fail;
    }

    return 0;

fail:
    enemy_renderer_shutdown();
    return -1;
}

void enemy_renderer_shutdown(void) {
    for (int i = 0; i < 4; i++) {
        if (g_enemy_tex[i]) {
            SDL_DestroyTexture(g_enemy_tex[i]);
            g_enemy_tex[i] = NULL;
        }
    }

    if (g_ic) {
        resource_free(g_ic);
        g_ic = NULL;
    }
}

void enemy_renderer_draw(SDL_Renderer* renderer, const Level* level, int camera_x, int camera_y) {
    if (!renderer || !level || level->objects.count == 0) return;

    const int view_l = camera_x;
    const int view_t = camera_y;
    const int view_r = camera_x + SCREEN_WIDTH;
    const int view_b = camera_y + SCREEN_HEIGHT;

    for (uint8_t i = 0; i < level->objects.count; i++) {
        uint8_t type = level->objects.ao[i];
        int sprite_idx = 1;

        if (type == 2) {
            sprite_idx = 0;
        } else if (type == 1) {
            sprite_idx = (level->objects.s[i][1] > 0) ? 3 : 2;
        }

        SDL_Texture* tex = g_enemy_tex[sprite_idx];
        if (!tex) continue;

        // Match player/object collision coordinates used in player.c.
        int world_top = level->objects.f[i][0] * 16 + level->objects.ag[i][1];
        int world_left = level->objects.f[i][1] * 16 + level->objects.ag[i][0];

        int obj_w = enemy_width_for_type(type);
        int obj_h = enemy_height_for_type(type);

        int obj_r = world_left + obj_w;
        int obj_b = world_top + obj_h;
        if (view_r <= world_left || view_b <= world_top || view_l >= obj_r || view_t >= obj_b) {
            continue;
        }

        SDL_Rect dst = {
            world_left - camera_x,
            world_top - camera_y,
            obj_w,
            obj_h
        };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
    }
}
