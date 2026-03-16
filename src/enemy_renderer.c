#include "enemy_renderer.h"

#include "game_constants.h"

#include <SDL2/SDL.h>

/* Textures borrowed from IcAssets — NOT owned by this module. */
static SDL_Texture* g_object_tex[4] = {0};

static int object_width_for_type(uint8_t type) {
    if (type == 0) return 32;
    if (type == 1) return 16;
    return 24;
}

static int object_height_for_type(uint8_t type) {
    if (type == 0) return 32;
    if (type == 1) return 16;
    return 11;
}

int object_renderer_init(const IcAssets* ic) {
    if (!ic) return -1;
    for (int i = 0; i < 4; i++) {
        if (!ic->object_sprite[i]) return -1;
        g_object_tex[i] = ic->object_sprite[i];
    }
    return 0;
}

void object_renderer_shutdown(void) {
    /* Textures owned by GameAssets — only null out borrowed pointers. */
    for (int i = 0; i < 4; i++)
        g_object_tex[i] = NULL;
}

void object_renderer_draw(SDL_Renderer* renderer, const Level* level, int camera_x, int camera_y) {
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

        SDL_Texture* tex = g_object_tex[sprite_idx];
        if (!tex) continue;

        // Match player/object collision coordinates used in player.c.
        int world_top = level->objects.f[i][0] * 16 + level->objects.ag[i][1];
        int world_left = level->objects.f[i][1] * 16 + level->objects.ag[i][0];

        int obj_w = object_width_for_type(type);
        int obj_h = object_height_for_type(type);

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
