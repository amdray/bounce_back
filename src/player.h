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

#include "collision_masks.h"
#include "input.h"
#include "level_loader.h"
#include "tile_metadata.h"

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

    // Resources
    SDL_Texture** ball_sprites;
    int sprite_count;
} Player;

Player* player_create(SDL_Renderer* renderer, int spawn_x_tiles, int spawn_y_tiles, bool is_large_ball);
void player_free(Player* p);
void player_update(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks, Input* input);
void player_render(Player* p, SDL_Renderer* renderer, int camera_x, int camera_y);

#endif // PLAYER_H
