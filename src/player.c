/**
 * Player (ball) implementation - Step 3.
 */

#include "player.h"
#include "collision.h"
#include "resource_loader.h"

#include <SDL2/SDL_image.h>
#include <stdlib.h>

// Constants from original code (a.java:662-678)
#define GRAVITY_NORMAL  9
#define MAX_FALL_SPEED 80

static bool tile_flag80_at_center(Level* level, int center_x, int center_y) {
    if (!level) return false;
    int tx = center_x / 16;
    int ty = center_y / 16;
    if (tx < 0 || ty < 0) return false;
    if (tx >= (int)level->width || ty >= (int)level->height) return false;
    return (level_get_tile(level, tx, ty) & 0x80) != 0;
}

static uint8_t tile_id_at_pixel(Level* level, int px, int py) {
    if (!level) return 0;
    int tx = px / 16;
    int ty = py / 16;
    if (tx < 0 || ty < 0) return 0;
    if (tx >= (int)level->width || ty >= (int)level->height) return 0;
    return (uint8_t)(level_get_tile(level, tx, ty) & 0x7F);
}

static void update_ring_tile_if_crossed(Level* level, int tx, int ty, uint8_t tile_id, int center_x, int center_y) {
    if (!level) return;
    uint8_t old_tile = level_get_tile(level, tx, ty);
    uint8_t flag80 = (uint8_t)(old_tile & 0x80);

    if (tile_id == 93 && center_x == tx * 16 + 8) {
        level_set_tile(level, tx, ty, (uint8_t)(flag80 | 95));
    } else if (tile_id == 94 && center_y == ty * 16 + 8) {
        level_set_tile(level, tx, ty, (uint8_t)(flag80 | 96));
    }
}

static bool player_refresh_sprite_dims(Player* p) {
    if (!p) return false;

    int w = 0, h = 0;
    SDL_Texture* tex = NULL;
    if (p->ball_sprites && p->sprite_index >= 0 && p->sprite_index < p->sprite_count) {
        tex = p->ball_sprites[p->sprite_index];
    }
    if (!tex) return false;
    if (SDL_QueryTexture(tex, NULL, NULL, &w, &h) != 0) return false;
    if (w <= 0 || h <= 0) return false;

    p->sprite_width = w;
    p->sprite_height = h;
    p->half_width = w / 2;
    p->half_height = h / 2;
    return true;
}

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

static int calculate_bounce_min(Player* p) {
    int i = 0;
    if (p->is_inverted) {
        i = BOUNCE_INVERTED;
    } else if (p->is_popped) {
        i = BOUNCE_POPPED;
    } else {
        i = BOUNCE_NORMAL;
    }
    if (p->has_jump_bonus) {
        i += i >> 2;
    }
    return i;
}

// Change sprite with collision check (a.java:258-326)
static void player_change_sprite(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks, int sprite_index) {
    if (!p || sprite_index < 0 || sprite_index >= p->sprite_count) return;
    if (sprite_index == p->sprite_index) return;

    int old_w = p->sprite_width;
    int old_h = p->sprite_height;
    
    p->sprite_index = sprite_index;
    if (!player_refresh_sprite_dims(p)) return;
    
    // Update mask for new sprite (a.java:243-255)
    p->active_mask = player_masks_select(p->masks, p->sprite_index, &p->mask_w, &p->mask_h);
    p->mask_half_w = p->mask_w / 2;
    p->mask_half_h = p->mask_h / 2;

    // Check if new size collides (a.java:267-325)
    bool collides = collision_test(level, tile_meta, masks,
        p->x_pos - p->mask_half_w, p->y_pos - p->mask_half_h,
        p->mask_w, p->mask_h, p->active_mask);
    
    if (collides) {
        // Try to resolve position (a.java:268-325)
        int w_delta = abs(p->sprite_width - old_w) + 1;
        int h_delta = abs(p->sprite_height - old_h) + 1;
        
        // Try horizontal shifts
        for (int dx = 1; dx <= w_delta; dx++) {
            if (!collision_test(level, tile_meta, masks,
                                p->x_pos + dx - p->mask_half_w, p->y_pos - p->mask_half_h,
                                p->mask_w, p->mask_h, p->active_mask)) {
                p->x_pos += dx;
                return;
            }
            if (!collision_test(level, tile_meta, masks,
                                p->x_pos - dx - p->mask_half_w, p->y_pos - p->mask_half_h,
                                p->mask_w, p->mask_h, p->active_mask)) {
                p->x_pos -= dx;
                return;
            }
        }
        
        // Try vertical shifts
        for (int dy = 1; dy <= h_delta; dy++) {
            if (!collision_test(level, tile_meta, masks,
                                p->x_pos - p->mask_half_w, p->y_pos + dy - p->mask_half_h,
                                p->mask_w, p->mask_h, p->active_mask)) {
                p->y_pos += dy;
                return;
            }
            if (!collision_test(level, tile_meta, masks,
                                p->x_pos - p->mask_half_w, p->y_pos - dy - p->mask_half_h,
                                p->mask_w, p->mask_h, p->active_mask)) {
                p->y_pos -= dy;
                return;
            }
        }
        
        // Try diagonal
        for (int dy = 1; dy <= h_delta; dy++) {
            for (int dx = 1; dx <= w_delta; dx++) {
                if (!collision_test(level, tile_meta, masks,
                                    p->x_pos + dx - p->mask_half_w, p->y_pos + dy - p->mask_half_h,
                                    p->mask_w, p->mask_h, p->active_mask)) {
                    p->x_pos += dx;
                    p->y_pos += dy;
                    return;
                }
                if (!collision_test(level, tile_meta, masks,
                                    p->x_pos + dx - p->mask_half_w, p->y_pos - dy - p->mask_half_h,
                                    p->mask_w, p->mask_h, p->active_mask)) {
                    p->x_pos += dx;
                    p->y_pos -= dy;
                    return;
                }
                if (!collision_test(level, tile_meta, masks,
                                    p->x_pos - dx - p->mask_half_w, p->y_pos + dy - p->mask_half_h,
                                    p->mask_w, p->mask_h, p->active_mask)) {
                    p->x_pos -= dx;
                    p->y_pos += dy;
                    return;
                }
                if (!collision_test(level, tile_meta, masks,
                                    p->x_pos - dx - p->mask_half_w, p->y_pos - dy - p->mask_half_h,
                                    p->mask_w, p->mask_h, p->active_mask)) {
                    p->x_pos -= dx;
                    p->y_pos -= dy;
                    return;
                }
            }
        }
    }
}

static void flip_object_velocity(Level* level, int idx) {
    if (!level) return;
    if (idx < 0 || idx >= level->objects.count) return;
    if (level->objects.s[idx][1] != 0) {
        level->objects.s[idx][1] = (int8_t)-level->objects.s[idx][1];
    } else {
        level->objects.s[idx][0] = (int8_t)-level->objects.s[idx][0];
    }
}

static void flip_object_vertical(Level* level, int idx) {
    if (!level) return;
    if (idx < 0 || idx >= level->objects.count) return;
    level->objects.s[idx][0] = (int8_t)-level->objects.s[idx][0];
}

static void player_kill(Player* p) {
    if (!p) return;
    // Cheat: god mode prevents death
    if (p->god_mode) return;
    // Match a.java:k() - trigger death sequence
    p->is_dying = true;
    p->timer_a = 25;    // 25 ticks death animation
    // Speeds zeroed on respawn, not here
}

// Match a.java:e() - respawn at checkpoint
static void player_respawn(Player* p) {
    if (!p) return;

    p->lives--;

    // Reset position to spawn checkpoint
    int offset = p->spawn_is_large ? 12 : 8;
    p->x_pos = p->spawn_tile_x * 16 + offset;
    p->y_pos = p->spawn_tile_y * 16 + offset;

    // Reset all state (matching a.java:147-183 e() method)
    p->x_speed = 0;
    p->y_speed = 0;
    p->prev_y_speed = 0;
    p->bounce_state = 0;
    p->timer_a = 0;
    p->timer_b = 0;
    p->timer_c = 0;

    p->is_grounded = false;
    p->is_dying = false;
    p->is_popped = false;
    p->is_inverted = false;
    p->gravity_down = false;
    p->stunned = false;
    p->has_speed_bonus = false;
    p->has_jump_bonus = false;
    p->has_grav_bonus = false;

    p->state_a = 0;
    p->state_r = 0;
    p->carrier_object_index = -1;

    // Reset sprite to base depending on original spawn ball size
    p->sprite_index = p->spawn_is_large ? 11 : 0;
    player_refresh_sprite_dims(p);

    // If original spawn is "large ball", restore inverted state
    if (p->spawn_is_large) {
        p->is_inverted = true;
    }
}

