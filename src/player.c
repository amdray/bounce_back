/**
 * Player (ball) implementation - Step 3.
 */

#include "player.h"
#include "collision.h"
#include "resource_loader.h"

#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdlib.h>

// Constants from original code (a.java:662-678)
#define GRAVITY_NORMAL  9
#define MAX_FALL_SPEED 80

static int calculate_jump_strength(Player* p) {
    int i = JUMP_NORMAL;
    if (p->is_inverted) {
        i = JUMP_INVERTED;
    } else if (p->is_popped) {
        i = JUMP_POPPED;
    }
    if (p->has_jump_bonus) {
        i += i >> 2;
    }
    if (p->has_grav_bonus) {
        i -= i >> 2;
    }
    return i;
}

Player* player_create(SDL_Renderer* renderer, int spawn_x_tiles, int spawn_y_tiles, bool is_large_ball) {
    Player* p = (Player*)calloc(1, sizeof(Player));
    if (!p) return NULL;

    ResourceContainer* ball_res = resource_load("res/b");
    if (!ball_res) {
        free(p);
        return NULL;
    }

    p->sprite_count = (int)ball_res->count - 1;
    p->ball_sprites = (SDL_Texture**)calloc((size_t)p->sprite_count, sizeof(SDL_Texture*));
    if (!p->ball_sprites) {
        resource_free(ball_res);
        free(p);
        return NULL;
    }

    for (int i = 0; i < p->sprite_count; i++) {
        size_t png_size = 0;
        const uint8_t* png_data = resource_get_element(ball_res, i + 1, &png_size);
        if (!png_data) continue;

        SDL_RWops* rw = SDL_RWFromConstMem(png_data, (int)png_size);
        SDL_Surface* surf = IMG_Load_RW(rw, 1);
        if (surf) {
            p->ball_sprites[i] = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
        }
    }

    resource_free(ball_res);

    int offset = is_large_ball ? 12 : 8;
    p->x_pos = spawn_x_tiles * 16 + offset;
    p->y_pos = spawn_y_tiles * 16 + offset;

    p->is_large = is_large_ball;
    p->sprite_index = is_large_ball ? 11 : 0;
    p->is_inverted = false;
    p->is_popped = false;
    p->gravity_down = true;
    p->is_grounded = false;
    p->has_speed_bonus = false;
    p->has_jump_bonus = false;
    p->has_grav_bonus = false;

    if (is_large_ball) {
        p->sprite_width = 16;
        p->sprite_height = 16;
    } else {
        p->sprite_width = 12;
        p->sprite_height = 12;
    }
    p->half_width = p->sprite_width / 2;
    p->half_height = p->sprite_height / 2;

    p->x_speed = 0;
    p->y_speed = 0;
    p->prev_y_speed = 0;

    return p;
}

void player_free(Player* p) {
    if (!p) return;
    if (p->ball_sprites) {
        for (int i = 0; i < p->sprite_count; i++) {
            if (p->ball_sprites[i]) SDL_DestroyTexture(p->ball_sprites[i]);
        }
        free(p->ball_sprites);
    }
    free(p);
}

void player_update(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks, Input* input) {
    if (!p) return;

    bool was_grounded = p->is_grounded;

    // Horizontal movement (a.java:1393-1430)
    if (input) {
        if (input->right && !input->left) {
            p->x_speed += p->has_speed_bonus ? ACCEL_BONUS : ACCEL_NORMAL;
            if (p->has_speed_bonus && p->x_speed > MAX_SPEED_BONUS) {
                p->x_speed = MAX_SPEED_BONUS;
            } else if (!p->has_speed_bonus && p->x_speed > MAX_SPEED_NORMAL) {
                p->x_speed = MAX_SPEED_NORMAL;
            }
        } else if (!input->right && input->left) {
            p->x_speed -= p->has_speed_bonus ? ACCEL_BONUS : ACCEL_NORMAL;
            if (p->has_speed_bonus && p->x_speed < -MAX_SPEED_BONUS) {
                p->x_speed = -MAX_SPEED_BONUS;
            } else if (!p->has_speed_bonus && p->x_speed < -MAX_SPEED_NORMAL) {
                p->x_speed = -MAX_SPEED_NORMAL;
            }
        }
    }

    // Jump (a.java:650-658) - only when grounded
    bool jump_pressed = false;
    if (input) {
        jump_pressed = input->jump && !input->down;
    }

    // Gravity
    p->y_speed += GRAVITY_NORMAL;
    if (p->y_speed > MAX_FALL_SPEED) p->y_speed = MAX_FALL_SPEED;

    if (jump_pressed && was_grounded) {
        p->y_speed = calculate_jump_strength(p);
        p->is_grounded = false;
        was_grounded = false;
    } else {
        p->is_grounded = false;
    }

    // Horizontal move with collisions
    // Original divides speeds by 10 (a.java:695: i2 = Math.abs(j) / 10)
    int x_pixels = abs(p->x_speed) / 10;
    if (x_pixels > 0) {
        int step_x = (p->x_speed > 0) ? 1 : -1;
        for (int i = 0; i < x_pixels; i++) {
            int test_x = p->x_pos + step_x;
            int rect_x = test_x - p->half_width;
            int rect_y = p->y_pos - p->half_height;

            if (collision_test(level, tile_meta, masks,
                               rect_x, rect_y, p->sprite_width, p->sprite_height, NULL)) {
                p->x_speed = 0;
                break;
            }
            p->x_pos = test_x;
        }
    }

    // Deceleration (a.java:1419-1430)
    if (p->x_speed > 0) {
        p->x_speed -= was_grounded ? DECEL_GROUNDED : DECEL_AIRBORNE;
        if (p->x_speed < 0) p->x_speed = 0;
    } else if (p->x_speed < 0) {
        p->x_speed += was_grounded ? DECEL_GROUNDED : DECEL_AIRBORNE;
        if (p->x_speed > 0) p->x_speed = 0;
    }

    // Vertical move with collisions
    // Original divides speeds by 10 (a.java:695: i2 = Math.abs(j) / 10)
    int y_pixels = abs(p->y_speed) / 10;
    if (y_pixels > 14) y_pixels = 14;  // a.java:697: if (i2 > 14) i2 = 14
    
    int step = (p->y_speed > 0) ? 1 : -1;
    for (int i = 0; i < y_pixels; i++) {
        int test_y = p->y_pos + step;
        int rect_x = p->x_pos - p->half_width;
        int rect_y = test_y - p->half_height;

        if (collision_test(level, tile_meta, masks,
                           rect_x, rect_y, p->sprite_width, p->sprite_height, NULL)) {
            p->y_speed = 0;
            p->is_grounded = (step > 0);
            break;
        }
        p->y_pos = test_y;
    }
}

void player_render(Player* p, SDL_Renderer* renderer, int camera_x, int camera_y) {
    if (!p || !p->ball_sprites) return;
    if (p->sprite_index < 0 || p->sprite_index >= p->sprite_count) return;

    SDL_Texture* sprite = p->ball_sprites[p->sprite_index];
    if (!sprite) return;

    int screen_x = p->x_pos - p->half_width - camera_x;
    int screen_y = p->y_pos - p->half_height - camera_y;

    SDL_Rect dest = { screen_x, screen_y, p->sprite_width, p->sprite_height };

    SDL_RendererFlip flip = p->is_inverted ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, sprite, NULL, &dest, 0.0, NULL, flip);
}
