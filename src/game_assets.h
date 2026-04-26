/**
 * game_assets.h — shared, pre-loaded game assets.
 *
 * All resources that live for the entire runtime (res/ic textures) are owned
 * here.  Gameplay modules (hud, object renderer, exit_door, foreground_pass)
 * receive const pointers into these structs via their init() calls and never
 * load res/ic themselves.
 */

#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include <SDL2/SDL.h>
#include <stdio.h>

/**
 * Pre-decoded textures from res/ic.
 *
 * Indices match the original J2ME ic[] array:
 *   ic[1]   — HUD lives strip
 *   ic[2]   — HUD ring icon
 *   ic[3–6] — hoop overlay sprites (foreground_pass)
 *   ic[7]   — exit door sprite sheet (32×96)
 *   ic[8–11]— moving object sprites (lift + enemies)
 */
typedef struct {
    SDL_Texture* lives_strip;   /* ic[1] */
    SDL_Texture* ring_icon;     /* ic[2] */
    SDL_Texture* hoop[4];       /* ic[3..6] */
    SDL_Texture* door;          /* ic[7]  */
    SDL_Texture* object_sprite[4]; /* ic[8..11] */
} IcAssets;

/**
 * Root asset container.  Extend with MenuAssets, BallAssets, TileAssets, …
 * in later increments.
 */
typedef struct {
    IcAssets ic;
} GameAssets;

/**
 * Load and decode all shared assets.
 *
 * @param assets   Output struct (must not be NULL).
 * @param renderer SDL renderer used to create textures.
 * @return 0 on success, -1 on failure.
 */
int  game_assets_init(GameAssets* assets, SDL_Renderer* renderer);

/**
 * Destroy all textures owned by GameAssets.
 * Safe to call even if game_assets_init was not called or failed partially.
 */
void game_assets_shutdown(GameAssets* assets);

#endif /* GAME_ASSETS_H */
