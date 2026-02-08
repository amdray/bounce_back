/**
 * Bounce Back - PSP Port
 * Step 2 bring-up: load and render static tileMap from level 0
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "bg_layer.h"
#include "camera.h"
#include "collision_masks.h"
#include "input.h"
#include "level_loader.h"
#include "level_renderer.h"
#include "exit_door.h"
#include "foreground_pass.h"
#include "hud.h"
#include "player.h"
#include "tile_animation.h"
#include "tile_metadata.h"
#include "tileset_loader.h"

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define DEFAULT_LEVEL_INDEX 0
#define LEVEL_COUNT 22

static int reload_level_state(SDL_Renderer* sdl_renderer,
                              int new_level_index,
                              int* level_index,
                              Level** level,
                              BgLayer** bg_layer,
                              ForegroundPass* fg,
                              TileMetadata* tile_meta,
                              TileAnimation* tile_anim,
                              Tileset** tileset,
                              LevelRenderer** level_renderer,
                              Player** player,
                              Camera* camera,
                              ExitDoorState* door,
                              FILE* log) {
    Level* new_level = level_load("res/lf", new_level_index);
    if (!new_level) {
        if (log) fprintf(log, "Level switch failed: level %d load error\n", new_level_index);
        return 0;
    }

    ForegroundPass new_fg = (ForegroundPass){0};
    if (foreground_pass_build(new_level, &new_fg) != 0) {
        if (log) fprintf(log, "Level switch failed: foreground build error (level %d)\n", new_level_index);
        level_free(new_level);
        return 0;
    }

    char theme_path[32];
    snprintf(theme_path, sizeof(theme_path), "res/if%d", (int)new_level->theme_id);
    Tileset* new_tileset = tileset_load(sdl_renderer, "res/if0", theme_path);
    if (!new_tileset) {
        if (log) fprintf(log, "Level switch failed: tileset load error (%s)\n", theme_path);
        free(new_fg.front_tiles);
        level_free(new_level);
        return 0;
    }

    char bg_theme_path[32];
    snprintf(bg_theme_path, sizeof(bg_theme_path), "res/ib%d", (int)new_level->theme_id);
    BgLayer* new_bg = bg_layer_load(sdl_renderer, "res/bg", "res/ib0", bg_theme_path);
    if (!new_bg) {
        if (log) fprintf(log, "Level switch failed: bg layer load error (%s)\n", bg_theme_path);
        tileset_free(new_tileset);
        free(new_fg.front_tiles);
        level_free(new_level);
        return 0;
    }

    LevelRenderer* new_renderer = renderer_create(new_level, tile_meta, tile_anim, new_tileset);
    if (!new_renderer) {
        if (log) fprintf(log, "Level switch failed: renderer create error (level %d)\n", new_level_index);
        bg_layer_free(new_bg);
        tileset_free(new_tileset);
        free(new_fg.front_tiles);
        level_free(new_level);
        return 0;
    }

    Player* new_player = player_create(sdl_renderer, new_level->spawn_x, new_level->spawn_y, new_level->ball_type != 0);
    if (!new_player) {
        if (log) fprintf(log, "Level switch failed: player create error (level %d)\n", new_level_index);
        renderer_free(new_renderer);
        bg_layer_free(new_bg);
        tileset_free(new_tileset);
        free(new_fg.front_tiles);
        level_free(new_level);
        return 0;
    }

    player_free(*player);
    renderer_free(*level_renderer);
    tileset_free(*tileset);
    bg_layer_free(*bg_layer);
    free(fg->front_tiles);
    level_free(*level);

    *player = new_player;
    *level_renderer = new_renderer;
    *tileset = new_tileset;
    *bg_layer = new_bg;
    *fg = new_fg;
    *level = new_level;
    *level_index = new_level_index;

    camera_reset(camera, new_level->width, new_level->height);
    door->I = 0;
    door->open = false;

    if (log) {
        fprintf(log, "Level switched to %d: width=%u height=%u theme=%u\n",
                *level_index, new_level->width, new_level->height, new_level->theme_id);
    }
    return 1;
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    FILE* log = fopen("debug.log", "w");
    if (log) fprintf(log, "=== Bounce Back Debug ===\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        if (log) { fprintf(log, "SDL_Init failed: %s\n", SDL_GetError()); fclose(log); }
        return 1;
    }
    if (log) fprintf(log, "SDL2 OK\n");

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

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

    ForegroundPass fg = (ForegroundPass){0};
    if (foreground_pass_build(level, &fg) != 0) {
        if (log) fclose(log);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }

    TileMetadata* tile_meta = tilemetadata_load("res/tf");
    if (!tile_meta) {
        if (log) { fprintf(log, "Failed to load res/tf\n"); fclose(log); }
        free(fg.front_tiles);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "TileMetadata loaded: 127 tiles\n");

    TileAnimation* tile_anim = animation_load("res/tf");
    if (!tile_anim) {
        if (log) { fprintf(log, "Failed to load tile animation\n"); fclose(log); }
        tilemetadata_free(tile_meta);
        free(fg.front_tiles);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }

    CollisionMasks* collision_masks = collision_masks_load("res/tf");
    if (!collision_masks) {
        if (log) { fprintf(log, "Failed to load collision masks\n"); fclose(log); }
        animation_free(tile_anim);
        tilemetadata_free(tile_meta);
        free(fg.front_tiles);
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
        free(fg.front_tiles);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Tileset loaded: 104 base + 7 theme textures\n");

    char bg_theme_path[32];
    snprintf(bg_theme_path, sizeof(bg_theme_path), "res/ib%d", (int)level->theme_id);
    BgLayer* bg_layer = bg_layer_load(renderer, "res/bg", "res/ib0", bg_theme_path);
    if (!bg_layer) {
        if (log) { fprintf(log, "Failed to load bg layer res/bg + res/ib0 + %s\n", bg_theme_path); fclose(log); }
        tileset_free(tileset);
        collision_masks_free(collision_masks);
        animation_free(tile_anim);
        tilemetadata_free(tile_meta);
        free(fg.front_tiles);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }

    LevelRenderer* level_renderer = renderer_create(level, tile_meta, tile_anim, tileset);
    if (!level_renderer) {
        if (log) { fprintf(log, "Failed to create LevelRenderer\n"); fclose(log); }
        bg_layer_free(bg_layer);
        tileset_free(tileset);
        collision_masks_free(collision_masks);
        animation_free(tile_anim);
        tilemetadata_free(tile_meta);
        free(fg.front_tiles);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }

    Player* player = player_create(renderer, level->spawn_x, level->spawn_y, level->ball_type != 0);
    if (!player) {
        if (log) { fprintf(log, "Failed to create player\n"); fclose(log); }
        renderer_free(level_renderer);
        bg_layer_free(bg_layer);
        tileset_free(tileset);
        collision_masks_free(collision_masks);
        animation_free(tile_anim);
        tilemetadata_free(tile_meta);
        free(fg.front_tiles);
        level_free(level);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Player created at (%d, %d)\n", player->x_pos, player->y_pos);

    if (hud_init(renderer) != 0) {
        if (log) { fprintf(log, "Failed to init HUD\n"); fclose(log); }
        player_free(player);
        bg_layer_free(bg_layer);
        renderer_free(level_renderer);
        collision_masks_free(collision_masks);
        animation_free(tile_anim);
        tilemetadata_free(tile_meta);
        free(fg.front_tiles);
        level_free(level);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    input_init();

    SDL_Event event;
    int running = 1;
    Input input = (Input){0};
    Camera camera = (Camera){0};
    camera_init(&camera);
    camera_reset(&camera, level->width, level->height);

    ExitDoorState door = (ExitDoorState){0};
    door.I = 0;
    door.open = false;

    while (running) {
        int level_delta = 0;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    running = 0;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
                    level_delta = 1;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
                    level_delta = -1;
                }
            }
        }

        if (level_delta != 0) {
            int next_level = (level_index + level_delta + LEVEL_COUNT) % LEVEL_COUNT;
            (void)reload_level_state(renderer,
                                     next_level,
                                     &level_index,
                                     &level,
                                     &bg_layer,
                                     &fg,
                                     tile_meta,
                                     tile_anim,
                                     &tileset,
                                     &level_renderer,
                                     &player,
                                     &camera,
                                     &door,
                                     log);
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

        level_objects_tick(level,
                           player->x_pos - player->half_width - 1,
                           player->y_pos - player->half_height - 1,
                           player->x_pos + player->half_width + 1,
                           player->y_pos + player->half_height + 1,
                           player->is_popped);
        player_update(player, level, tile_meta, collision_masks, &input);

        exit_door_tick(&door, level->hoops_remaining);
        if (exit_door_test_complete(&door, level, player)) {
            if (log) fprintf(log, "LEVEL_COMPLETE\n");
            running = 0;
        }

        camera_update(&camera, player->x_pos, player->y_pos, level->width, level->height);
        if (log) fprintf(log, "cameraX=%d cameraY=%d\n", camera.x, camera.y);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        animation_tick(tile_anim);
        bg_layer_draw(bg_layer, renderer, camera.x >> 1, camera.y >> 1, SCREEN_WIDTH, SCREEN_HEIGHT);
        renderer_draw(level_renderer, renderer, player, &fg, camera.x, camera.y);
        HudState hud = {
            .score = player->score,
            .num_lives = player->lives,
            .total_rings = level->hoops_total,
            .num_rings = level->hoops_total - level->hoops_remaining,
            .speed_bonus_counter = player->has_speed_bonus ? player->timer_b : 0,
            .grav_bonus_counter = player->has_grav_bonus ? player->timer_b : 0,
            .jump_bonus_counter = player->has_jump_bonus ? player->timer_b : 0,
        };
        SDL_Texture* hud_life_ball_icon = (player && player->ball_sprites && player->sprite_count > 0)
                                        ? player->ball_sprites[0]
                                        : NULL;
        hud_render(renderer, tileset, hud_life_ball_icon, &hud);
        SDL_RenderPresent(renderer);

        SDL_Delay(50);
    }

    if (log) { fprintf(log, "Shutdown OK\n"); fclose(log); }
    input_cleanup();
    hud_shutdown();
    player_free(player);
    bg_layer_free(bg_layer);
    renderer_free(level_renderer);
    collision_masks_free(collision_masks);
    animation_free(tile_anim);
    tilemetadata_free(tile_meta);
    free(fg.front_tiles);
    level_free(level);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
