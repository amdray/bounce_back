/**
 * Bounce Back - PSP Port
 * Step 2 bring-up: load and render static tileMap from level 0
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "camera.h"
#include "collision_masks.h"
#include "input.h"
#include "level_loader.h"
#include "level_renderer.h"
#include "player.h"
#include "tile_animation.h"
#include "tile_metadata.h"
#include "tileset_loader.h"

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define DEFAULT_LEVEL_INDEX 0

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    FILE* log = fopen("debug.log", "w");
    if (log) fprintf(log, "=== Bounce Back Debug ===\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        if (log) { fprintf(log, "SDL_Init failed: %s\n", SDL_GetError()); fclose(log); }
        return 1;
    }
    if (log) fprintf(log, "SDL2 OK\n");

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        if (log) { fprintf(log, "IMG_Init failed: %s\n", IMG_GetError()); fclose(log); }
        SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "SDL2_image OK\n");

    SDL_Window* window = SDL_CreateWindow("Bounce Back", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        if (log) { fprintf(log, "CreateWindow failed: %s\n", SDL_GetError()); fclose(log); }
        IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Window OK\n");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        if (log) { fprintf(log, "CreateRenderer failed: %s\n", SDL_GetError()); fclose(log); }
        SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Renderer OK\n");

    int level_index = DEFAULT_LEVEL_INDEX;
    Level* level = level_load("res/lf", level_index);
    if (!level) {
        if (log) { fprintf(log, "Failed to load res/lf level %d\n", level_index); fclose(log); }
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Level %d loaded: width=%u height=%u theme=%u\n", level_index, level->width, level->height, level->theme_id);

    TileMetadata* tile_meta = tilemetadata_load("res/tf");
    if (!tile_meta) {
        if (log) { fprintf(log, "Failed to load res/tf\n"); fclose(log); }
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "TileMetadata loaded: 127 tiles\n");

    TileAnimation* tile_anim = animation_load("res/tf");
    if (!tile_anim) {
        if (log) { fprintf(log, "Failed to load tile animation\n"); fclose(log); }
        tilemetadata_free(tile_meta);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }

    CollisionMasks* collision_masks = collision_masks_load("res/tf");
    if (!collision_masks) {
        if (log) { fprintf(log, "Failed to load collision masks\n"); fclose(log); }
        animation_free(tile_anim);
        tilemetadata_free(tile_meta);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }

    char theme_path[32];
    snprintf(theme_path, sizeof(theme_path), "res/if%d", (int)level->theme_id);

    Tileset* tileset = tileset_load(renderer, "res/if0", theme_path);
    if (!tileset) {
        if (log) { fprintf(log, "Failed to load tileset res/if0 + %s\n", theme_path); fclose(log); }
        tilemetadata_free(tile_meta);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Tileset loaded: 104 base + 7 theme textures\n");

    LevelRenderer* level_renderer = renderer_create(level, tile_meta, tile_anim, tileset);
    if (!level_renderer) {
        if (log) { fprintf(log, "Failed to create LevelRenderer\n"); fclose(log); }
        tileset_free(tileset);
        animation_free(tile_anim);
        tilemetadata_free(tile_meta);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }

    Player* player = player_create(renderer, level->spawn_x, level->spawn_y, level->ball_type != 0);
    if (!player) {
        if (log) { fprintf(log, "Failed to create player\n"); fclose(log); }
        renderer_free(level_renderer);
        tileset_free(tileset);
        tilemetadata_free(tile_meta);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Player created at (%d, %d)\n", player->x_pos, player->y_pos);

    input_init();

    SDL_Event event;
    int running = 1;
    Input input = (Input){0};
    Camera camera = (Camera){0};
    camera_init(&camera);
    camera_reset(&camera, level->width, level->height);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    running = 0;
                }
            }
        }

        input_update(&input);

        if (log) {
            if (input.right && !input.left) fprintf(log, "xSpeed=%d\n", ACCEL_NORMAL);
            if (input.left && !input.right) fprintf(log, "xSpeed=%d\n", -ACCEL_NORMAL);
            if (input.jump && !input.down && player->is_grounded) {
                fprintf(log, "jump=%d\n", JUMP_NORMAL);
                fprintf(log, "ySpeed=%d\n", JUMP_NORMAL);
            }
        }

        player_update(player, level, tile_meta, collision_masks, &input);
        camera_update(&camera, player->x_pos, player->y_pos, level->width, level->height);
        if (log) fprintf(log, "cameraX=%d cameraY=%d\n", camera.x, camera.y);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        animation_tick(tile_anim);
        if (log) {
            for (int i = 0; i < tile_anim->count; i++) {
                if (tile_anim->timer[i] == tile_anim->period[i]) {
                    fprintf(log, "anim_group=%d frame=%d frame_index=%d\n", i, tile_anim->frame_index[i], tile_anim->frame_index[i]);
                }
            }
        }

        renderer_draw(level_renderer, renderer, camera.x, camera.y);
        player_render(player, renderer, camera.x, camera.y);
        SDL_RenderPresent(renderer);

        SDL_Delay(50);
    }

    if (log) { fprintf(log, "Shutdown OK\n"); fclose(log); }
    input_cleanup();
    player_free(player);
    renderer_free(level_renderer);
    collision_masks_free(collision_masks);
    animation_free(tile_anim);
    tilemetadata_free(tile_meta);
    level_free(level);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
