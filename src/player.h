/**
 * Player (ball) for Bounce Back - Step 3.
 *
 * Spec: docs/STEP_03_PLAYER_PHYSICS.md
 * Reference: docs/DEOBFUSCATION.md (Player a.java)
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "game_constants.h"
#include "input.h"
#include "level_loader.h"
#include "player_masks.h"

typedef struct {
    // Position (pixels) - center of ball
    int x_pos;        // this.D
    int y_pos;        // this.i

    // Speeds (pixels/tick)
    int x_speed;      // this.s
    int y_speed;      // this.h
    int prev_y_speed; // this.z

    // Sprite and dimensions
    int sprite_index;  // this.g
    int sprite_width;  // this.d
    int sprite_height; // this.u
    int half_width;    // this.J
    int half_height;   // this.c

    // State
    bool is_large;     // this.t
    bool is_inverted;  // this.I
    bool is_stone;     // this.F
    bool gravity_down; // this.p
    bool is_grounded;  // this.x
    bool has_speed_bonus; // this.j
    bool has_jump_bonus;  // this.C
    bool has_grav_bonus;  // this.l
    bool stunned;         // this.q (reduced in port)
    int control_mask;     // this.m
    int bounce_state;     // this.G
    int timer_a;          // this.A
    int timer_b;          // this.B
    int stone_timer;      // this.b
    int state_r;          // this.r
    int state_a;          // this.a
    int carrier_object_index; // this.k (active moving object index)

    // Death/respawn
    bool is_dying;            // this.e
    int spawn_tile_x;         // this.H - spawn checkpoint X
    int spawn_tile_y;         // this.n - spawn checkpoint Y
    bool spawn_is_large;      // original is_large at spawn
    bool god_mode;            // Cheat: invincibility (R+L)
    bool prev_cheat_pressed;  // Edge-state for L+R god mode toggle
    int lives;                // CrystalMidlet.h analog for HUD
    int score;                // Score counter for HUD

    // Resources
    SDL_Texture** ball_sprites;
    int sprite_count;

    // Player masks (/res/b chunk[0]) - Step 10
    PlayerMasks* masks;
    int mask_w;
    int mask_h;
    int mask_half_w;
    int mask_half_h;
    const bool* active_mask;
} Player;

typedef struct {
    int x_pos;
    int y_pos;
    int spawn_tile_y;
    int spawn_tile_x;
    int x_speed;
    int y_speed;
    int bounce_state;
    int prev_y_speed;
    int stone_timer;
    int timer_b;
    bool is_grounded;
    bool is_stone;
    bool is_inverted;
    bool gravity_down;
    bool stunned;
    int state_a;
    int state_r;
    int timer_a;
    int sprite_index;
} PlayerRmsState;

Player* player_create(SDL_Renderer* renderer, int spawn_x_tiles, int spawn_y_tiles, bool is_large_ball);
void player_free(Player* p);
void player_update(Player* p, Level* level, Input* input);
void player_render(Player* p, SDL_Renderer* renderer, int camera_x, int camera_y);
void player_export_rms_state(const Player* p, PlayerRmsState* out_state);
bool player_import_rms_state(Player* p, Level* level, const PlayerRmsState* state);

#endif // PLAYER_H