// Forward declarations for animation functions
static void player_change_sprite(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks, int sprite_index);

// Match a.java:g() - right-side inflation state machine
static void player_apply_r_state(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks) {
    if (!p) return;
    
    if (p->state_r == 1) {
        // Inflate small -> popped (sprites 20->21->22)
        if (p->timer_a == 2) {
            player_change_sprite(p, level, tile_meta, masks, 20);
        } else if (p->timer_a == 1) {
            player_change_sprite(p, level, tile_meta, masks, 21);
        } else if (p->timer_a == 0) {
            player_change_sprite(p, level, tile_meta, masks, 22);
            p->is_popped = true;
            p->timer_c = 550;  // this.b = 550
            p->state_r = 0;
        }
    }
    
    if (p->state_r == 2) {
        // Deflate popped -> small (sprites 21->20->0)
        if (p->timer_a == 2) {
            player_change_sprite(p, level, tile_meta, masks, 21);
        } else if (p->timer_a == 1) {
            player_change_sprite(p, level, tile_meta, masks, 20);
        } else if (p->timer_a == 0) {
            player_change_sprite(p, level, tile_meta, masks, 0);
            p->is_popped = false;
            p->stunned = false;
            p->state_r = 0;
        }
    }
    
    if (p->state_r == 3) {
        // Inverted -> small first (sprites 10->9->0), then chain to r=1
        if (p->timer_a == 2) {
            player_change_sprite(p, level, tile_meta, masks, 10);
        } else if (p->timer_a == 1) {
            player_change_sprite(p, level, tile_meta, masks, 9);
        } else if (p->timer_a == 0) {
            player_change_sprite(p, level, tile_meta, masks, 0);
            p->is_inverted = false;
            p->state_r = 1;  // Chain to inflate
            p->timer_a = 3;
        }
    }
}

// Match a.java:a() - left-side inflation state machine
static void player_apply_a_state(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks) {
    if (!p) return;
    
    if (p->state_a == 1) {
        // Inflate small -> inverted (sprites 9->10->11)
        if (p->timer_a == 2) {
            player_change_sprite(p, level, tile_meta, masks, 9);
        } else if (p->timer_a == 1) {
            player_change_sprite(p, level, tile_meta, masks, 10);
        } else if (p->timer_a == 0) {
            player_change_sprite(p, level, tile_meta, masks, 11);
            p->is_inverted = true;
            p->stunned = false;
            p->state_a = 0;
        }
    }
    
    if (p->state_a == 2) {
        // Deflate inverted -> small (sprites 10->9->0)
        if (p->timer_a == 2) {
            player_change_sprite(p, level, tile_meta, masks, 10);
        } else if (p->timer_a == 1) {
            player_change_sprite(p, level, tile_meta, masks, 9);
        } else if (p->timer_a == 0) {
            player_change_sprite(p, level, tile_meta, masks, 0);
            p->is_inverted = false;
            p->stunned = false;
            p->state_a = 0;
        }
    }
}

// Match a.java:j() - squash/stretch animation during stunned
// Each frame decrements timer_a and changes sprite to create elastic bounce effect
// Frame 1 (A=2): sprite+1 (more squashed)
// Frame 2 (A=1): sprite-1 (recovering/stretching back)
// Frame 3 (A=0): return to base sprite (0 or 11)
static void player_apply_j_state(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks) {
    if (!p) return;
    
    if (p->timer_a == 2) {
        if (p->sprite_index + 1 < p->sprite_count) {
            player_change_sprite(p, level, tile_meta, masks, p->sprite_index + 1);
        }
    } else if (p->timer_a == 1) {
        if (p->sprite_index > 0) {
            player_change_sprite(p, level, tile_meta, masks, p->sprite_index - 1);
        }
    } else if (p->timer_a == 0) {
        // Return to base sprite
        player_change_sprite(p, level, tile_meta, masks, p->is_inverted ? 11 : 0);
        p->stunned = false;
    }
}

// Match a.java:532-536 - squash/stretch animation trigger on impact
static void player_trigger_squash(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks, int sprite_delta) {
    if (!p || !level || !tile_meta || !masks) return;
    int target_sprite = p->sprite_index + sprite_delta;
    if (target_sprite >= 0 && target_sprite < p->sprite_count) {
        player_change_sprite(p, level, tile_meta, masks, target_sprite);
    }
    p->timer_a = 3;
    p->stunned = true;
}

static void player_apply_state_c(Player* p) {
    if (!p) return;
    if (!p->is_popped && p->state_a == 0 && p->state_r == 0) {
        p->timer_a = 0;
        p->stunned = false;
        p->sprite_index = p->is_inverted ? 11 : 0;
    }
}

static void player_apply_state_l(Player* p) {
    if (!p) return;
    if (!p->stunned) {
        p->state_a = p->is_inverted ? 2 : 1;
        p->stunned = true;
        p->timer_a = 3;
    }
}

static void player_apply_state_b(Player* p) {
    if (!p) return;
    if (!p->stunned) {
        p->state_r = p->is_inverted ? 3 : 1;
        p->stunned = true;
        p->timer_a = 3;
    }
}

static void player_apply_tile_break(Level* level, int tx, int ty) {
    if (!level) return;
    level_set_tile(level, tx, ty, 105);
}

static void player_collect_tile(Player* p, Level* level, int tx, int ty, uint8_t tile_id) {
    if (!p) return;
    if (!level) return;
    uint8_t old = level_get_tile(level, tx, ty);
    uint8_t flag = (uint8_t)(old & 0x80);
    switch (tile_id) {
        case 34:
            p->score += 200;
            // Coin/gem collected
            level_set_tile(level, tx, ty, (uint8_t)(0x23 | flag));
            break;
        case 12:
            p->score += 1000;
            if (p->lives < 5) p->lives++;
            level_set_tile(level, tx, ty, flag);
            break;
        case 30:
            p->score += 2500;
            // Crystal/bonus collected
            level_set_tile(level, tx, ty, flag);
            break;
        case 93:
        case 94:
        case 97:
        case 98:
        case 101:
        case 102:
            p->score += 500;
            // Hoop/ring collected - decrement counter (a.java:350-360)
            if (level->hoops_remaining > 0) {
                level->hoops_remaining--;
            }
            // Tile transformation handled by apply_tile_*() functions
            break;
        default:
            break;
    }
}

static void player_special_tile(Player* p, Level* level, int tx, int ty, uint8_t tile_id) {
    (void)level;
    (void)tx;
    (void)ty;
    if (!p) return;
    switch (tile_id) {
        case 39:
            p->gravity_down = false;
            p->has_jump_bonus = false;
            p->has_speed_bonus = true;
            p->timer_b = 450;
            break;
        case 26:
            p->gravity_down = false;
            p->has_jump_bonus = true;
            p->has_speed_bonus = false;
            p->timer_b = 450;
            break;
        case 15:
            p->gravity_down = true;
            p->has_jump_bonus = false;
            p->has_speed_bonus = false;
            p->is_grounded = false;
            p->timer_b = 450;
            break;
        case 22:
            if (p->is_popped) p->timer_c = 550;
            player_apply_state_c(p);
            player_apply_state_b(p);
            break;
        case 18:
            if (!p->is_inverted && !p->is_popped && p->state_a == 0) {
                player_apply_state_c(p);
                player_apply_state_l(p);
            }
            break;
        case 11:
            if (p->is_inverted && !p->is_popped && p->state_a == 0) {
                player_apply_state_c(p);
                player_apply_state_l(p);
            }
            break;
        default:
            break;
    }
}

static void apply_tile_101_102(Level* level, int tx, int ty, uint8_t tile_id) {
    if (!level) return;
    uint8_t old = level_get_tile(level, tx, ty);
    uint8_t flag = (uint8_t)(old & 0x80);
    if (tile_id == 102) {
        uint8_t n = level_get_tile(level, tx - 1, ty);
        uint8_t nf = (uint8_t)(n & 0x80);
        level_set_tile(level, tx - 1, ty, (uint8_t)(nf | 0x67));
        level_set_tile(level, tx, ty, (uint8_t)(0x68 | flag));
    } else if (tile_id == 101) {
        level_set_tile(level, tx, ty, (uint8_t)(0x67 | flag));
        uint8_t n = level_get_tile(level, tx + 1, ty);
        uint8_t nf = (uint8_t)(n & 0x80);
        level_set_tile(level, tx + 1, ty, (uint8_t)(nf | 0x68));
    }
}

