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

#include "input.h"
#include "level_loader.h"
#include "player_masks.h"

// Jumps (a.java:543-568)
#define JUMP_NORMAL    -125
#define JUMP_INVERTED  -180
#define JUMP_POPPED     -95
#define BOUNCE_NORMAL   -83
#define BOUNCE_INVERTED -120
#define BOUNCE_POPPED   -63

// Movement (a.java:1393-1430)
#define ACCEL_NORMAL     18
#define ACCEL_BONUS      22
#define MAX_SPEED_NORMAL 60
#define MAX_SPEED_BONUS  100
#define DECEL_GROUNDED   8
#define DECEL_AIRBORNE   3

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
    bool is_popped;    // this.F
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
    int timer_c;          // this.b
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
    int x_speed;
    int y_speed;
    int prev_y_speed;
    int sprite_index;
    bool is_large;
    bool is_inverted;
    bool is_popped;
    bool gravity_down;
    bool is_grounded;
    bool has_speed_bonus;
    bool has_jump_bonus;
    bool has_grav_bonus;
    bool stunned;
    int control_mask;
    int bounce_state;
    int timer_a;
    int timer_b;
    int timer_c;
    int state_r;
    int state_a;
    int carrier_object_index;
    bool is_dying;
    int spawn_tile_x;
    int spawn_tile_y;
    bool spawn_is_large;
    bool god_mode;
    int lives;
    int score;
} PlayerSaveState;

Player* player_create(SDL_Renderer* renderer, int spawn_x_tiles, int spawn_y_tiles, bool is_large_ball);
void player_free(Player* p);
void player_update(Player* p, Level* level, Input* input);
void player_render(Player* p, SDL_Renderer* renderer, int camera_x, int camera_y);
void player_export_state(const Player* p, PlayerSaveState* out_state);
bool player_import_state(Player* p, const PlayerSaveState* state);

#endif // PLAYER_H