static void apply_tile_97_98(Level* level, int tx, int ty, uint8_t tile_id) {
    if (!level) return;
    uint8_t old = level_get_tile(level, tx, ty);
    uint8_t flag = (uint8_t)(old & 0x80);
    if (tile_id == 97) {
        level_set_tile(level, tx, ty, (uint8_t)(0x63 | flag));
        uint8_t n = level_get_tile(level, tx, ty + 1);
        uint8_t nf = (uint8_t)(n & 0x80);
        level_set_tile(level, tx, ty + 1, (uint8_t)(nf | 0x64));
    } else if (tile_id == 98) {
        uint8_t n = level_get_tile(level, tx, ty - 1);
        uint8_t nf = (uint8_t)(n & 0x80);
        level_set_tile(level, tx, ty - 1, (uint8_t)(nf | 0x63));
        level_set_tile(level, tx, ty, (uint8_t)(0x64 | flag));
    }
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
        if (!png_data) {
            fprintf(stderr, "Failed to load ball sprite %d\n", i + 1);
            continue;
        }

        SDL_RWops* rw = SDL_RWFromConstMem(png_data, (int)png_size);
        if (!rw) {
            fprintf(stderr, "Failed to create RW for ball sprite %d\n", i);
            continue;
        }
        
        SDL_Surface* surf = IMG_Load_RW(rw, 1);
        if (surf) {
            p->ball_sprites[i] = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
            if (!p->ball_sprites[i]) {
                fprintf(stderr, "Failed to create texture for ball sprite %d\n", i);
            }
        } else {
            fprintf(stderr, "Failed to load surface for ball sprite %d: %s\n", i, IMG_GetError());
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
    p->gravity_down = false;
    p->is_grounded = false;
    p->has_speed_bonus = false;
    p->has_jump_bonus = false;
    p->has_grav_bonus = false;

    // Verify base sprite loaded before initializing
    if (!p->ball_sprites[p->sprite_index]) {
        fprintf(stderr, "Base ball sprite %d not loaded\n", p->sprite_index);
        player_free(p);
        return NULL;
    }

    if (!player_refresh_sprite_dims(p)) {
        fprintf(stderr, "Failed to refresh sprite dims\n");
        player_free(p);
        return NULL;
    }

    p->x_speed = 0;
    p->y_speed = 0;
    p->prev_y_speed = 0;
    p->state_r = 0;
    p->state_a = 0;
    p->carrier_object_index = -1;

    // Save spawn checkpoint (for respawn after death)
    p->spawn_tile_x = spawn_x_tiles;
    p->spawn_tile_y = spawn_y_tiles;
    p->spawn_is_large = is_large_ball;
    p->is_dying = false;
    p->god_mode = false;
    p->lives = 3;
    p->score = 0;

    p->masks = player_masks_load("res/b");
    p->active_mask = player_masks_select(p->masks, p->sprite_index, &p->mask_w, &p->mask_h);
    if (!p->active_mask || p->mask_w <= 0 || p->mask_h <= 0) {
        player_free(p);
        return NULL;
    }
    p->mask_half_w = p->mask_w / 2;
    p->mask_half_h = p->mask_h / 2;

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
    player_masks_free(p->masks);
    free(p);
}

void player_update(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks, Input* input) {
    if (!p || !level || !tile_meta || !masks) return;

    // Handle death sequence (a.java:593-605, 1646-1665)
    if (p->is_dying) {
        if (p->timer_a > 0) {
            p->timer_a--;
        }
        if (p->timer_a == 0 && p->is_dying) {
            player_respawn(p);
        }
        return; // Skip normal update while dying
    }

    p->active_mask = player_masks_select(p->masks, p->sprite_index, &p->mask_w, &p->mask_h);
    if (!p->active_mask || p->mask_w <= 0 || p->mask_h <= 0) return;
    p->mask_half_w = p->mask_w / 2;
    p->mask_half_h = p->mask_h / 2;

    int i = p->x_speed;
    int j = p->y_speed;
    int k = p->prev_y_speed;
    p->carrier_object_index = -1;
    bool* obj_processed = NULL;
    if (level->objects.count > 0) {
        obj_processed = (bool*)calloc(level->objects.count, sizeof(bool));
    }

    p->control_mask = 0;
    if (input) {
        if (input->left) p->control_mask |= 0x1;
        if (input->right) p->control_mask |= 0x2;
        if (input->down) p->control_mask |= 0x4;
        if (input->jump) p->control_mask |= 0x8;
        // Cheat: R+L = toggle god mode
        static bool prev_cheat = false;
        bool cheat_pressed = input->shoulder_l && input->shoulder_r;
        if (cheat_pressed && !prev_cheat) {
            p->god_mode = !p->god_mode;
        }
        prev_cheat = cheat_pressed;
    }

    int m = p->x_pos / 16;
    int n = p->y_pos / 16;
    uint8_t center_tile = (uint8_t)(level_get_tile(level, m, n) & 0x7F);
    bool tile_flag80 = tile_flag80_at_center(level, p->x_pos, p->y_pos);
    p->has_grav_bonus = tile_flag80;
    bool jump_hold = ((p->control_mask & 0x8) != 0 && (p->control_mask & 0x4) == 0);

    if (p->timer_a != 0) {
        p->timer_a--;
        if (p->state_r != 0) player_apply_r_state(p, level, tile_meta, masks);
        if (p->state_a != 0) player_apply_a_state(p, level, tile_meta, masks);
        if (p->stunned && !p->is_popped) player_apply_j_state(p, level, tile_meta, masks);
    }
    if (p->timer_b != 0) {
        p->timer_b--;
        if (p->timer_b == 0) {
            p->gravity_down = false;
            p->has_speed_bonus = false;
            p->has_jump_bonus = false;
        }
    }
    if (p->timer_c != 0) {
        p->timer_c--;
        if (p->timer_c == 0) {
            p->state_r = 2;
            p->timer_a = 3;
        }
    }

    if (jump_hold && p->is_grounded) {
        j = calculate_jump_strength(p);
        p->is_grounded = false;
    }

    if (!p->is_grounded || (p->has_grav_bonus && p->is_inverted)) {
        int max_fall = p->has_grav_bonus ? (p->is_popped ? 80 : 20) : 80;
        int gravity = 9;
        if (p->has_grav_bonus) {
            if (p->is_inverted) {
                gravity = -6;
                p->is_grounded = false;  // Only for inverted!
            } else {
                gravity = 7;
            }
        }
        j += gravity;
        if (j > max_fall) {
            j = max_fall;
        } else if (p->has_grav_bonus && p->is_inverted && -j > max_fall) {
            j = -max_fall;
        }
    }

    bool bool2 = false;
    if (center_tile == 45 || center_tile == 51 || center_tile == 53 ||
        center_tile == 67 || center_tile == 71 || center_tile == 75) {
        if (j >= -100) {
            if (p->gravity_down) {
                j += (center_tile == 75) ? 50 : 100;
            } else {
                j -= (center_tile == 75) ? 50 : 100;
            }
        }
        p->is_grounded = false;
        bool2 = true;
    }

    if (p->gravity_down) j = -j;
    int y_pixels = abs(j) / 10;
    int step_y = (j == 0) ? 0 : ((j < 0) ? -1 : 1);
    // After j inversion, step_y > 0 always means "toward surface" regardless of gravity_down
    // Because when gravity_down=true, j was inverted, so step_y=+1 means original j was negative (moving up to ceiling)
    // When gravity_down=false, j unchanged, so step_y=+1 means moving down to floor
    bool toward_surface = (step_y > 0);
    if (y_pixels > 14) y_pixels = 14;
    if (p->gravity_down) j = -j;

    for (int s = 0; s < y_pixels; s++) {
        int test_y = p->y_pos + step_y;
        int rect_x = p->x_pos - p->mask_half_w;
        int rect_y = test_y - p->mask_half_h;

        CollisionHits hits;
        collision_hits_clear(&hits);
        bool blocking = collision_test_collect(level, tile_meta, masks,
                                               rect_x, rect_y, p->mask_w, p->mask_h,
                                               p->active_mask, &hits);

        // Match a.java: after vertical probe, force center tile 101-104 into hit list.
        // a.java uses R[n][m] with n=(i+b1)/16 and m=D/16 before switch processing.
        int center_tx = p->x_pos / 16;
        int center_ty = test_y / 16;
        if (center_tx >= 0 && center_ty >= 0 &&
            center_tx < (int)level->width && center_ty < (int)level->height) {
            uint8_t center_id = (uint8_t)(level_get_tile(level, center_tx, center_ty) & 0x7F);
            if (center_id == 101 || center_id == 102 || center_id == 103 || center_id == 104) {
                bool already = false;
                int free_idx = -1;
                for (int hh = 0; hh < COLLISION_HITS_MAX; hh++) {
                    if (hits.x[hh] == center_tx && hits.y[hh] == center_ty) {
                        already = true;
                        break;
                    }
                    if (free_idx < 0 && hits.x[hh] == -1 && hits.y[hh] == -1) {
                        free_idx = hh;
                    }
                }
                if (!already && free_idx >= 0) {
                    hits.x[free_idx] = center_tx;
                    hits.y[free_idx] = center_ty;
                    already = true;
                }
                if (already) {
                    blocking = true;
                }
            }
        }

        if (level->active_object_index >= 0) p->carrier_object_index = level->active_object_index;
        if (!blocking) {
            p->y_pos = test_y;
            p->is_grounded = false;
            continue;
        }

        bool resolved = false;
        for (int h = 0; h < COLLISION_HITS_MAX; h++) {
            int tx = hits.x[h];
            int ty = hits.y[h];
            uint8_t hit_id = 0;
            uint8_t id_logic = 0;

            if (tx >= 0 && ty >= 0) {
                hit_id = (uint8_t)(level_get_tile(level, tx, ty) & 0x7F);
                id_logic = hit_id;
            } else if (p->carrier_object_index >= 0 && p->carrier_object_index < level->objects.count) {
                // Match a.java: when tile slot is empty but object is active, synthesize 200/201/202.
                uint8_t obj_type = level->objects.ao[p->carrier_object_index];
                if (obj_type == 2) {
                    id_logic = 200;
                } else if (obj_type == 0) {
                    id_logic = p->is_popped ? 200 : 201;
                } else {
                    id_logic = p->is_popped ? 200 : 202;
                }
                hit_id = id_logic;
            } else {
                continue;
            }

            if (tx >= 0 && ty >= 0 && !p->is_inverted && (hit_id == 94 || hit_id == 96) &&
                (p->x_pos - p->half_width == tx * 16)) {
                p->y_pos = test_y;
                if (hit_id == 94 && p->y_pos == ty * 16 + 8) {
                    player_collect_tile(p, level, tx, ty, hit_id);
                    update_ring_tile_if_crossed(level, tx, ty, hit_id, p->x_pos, p->y_pos);
                }
                continue;
            }

            // Match a.java case 94/96: if not aligned for pass-through, try side-shift at the same Y step.
            if (tx >= 0 && ty >= 0 && (hit_id == 94 || hit_id == 96)) {
                if (!collision_test(level, tile_meta, masks,
                                    (p->x_pos + 1) - p->mask_half_w,
                                    test_y - p->mask_half_h,
                                    p->mask_w, p->mask_h,
                                    p->active_mask)) {
                    p->x_pos += 1;
                    p->y_pos = test_y;
                    if (hit_id == 94 && p->y_pos == ty * 16 + 8) {
                        player_collect_tile(p, level, tx, ty, hit_id);
                        update_ring_tile_if_crossed(level, tx, ty, hit_id, p->x_pos, p->y_pos);
                    }
                    continue;
                }
                if (!collision_test(level, tile_meta, masks,
                                    (p->x_pos - 1) - p->mask_half_w,
                                    test_y - p->mask_half_h,
                                    p->mask_w, p->mask_h,
                                    p->active_mask)) {
                    p->x_pos -= 1;
                    p->y_pos = test_y;
                    if (hit_id == 94 && p->y_pos == ty * 16 + 8) {
                        player_collect_tile(p, level, tx, ty, hit_id);
                        update_ring_tile_if_crossed(level, tx, ty, hit_id, p->x_pos, p->y_pos);
                    }
                    continue;
                }

                if (toward_surface) {
                    if (jump_hold) {
                        j = calculate_jump_strength(p);
                        p->is_grounded = false;
                        p->bounce_state = 0;
                    } else if (j > 30) {
                        if (p->bounce_state == 0) {
                            p->bounce_state = -j;
                            int min_bounce = calculate_bounce_min(p);
                            if (p->bounce_state < min_bounce) {
                                p->bounce_state = min_bounce;
                            }
                        }
                        j = p->bounce_state;
                        p->bounce_state >>= 1;
                        if (p->bounce_state > -10) {
                            p->is_grounded = true;
                            j = 30;
                            p->bounce_state = 0;
                        }
                    } else if (j > -30) {
                        p->is_grounded = true;
                        j = 30;
                        p->bounce_state = 0;
                    }
                } else {
                    j = 0;
                }
                continue;
            }

            // Match a.java case 101/102/103/104: handled for both vertical directions.
            if (id_logic == 101 || id_logic == 102 || id_logic == 103 || id_logic == 104) {
                if (!collision_test(level, tile_meta, masks,
                                    p->x_pos - p->mask_half_w,
                                    test_y - p->mask_half_h,
                                    p->mask_w, p->mask_h,
                                    p->active_mask)) {
                    p->y_pos = test_y;
                    if ((id_logic == 102 || id_logic == 101) && p->y_pos == ty * 16 + 8) {
                        player_collect_tile(p, level, tx, ty, id_logic);
                        apply_tile_101_102(level, tx, ty, id_logic);
                    }
                    continue;
                }
                if (!collision_test(level, tile_meta, masks,
                                    (p->x_pos + 1) - p->mask_half_w,
                                    test_y - p->mask_half_h,
                                    p->mask_w, p->mask_h,
                                    p->active_mask)) {
                    p->x_pos += 1;
                    p->y_pos = test_y;
                    if ((id_logic == 102 || id_logic == 101) && p->y_pos == ty * 16 + 8) {
                        player_collect_tile(p, level, tx, ty, id_logic);
                        apply_tile_101_102(level, tx, ty, id_logic);
                    }
                    continue;
                }
                if (!collision_test(level, tile_meta, masks,
                                    (p->x_pos - 1) - p->mask_half_w,
                                    test_y - p->mask_half_h,
                                    p->mask_w, p->mask_h,
                                    p->active_mask)) {
                    p->x_pos -= 1;
                    p->y_pos = test_y;
                    if ((id_logic == 102 || id_logic == 101) && p->y_pos == ty * 16 + 8) {
                        player_collect_tile(p, level, tx, ty, id_logic);
                        apply_tile_101_102(level, tx, ty, id_logic);
                    }
                    continue;
                }

                if (toward_surface) {
                    if (jump_hold) {
                        j = calculate_jump_strength(p);
                        p->is_grounded = false;
                        p->bounce_state = 0;
                    } else if (j > 30) {
                        if (p->bounce_state == 0) {
                            p->bounce_state = -j;
                            int min_bounce = calculate_bounce_min(p);
                            if (p->bounce_state < min_bounce) {
                                p->bounce_state = min_bounce;
                            }
                        }
                        j = p->bounce_state;
                        p->bounce_state >>= 1;
                        if (p->bounce_state > -10) {
                            p->is_grounded = true;
                            j = 30;
                            p->bounce_state = 0;
                        }
                    } else if (j > -30) {
                        p->is_grounded = true;
                        j = 30;
                        p->bounce_state = 0;
                    }
                } else {
                    j = 0;
                }
                continue;
            }

            if (toward_surface) {
                if (!p->gravity_down) {
                    if (id_logic == 6 || id_logic == 5) id_logic = 2;
                    if (id_logic == 116 || id_logic == 115) id_logic = 110;
                } else {
                    if (id_logic == 3 || id_logic == 4) id_logic = 2;
                    if (id_logic == 113 || id_logic == 114) id_logic = 110;
                }
            } else {
                if (!p->gravity_down) {
                    if (id_logic == 113 || id_logic == 114) id_logic = 110;
                    if (id_logic == 3 || id_logic == 4) id_logic = 2;
                } else {
                    if (id_logic == 116 || id_logic == 115) id_logic = 110;
                    if (id_logic == 6 || id_logic == 5) id_logic = 2;
                }
            }

            if (!toward_surface) {
                switch (id_logic) {
                    case 3:
                    case 6:
                    case 113:
                    case 116:
                        if (bool2) {
                            if (!collision_test(level, tile_meta, masks,
                                                (p->x_pos - 1) - p->mask_half_w,
                                                test_y - p->mask_half_h, p->mask_w, p->mask_h,
                                                p->active_mask)) {
                                p->x_pos -= 1;
                                p->y_pos = test_y;
                            }
                            resolved = true;
                        }
                        // a.java:1256-1260 - squash on upward slope collision
                        else if (j <= -40 && !p->stunned) {
                            player_trigger_squash(p, level, tile_meta, masks, p->gravity_down ? 5 : 7);
                            resolved = true;
                        }
                        else {
                            if (j != 0) i = j >> 1;
                            j = 0;
                            resolved = true;
                        }
                        break;
                    case 4:
                    case 5:
                    case 114:
                    case 115:
                        if (bool2) {
                            if (!collision_test(level, tile_meta, masks,
                                                (p->x_pos + 1) - p->mask_half_w,
                                                test_y - p->mask_half_h, p->mask_w, p->mask_h,
                                                p->active_mask)) {
                                p->x_pos += 1;
                                p->y_pos = test_y;
                            }
                            resolved = true;
                        }
                        // a.java:1278-1282 - squash on upward slope collision
                        else if (j <= -40 && !p->stunned) {
                            player_trigger_squash(p, level, tile_meta, masks, p->gravity_down ? 7 : 5);
                            resolved = true;
                        }
                        else {
                            if (j != 0) i = -(j >> 1);
                            j = 0;
                            resolved = true;
                        }
                        break;
                    case 201:
                    case 202:
                        // Enemy collision - always death
                        player_kill(p);
                        resolved = true;
                        break;
                    case 1:
                    case 62:
                    case 63:
                    case 64:
                    case 65:
                    case 85:
                    case 86:
                    case 87:
                    case 88:
                    case 89:
                    case 90:
                    case 91:
                    case 92:
                        // Spike tiles - death unless popped
                        if (p->is_popped) {
                            if (toward_surface) {
                                p->is_grounded = true;
                                j = 30;
                            }
                            j = 30;
                        } else {
                            player_kill(p);
                        }
                        resolved = true;
                        break;
                    case 200:
                        j = 0;
                        resolved = true;
                        break;
                    case 12:
                    case 30:
                    case 34:
                        player_collect_tile(p, level, tx, ty, id_logic);
                        resolved = true;
                        break;
                    case 11:
                    case 15:
                    case 18:
                    case 22:
                    case 26:
                    case 39:
                        player_special_tile(p, level, tx, ty, id_logic);
                        resolved = true;
                        break;
                    default:
                        if ((id_logic == 7 || id_logic == 8) && p->is_popped) {
                            player_apply_tile_break(level, tx, ty);
                        }
                        // Upward blocking collision in original code zeros vertical speed.
                        j = 0;
                        resolved = true;
                        break;
                }
            } else {
                switch (id_logic) {
                    case 3:
                    case 6:
                    case 52:
                    case 113:
                    case 116:
                        // a.java:918 - reset prev_y_speed for non-corner slopes
                        if (id_logic != 6 && id_logic != 3) {
                            k = 0;
                        }
                        // a.java:919-924 - squash animation on very fast landing (j>=80)
                        if (j >= 80 && !p->stunned) {
                            player_trigger_squash(p, level, tile_meta, masks, p->gravity_down ? 7 : 5);
                            break;
                        }
                        // a.java:926-942 - bounce state management for medium speed landing
                        if (j > 40 && !p->is_grounded) {
                            if (p->bounce_state == 0) {
                                p->bounce_state = -(j >> 1);
                                int min_bounce = calculate_bounce_min(p) >> 1;
                                if (p->bounce_state < min_bounce) {
                                    p->bounce_state = min_bounce;
                                }
                            }
                            j = p->bounce_state;
                            if (i <= 0) {
                                i = p->bounce_state;
                            }
                            p->bounce_state = (3 * p->bounce_state) >> 2;
                            if (p->bounce_state > -10) {
                                p->is_grounded = true;
                                j = 30;
                                p->bounce_state = 0;
                            }
                            break;
                        }
                        // a.java:944-952 - grounding on slope when no x-speed
                        if (i == 0) {
                            if ((!p->has_grav_bonus || !p->is_inverted) &&
                                !collision_test(level, tile_meta, masks,
                                                (p->x_pos - 1) - p->mask_half_w,
                                                test_y - p->mask_half_h, p->mask_w, p->mask_h,
                                                p->active_mask)) {
                                p->x_pos -= 1;
                                p->y_pos = test_y;
                                p->is_grounded = true;
                                j = 30;
                                p->bounce_state = 0;
                                resolved = true;
                            }
                            break;
                        }
                        // a.java:954-958 - final bounce state reset
                        if (j > -30) {
                            p->is_grounded = true;
                            j = 30;
                            p->bounce_state = 0;
                        }
                        break;
                    case 4:
                    case 5:
                    case 53:
                    case 114:
                    case 115:
                        // a.java:964 - reset prev_y_speed for non-corner slopes
                        if (id_logic != 5 && id_logic != 4) {
                            k = 0;
                        }
                        // a.java:967-972 - squash animation on very fast landing (j>=80)
                        if (j >= 80 && !p->stunned) {
                            player_trigger_squash(p, level, tile_meta, masks, p->gravity_down ? 5 : 7);
                            break;
                        }
                        // a.java:975-991 - bounce state management (NOTE: j > 30, not j > 40!)
                        if (j > 30 && !p->is_grounded) {
                            if (p->bounce_state == 0) {
                                p->bounce_state = -(j >> 1);
                                int min_bounce = calculate_bounce_min(p) >> 1;
                                if (p->bounce_state < min_bounce) {
                                    p->bounce_state = min_bounce;
                                }
                            }
                            j = p->bounce_state;
                            if (i <= 0) {
                                i = -p->bounce_state;
                            }
                            p->bounce_state = (3 * p->bounce_state) >> 2;
                            if (p->bounce_state > -10) {
                                p->is_grounded = true;
                                j = 30;
                                p->bounce_state = 0;
                            }
                            break;
                        }
                        // a.java:993-1001 - grounding on slope when no x-speed
                        if (i == 0) {
                            if ((!p->has_grav_bonus || !p->is_inverted) &&
                                !collision_test(level, tile_meta, masks,
                                                (p->x_pos + 1) - p->mask_half_w,
                                                test_y - p->mask_half_h, p->mask_w, p->mask_h,
                                                p->active_mask)) {
                                p->x_pos += 1;
                                p->y_pos = test_y;
                                p->is_grounded = true;
                                j = 30;
                                p->bounce_state = 0;
                                resolved = true;
                            }
                            break;
                        }
                        // a.java:1003-1007 - final bounce state reset
                        if (j > -30) {
                            p->is_grounded = true;
                            j = 30;
                            p->bounce_state = 0;
                        }
                        break;
                    case 2:
                        // a.java:1011-1020 - try to slide left or right
                        {
                            int slid = 0;
                            if (!collision_test(level, tile_meta, masks,
                                                (p->x_pos + 1) - p->mask_half_w,
                                                test_y - p->mask_half_h, p->mask_w, p->mask_h,
                                                p->active_mask)) {
                                p->x_pos += 1;
                                p->y_pos = test_y;
                                slid = 1;
                            } else if (!collision_test(level, tile_meta, masks,
                                                       (p->x_pos - 1) - p->mask_half_w,
                                                       test_y - p->mask_half_h, p->mask_w, p->mask_h,
                                                       p->active_mask)) {
                                p->x_pos -= 1;
                                p->y_pos = test_y;
                                slid = 1;
                            }
                            if (slid) {
                                resolved = true;
                                break;
                            }
                        }
                        // a.java:1022-1025 - squash animation on fast landing when can't slide
                        if (j > 40 && !p->stunned) {
                            player_trigger_squash(p, level, tile_meta, masks, 1);
                            k = p->prev_y_speed;
                            break;
                        }
                        // a.java:1027-1037 - jump from flat surface
                        if (jump_hold) {
                            p->bounce_state = 0;
                            k = p->prev_y_speed;
                            if (k == 0) {
                                k = -30;
                            }
                            j = calculate_jump_strength(p) + k;
                            p->is_grounded = false;
                            k -= 30;
                            if (k < -83) {
                                k = -83;
                            }
                            break;
                        }
                        // a.java:1039-1052 - bounce state for flat surface (NOTE: G = -j, not -(j>>1)!)
                        if (j > 30 && !p->is_grounded) {
                            if (p->bounce_state == 0) {
                                p->bounce_state = -j;
                                int min_bounce = calculate_bounce_min(p);
                                if (p->bounce_state < min_bounce) {
                                    p->bounce_state = min_bounce;
                                }
                            }
                            j = p->bounce_state;
                            p->bounce_state >>= 1;
                            if (p->bounce_state > -10) {
                                p->is_grounded = true;
                                j = 30;
                                p->bounce_state = 0;
                            }
                            break;
                        }
                        // a.java:1054-1058 - final bounce state reset
                        if (j > -30) {
                            p->is_grounded = true;
                            j = 30;
                            p->bounce_state = 0;
                        }
                        break;
                    case 200: {
                        int obj = p->carrier_object_index;
                        if (obj >= 0 && obj < level->objects.count) {
                            int top = level->objects.f[obj][0] * 16 + level->objects.ag[obj][1];
                            int left = level->objects.f[obj][1] * 16 + level->objects.ag[obj][0];
                            int obj_h = (level->objects.ao[obj] == 2) ? 11 : ((level->objects.ao[obj] == 0) ? 32 : 16);
                            int obj_w = (level->objects.ao[obj] == 2) ? 24 : ((level->objects.ao[obj] == 0) ? 32 : 16);
                            int sv = level->objects.s[obj][0];
                            int sh = level->objects.s[obj][1];
                            if (sh != 0) {
                                if ((!p->gravity_down && p->y_pos - p->half_height >= top) ||
                                    (p->gravity_down && p->y_pos + p->half_height <= top + obj_h)) {
                                    int target_y = p->gravity_down ? (top - p->half_height) : (top + obj_h + p->half_height);
                                    if (!collision_test(level, tile_meta, masks,
                                                        p->x_pos - p->mask_half_w,
                                                        target_y - p->mask_half_h,
                                                        p->mask_w, p->mask_h,
                                                        p->active_mask)) {
                                        p->y_pos = target_y;
                                        j = 30;
                                        resolved = true;
                                    } else if (p->is_popped) {
                                        if (!obj_processed || !obj_processed[obj]) {
                                            flip_object_velocity(level, obj);
                                            if (obj_processed) obj_processed[obj] = true;
                                        }
                                        resolved = true;
                                    } else {
                                        player_kill(p);
                                        resolved = true;
                                    }
                                } else if (jump_hold) {
                                    j = calculate_jump_strength(p);
                                    p->is_grounded = false;
                                    // a.java:1105 - squash animation on platform jump
                                    if (!p->stunned) {
                                        player_trigger_squash(p, level, tile_meta, masks, 1);
                                    }
                                    resolved = true;
                                } else if (p->is_grounded) {
                                    int target_y = p->gravity_down ? (top + obj_h + p->half_height) : (top - p->half_height);
                                    if (!collision_test(level, tile_meta, masks,
                                                        p->x_pos - p->mask_half_w,
                                                        target_y - p->mask_half_h,
                                                        p->mask_w, p->mask_h,
                                                        p->active_mask)) {
                                        p->y_pos = target_y;
                                        y_pixels = 0;
                                        j = 30;
                                        resolved = true;
                                    } else if (p->is_popped) {
                                        if (!obj_processed || !obj_processed[obj]) {
                                            flip_object_velocity(level, obj);
                                            if (obj_processed) obj_processed[obj] = true;
                                        }
                                        resolved = true;
                                    } else {
                                        player_kill(p);
                                        resolved = true;
                                    }
                                }
                            } else if (sv != 0) {
                                int target_x = p->x_pos;
                                int target_y = p->y_pos;
                                if (p->x_pos > left + obj_w) target_x = left + obj_w + p->half_width;
                                else if (p->x_pos < left) target_x = left - p->half_width;
                                if ((!p->gravity_down && p->y_pos < top) || (p->gravity_down && p->y_pos > top + obj_h)) {
                                    target_x = p->x_pos;
                                    target_y = p->gravity_down ? (top + obj_h + p->half_height) : (top - p->half_height);
                                }
                                if (!collision_test(level, tile_meta, masks,
                                                    target_x - p->mask_half_w,
                                                    target_y - p->mask_half_h,
                                                    p->mask_w, p->mask_h,
                                                    p->active_mask)) {
                                    p->x_pos = target_x;
                                    p->y_pos = target_y;
                                    resolved = true;
                                } else if (p->is_popped) {
                                    if (!obj_processed || !obj_processed[obj]) {
                                        flip_object_vertical(level, obj);
                                        if (obj_processed) obj_processed[obj] = true;
                                    }
                                    resolved = true;
                                } else {
                                    player_kill(p);
                                    resolved = true;
                                }
                            }
                        }
                        break;
                    }
                    case 7:
                    case 8:
                        if (p->is_popped) player_apply_tile_break(level, tx, ty);
                        resolved = true;
                        break;
                    case 12:
                    case 30:
                    case 34:
                        player_collect_tile(p, level, tx, ty, id_logic);
                        resolved = true;
                        break;
                    case 11:
                    case 15:
                    case 18:
                    case 22:
                    case 26:
                    case 39:
                        player_special_tile(p, level, tx, ty, id_logic);
                        resolved = true;
                        break;
                    default:
                        // a.java:1173-1182 - try to slide left or right
                        k = 0;
                        {
                            int slid = 0;
                            if (!collision_test(level, tile_meta, masks,
                                                (p->x_pos + 1) - p->mask_half_w,
                                                test_y - p->mask_half_h, p->mask_w, p->mask_h,
                                                p->active_mask)) {
                                p->x_pos += 1;
                                p->y_pos = test_y;
                                slid = 1;
                            } else if (!collision_test(level, tile_meta, masks,
                                                       (p->x_pos - 1) - p->mask_half_w,
                                                       test_y - p->mask_half_h, p->mask_w, p->mask_h,
                                                       p->active_mask)) {
                                p->x_pos -= 1;
                                p->y_pos = test_y;
                                slid = 1;
                            }
                            // a.java:1185-1186 - break tiles after slide
                            if ((id_logic == 7 || id_logic == 8) && p->is_popped) {
                                player_apply_tile_break(level, tx, ty);
                            }
                            // a.java:1187-1188 - early exit if slid
                            if (slid) {
                                resolved = true;
                                break;
                            }
                        }
                        // a.java:1189-1190 - squash animation if can't slide
                        if (j > 40 && !p->stunned) {
                            player_trigger_squash(p, level, tile_meta, masks, 1);
                        }
                        // a.java:1191-1192 - bounce calculation
                        if (!p->has_grav_bonus || !p->is_inverted) {
                            // Bounce logic for default case
                            if (jump_hold) {
                                j = calculate_jump_strength(p);
                                p->is_grounded = false;
                                p->bounce_state = 0;
                            } else if (j > 30) {
                                if (p->bounce_state == 0) {
                                    p->bounce_state = -j;
                                    int min_bounce = calculate_bounce_min(p);
                                    if (p->bounce_state < min_bounce) {
                                        p->bounce_state = min_bounce;
                                    }
                                }
                                j = p->bounce_state;
                                p->bounce_state >>= 1;
                                if (p->bounce_state > -10) {
                                    p->is_grounded = true;
                                    j = 30;
                                    p->bounce_state = 0;
                                }
                            } else if (j > -30) {
                                p->is_grounded = true;
                                j = 30;
                                p->bounce_state = 0;
                            }
                        }
                        // a.java:1193-1229 - arrow tiles: try horizontal shift
                        if (id_logic == 84 || id_logic == 108 || id_logic == 83) {
                            if (!collision_test(level, tile_meta, masks,
                                                (p->x_pos + 1) - p->mask_half_w,
                                                p->y_pos - p->mask_half_h, p->mask_w, p->mask_h,
                                                p->active_mask)) {
                                p->x_pos += 1;
                                resolved = true;
                                break;
                            }
                            // a.java:1198-1209 - check for spike tiles after failed shift
                            if (!p->is_popped) {
                                // Get tile at new position after vertical movement
                                int check_tx = hits.x[0];
                                int check_ty = hits.y[0];
                                if (check_tx != -1 && check_ty != -1) {
                                    uint8_t check_tile = (uint8_t)(level_get_tile(level, check_tx, check_ty) & 0x7F);
                                    if (check_tile == 85 || check_tile == 86 || check_tile == 87 ||
                                        check_tile == 88 || check_tile == 89 || check_tile == 63 ||
                                        check_tile == 64 || check_tile == 92) {
                                        player_kill(p);
                                    }
                                }
                            }
                            resolved = true;
                        } else if (id_logic == 79 || id_logic == 109 || id_logic == 82) {
                            if (!collision_test(level, tile_meta, masks,
                                                (p->x_pos - 1) - p->mask_half_w,
                                                p->y_pos - p->mask_half_h, p->mask_w, p->mask_h,
                                                p->active_mask)) {
                                p->x_pos -= 1;
                                resolved = true;
                                break;
                            }
                            // a.java:1217-1228 - check for spike tiles after failed shift
                            if (!p->is_popped) {
                                // Get tile at new position after vertical movement
                                int check_tx = hits.x[0];
                                int check_ty = hits.y[0];
                                if (check_tx != -1 && check_ty != -1) {
                                    uint8_t check_tile = (uint8_t)(level_get_tile(level, check_tx, check_ty) & 0x7F);
                                    if (check_tile == 85 || check_tile == 86 || check_tile == 87 ||
                                        check_tile == 88 || check_tile == 89 || check_tile == 63 ||
                                        check_tile == 64 || check_tile == 92) {
                                        player_kill(p);
                                    }
                                }
                            }
                            resolved = true;
                        }
                        break;
                }
            }
            if (resolved) break;
        }
        if (resolved) break;
    }

    p->prev_y_speed = k;
    p->y_speed = j;

    if ((p->control_mask & 0x2) != 0 && (p->control_mask & 0x1) == 0) {
        i += p->has_speed_bonus ? ACCEL_BONUS : ACCEL_NORMAL;
        if (p->has_speed_bonus && i > MAX_SPEED_BONUS) i = MAX_SPEED_BONUS;
        if (!p->has_speed_bonus && i > MAX_SPEED_NORMAL) i = MAX_SPEED_NORMAL;
    } else if ((p->control_mask & 0x2) == 0 && (p->control_mask & 0x1) != 0) {
        i -= p->has_speed_bonus ? ACCEL_BONUS : ACCEL_NORMAL;
        if (p->has_speed_bonus && i < -MAX_SPEED_BONUS) i = -MAX_SPEED_BONUS;
        if (!p->has_speed_bonus && i < -MAX_SPEED_NORMAL) i = -MAX_SPEED_NORMAL;
    }

    if (center_tile == 44 || center_tile == 50 || center_tile == 76) {
        if (i > 0) i -= p->is_grounded ? 8 : 3;
        else if (i < 0) i += p->is_grounded ? 8 : 3;
        i -= (center_tile == 76) ? 125 : 250;
    } else if (center_tile == 43 || center_tile == 49 || center_tile == 52 ||
               center_tile == 54 || center_tile == 66 || center_tile == 69 || center_tile == 73) {
        i = 250;
        if (i > 0) i -= p->is_grounded ? 8 : 3;
        else if (i < 0) i += p->is_grounded ? 8 : 3;
    } else if (i > 0) {
        i -= p->is_grounded ? DECEL_GROUNDED : DECEL_AIRBORNE;
        if (i < 0) i = 0;
    } else if (i < 0) {
        i += p->is_grounded ? DECEL_GROUNDED : DECEL_AIRBORNE;
        if (i > 0) i = 0;
    }

    int x_pixels = abs(i);
    x_pixels = (x_pixels < 10 && x_pixels != 0) ? 1 : (x_pixels / 10);
    int step_x = (x_pixels == 0) ? 0 : ((i < 0) ? -1 : 1);
    if (x_pixels > 8) x_pixels = 8;

    for (int s = 0; s < x_pixels; s++) {
        int test_x = p->x_pos + step_x;
        int rect_x = test_x - p->mask_half_w;
        int rect_y = p->y_pos - p->mask_half_h;

        CollisionHits hits;
        collision_hits_clear(&hits);
        bool blocking = collision_test_collect(level, tile_meta, masks,
                                               rect_x, rect_y, p->mask_w, p->mask_h,
                                               p->active_mask, &hits);

        // Match a.java horizontal probe: force center tile 97-100 into hit list.
        // a.java uses i6=(D+b3)/16 and i7=i/16 before switch processing.
        int center_tx = test_x / 16;
        int center_ty = p->y_pos / 16;
        if (center_tx >= 0 && center_ty >= 0 &&
            center_tx < (int)level->width && center_ty < (int)level->height) {
            uint8_t center_id = (uint8_t)(level_get_tile(level, center_tx, center_ty) & 0x7F);
            if (center_id == 97 || center_id == 98 || center_id == 99 || center_id == 100) {
                bool already = false;
                int free_idx = -1;
                for (int hh = 0; hh < COLLISION_HITS_MAX; hh++) {
                    if (hits.x[hh] == center_tx && hits.y[hh] == center_ty) {
                        already = true;
                        break;
                    }
                    if (free_idx < 0 && hits.x[hh] == -1 && hits.y[hh] == -1) {
                        free_idx = hh;
                    }
                }
                if (!already && free_idx >= 0) {
                    hits.x[free_idx] = center_tx;
                    hits.y[free_idx] = center_ty;
                    already = true;
                }
                if (already) {
                    blocking = true;
                }
            }
        }

        if (level->active_object_index >= 0) p->carrier_object_index = level->active_object_index;
        if (!blocking) {
            p->x_pos = test_x;
            if (p->is_grounded && (!p->has_grav_bonus || !p->is_inverted)) {
                int drop_y = p->gravity_down ? -1 : 1;
                if (!collision_test(level, tile_meta, masks,
                                    p->x_pos - p->mask_half_w,
                                    (p->y_pos + drop_y) - p->mask_half_h,
                                    p->mask_w, p->mask_h,
                                    p->active_mask)) {
                    uint8_t under_id = tile_id_at_pixel(level, p->x_pos, p->y_pos + drop_y);
                    if (under_id != 101 && under_id != 102 && under_id != 94) {
                        p->y_pos += drop_y;
                    }
                }
            }
            continue;
        }

        bool resolved = false;
        for (int h = 0; h < COLLISION_HITS_MAX; h++) {
            int tx = hits.x[h];
            int ty = hits.y[h];
            uint8_t hit_id = 0;
            uint8_t id_logic = 0;

            if (tx >= 0 && ty >= 0) {
                hit_id = (uint8_t)(level_get_tile(level, tx, ty) & 0x7F);
                id_logic = hit_id;
            } else if (p->carrier_object_index >= 0 && p->carrier_object_index < level->objects.count) {
                // Match a.java: when tile slot is empty but object is active, synthesize 200/201/202.
                uint8_t obj_type = level->objects.ao[p->carrier_object_index];
                if (obj_type == 2) {
                    id_logic = 200;
                } else if (obj_type == 0) {
                    id_logic = p->is_popped ? 200 : 201;
                } else {
                    id_logic = p->is_popped ? 200 : 202;
                }
                hit_id = id_logic;
            } else {
                continue;
            }

            if (step_x > 0) {
                if (id_logic == 114) id_logic = 110;
                if (id_logic == 5) id_logic = 2;
            } else if (step_x < 0) {
                if (id_logic == 113) id_logic = 110;
                if (id_logic == 6) id_logic = 2;
            }

            if (tx >= 0 && ty >= 0 && !p->is_inverted &&
                (id_logic == 93 || id_logic == 95) &&
                (p->y_pos - p->half_height == ty * 16)) {
                p->x_pos = test_x;
                if (id_logic == 93 && p->x_pos == tx * 16 + 8) {
                    player_collect_tile(p, level, tx, ty, hit_id);
                    update_ring_tile_if_crossed(level, tx, ty, hit_id, p->x_pos, p->y_pos);
                }
                resolved = true;
                break;
            }

            switch (id_logic) {
                case 3:
                case 4:
                case 113:
                case 114:
                    if (!p->gravity_down) {
                        if (!collision_test(level, tile_meta, masks,
                                            test_x - p->mask_half_w,
                                            (p->y_pos - 1) - p->mask_half_h,
                                            p->mask_w, p->mask_h, p->active_mask)) {
                            p->x_pos = test_x;
                            p->y_pos -= 1;
                            resolved = true;
                        } else if (!p->has_grav_bonus || !p->is_inverted) {
                            p->y_pos -= 1;
                            resolved = true;
                        }
                    }
                    break;
                case 5:
                case 6:
                case 115:
                case 116:
                    if (p->gravity_down || (p->has_grav_bonus && p->is_inverted)) {
                        if (!collision_test(level, tile_meta, masks,
                                            test_x - p->mask_half_w,
                                            (p->y_pos + 1) - p->mask_half_h,
                                            p->mask_w, p->mask_h, p->active_mask)) {
                            p->x_pos = test_x;
                            p->y_pos += 1;
                            resolved = true;
                        } else {
                            p->y_pos += 1;
                            resolved = true;
                        }
                    }
                    break;
                case 52:
                case 53:
                case 54:
                case 55: {
                    int dy = p->gravity_down ? 1 : -1;
                    if (!collision_test(level, tile_meta, masks,
                                        test_x - p->mask_half_w,
                                        (p->y_pos + dy) - p->mask_half_h,
                                        p->mask_w, p->mask_h, p->active_mask)) {
                        p->x_pos = test_x;
                        p->y_pos += dy;
                        resolved = true;
                    } else if (!collision_test(level, tile_meta, masks,
                                               test_x - p->mask_half_w,
                                               (p->y_pos - dy) - p->mask_half_h,
                                               p->mask_w, p->mask_h, p->active_mask)) {
                        p->x_pos = test_x;
                        p->y_pos -= dy;
                        resolved = true;
                    }
                    break;
                }
                case 93:
                case 95:
                    // Match a.java case 93/95: if pass-through precondition failed, tile stays blocking.
                    break;
                case 97:
                case 98:
                case 99:
                case 100:
                    if (!collision_test(level, tile_meta, masks,
                                        test_x - p->mask_half_w, p->y_pos - p->mask_half_h,
                                        p->mask_w, p->mask_h, p->active_mask)) {
                        p->x_pos = test_x;
                        if ((id_logic == 97 || id_logic == 98) && p->x_pos == tx * 16 + 8) {
                            player_collect_tile(p, level, tx, ty, id_logic);
                            apply_tile_97_98(level, tx, ty, id_logic);
                        }
                        resolved = true;
                    } else {
                        int dy = p->gravity_down ? 1 : -1;
                        if (!collision_test(level, tile_meta, masks,
                                            test_x - p->mask_half_w,
                                            (p->y_pos + dy) - p->mask_half_h,
                                            p->mask_w, p->mask_h, p->active_mask)) {
                            p->x_pos = test_x;
                            p->y_pos += dy;
                            resolved = true;
                        }
                    }
                    break;
                case 200: {
                    int obj = p->carrier_object_index;
                    if (obj >= 0 && obj < level->objects.count) {
                        int top = level->objects.f[obj][0] * 16 + level->objects.ag[obj][1];
                        int left = level->objects.f[obj][1] * 16 + level->objects.ag[obj][0];
                        int obj_h = (level->objects.ao[obj] == 2) ? 11 : ((level->objects.ao[obj] == 0) ? 32 : 16);
                        int obj_w = (level->objects.ao[obj] == 2) ? 24 : ((level->objects.ao[obj] == 0) ? 32 : 16);
                        int sv = level->objects.s[obj][0];
                        int sh = level->objects.s[obj][1];

                        if (sh != 0) {
                            if ((!p->gravity_down && p->y_pos - p->half_height <= top) ||
                                (p->gravity_down && p->y_pos + p->half_height >= top + obj_h)) {
                                if ((h != 0) || (h + 1 < COLLISION_HITS_MAX && hits.x[h + 1] != -1)) {
                                    if (p->is_popped) {
                                        if (!obj_processed || !obj_processed[obj]) {
                                            flip_object_velocity(level, obj);
                                            if (obj_processed) obj_processed[obj] = true;
                                        }
                                        resolved = true;
                                    } else {
                                        player_kill(p);
                                        resolved = true;
                                    }
                                }
                            } else {
                                int target_y = p->gravity_down ? (top - p->half_height) : (top + obj_h + p->half_height + 2);
                                if (!collision_test(level, tile_meta, masks,
                                                    p->x_pos - p->mask_half_w,
                                                    target_y - p->mask_half_h,
                                                    p->mask_w, p->mask_h,
                                                    p->active_mask)) {
                                    p->y_pos = target_y;
                                    x_pixels = 0;
                                    j = 30;
                                    resolved = true;
                                } else if (p->is_popped) {
                                    if (!obj_processed || !obj_processed[obj]) {
                                        flip_object_velocity(level, obj);
                                        if (obj_processed) obj_processed[obj] = true;
                                    }
                                    resolved = true;
                                } else {
                                    player_kill(p);
                                    resolved = true;
                                }
                            }
                        } else if (sv != 0) {
                            int target_x = p->x_pos;
                            int target_y = p->y_pos;
                            bool centered = false;
                            if (p->x_pos > left + obj_w) {
                                target_x = left + obj_w + p->half_width;
                            } else if (p->x_pos < left) {
                                target_x = left - p->half_width;
                            } else {
                                centered = true;
                            }
                            if (!collision_test(level, tile_meta, masks,
                                                target_x - p->mask_half_w,
                                                target_y - p->mask_half_h,
                                                p->mask_w, p->mask_h,
                                                p->active_mask)) {
                                p->x_pos = target_x;
                                p->y_pos = target_y;
                                if (centered) {
                                    x_pixels = 0;
                                    j = 30;
                                }
                                resolved = true;
                            } else if (p->is_popped) {
                                if (!obj_processed || !obj_processed[obj]) {
                                    flip_object_vertical(level, obj);
                                    if (obj_processed) obj_processed[obj] = true;
                                }
                                resolved = true;
                            } else {
                                player_kill(p);
                                resolved = true;
                            }
                        }
                    }
                    break;
                }
                case 201:
                case 202:
                    // Enemy collision horizontal - always death
                    if (!p->is_popped) {
                        player_kill(p);
                    }
                    i = 0;
                    resolved = true;
                    break;
                case 1:
                case 62:
                case 63:
                case 64:
                case 65:
                case 85:
                case 86:
                case 87:
                case 88:
                case 89:
                case 90:
                case 91:
                case 92:
                    // Spike tiles horizontal - death unless popped
                    if (!p->is_popped) {
                        player_kill(p);
                    }
                    i = 0;
                    resolved = true;
                    break;
                case 12:
                case 30:
                case 34:
                    player_collect_tile(p, level, tx, ty, id_logic);
                    resolved = true;
                    break;
                case 11:
                case 15:
                case 18:
                case 22:
                case 26:
                case 39:
                    player_special_tile(p, level, tx, ty, id_logic);
                    if ((i > 90 || i < -90) && p->is_grounded) {
                        i = -i;
                    } else {
                        i = 0;
                    }
                    resolved = true;
                    break;
                default:
                    if ((id_logic == 7 || id_logic == 8) && p->is_popped) {
                        player_apply_tile_break(level, tx, ty);
                    }
                    // a.java:1622-1624 - squash animation on horizontal collision
                    if ((i > 90 || i < -90) && !p->stunned) {
                        player_trigger_squash(p, level, tile_meta, masks, 3);
                    } else if (p->is_grounded && (i > 90 || i < -90)) {
                        i = -i;
                    } else {
                        i = 0;
                    }
                    resolved = true;
                    break;
            }
            if (resolved) break;
        }
        if (resolved) {
            if (p->is_dying) {
                i = 0;
                break;
            }
            continue;
        }
    }

    if (p->is_grounded) p->bounce_state = 0;
    p->x_speed = i;
    free(obj_processed);
}

void player_render(Player* p, SDL_Renderer* renderer, int camera_x, int camera_y) {
    if (!p || !p->ball_sprites) return;

    // Death animation (a.java:1646-1665): fixed clip square + vertical reveal
    if (p->is_dying && p->timer_a > 13) {
        int death_sprite = p->is_inverted ? 24 : 23;
        int death_size = p->is_inverted ? 32 : 24;
        int half_size = death_size / 2;
        
        SDL_Texture* sprite = p->ball_sprites[death_sprite];
        if (!sprite) return;
        
        int screen_x = p->x_pos - half_size - camera_x;
        int screen_y = p->y_pos - half_size - camera_y;
        
        // Animation frames by timer
        int frame_offset = 0;
        if (p->timer_a <= 21 && p->timer_a > 17) {
            frame_offset = 1;
        } else if (p->timer_a <= 17 && p->timer_a > 13) {
            frame_offset = 2;
        }

        SDL_Rect clip_rect = { screen_x, screen_y, death_size, death_size };
        SDL_Rect prev_clip = {0, 0, 0, 0};
        const SDL_bool had_clip = SDL_RenderIsClipEnabled(renderer);
        if (had_clip) {
            SDL_RenderGetClipRect(renderer, &prev_clip);
        }
        SDL_RenderSetClipRect(renderer, &clip_rect);

        SDL_Rect dest = { screen_x, screen_y - frame_offset * death_size, death_size, death_size };
        SDL_RenderCopy(renderer, sprite, NULL, &dest);

        if (had_clip) {
            SDL_RenderSetClipRect(renderer, &prev_clip);
        } else {
            SDL_RenderSetClipRect(renderer, NULL);
        }
        return;
    }
    
    // Normal render
    if (!p->is_dying) {
        if (p->sprite_index < 0 || p->sprite_index >= p->sprite_count) return;
        
        SDL_Texture* sprite = p->ball_sprites[p->sprite_index];
        if (!sprite) return;
        
        int screen_x = p->x_pos - p->half_width - camera_x;
        int screen_y = p->y_pos - p->half_height - camera_y;
        
        SDL_Rect dest = { screen_x, screen_y, p->sprite_width, p->sprite_height };
        
        SDL_RendererFlip flip = p->is_inverted ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, sprite, NULL, &dest, 0.0, NULL, flip);
    }
}
