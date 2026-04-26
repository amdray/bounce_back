/**
 * Bounce Back - PSP Port
 * Step 2 bring-up: load and render static tileMap from level 0
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include "bg_layer.h"
#include "camera.h"
#include "enemy_renderer.h"
#include "game_assets.h"
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
#include "menu.h"
#include "save.h"
#include "sound.h"
#include "game_constants.h"

static int clamp_campaign_level_count(int unlocked_level) {
    if (unlocked_level <= 0) {
        return CAMPAIGN_LEVEL_COUNT;
    }
    if (unlocked_level > CAMPAIGN_LEVEL_COUNT) {
        return CAMPAIGN_LEVEL_COUNT;
    }
    return unlocked_level;
}

static int get_next_campaign_level_index(int completed_level_index, bool one_go_run) {
    if (completed_level_index < 0) {
        return -1;
    }
    if (completed_level_index < REGULAR_LEVEL_COUNT - 1) {
        return completed_level_index + 1;
    }
    if (completed_level_index == REGULAR_LEVEL_COUNT - 1 && one_go_run) {
        return HIDDEN_LEVEL_INDEX;
    }
    return -1;
}

static void present_loading_splash(SDL_Renderer* renderer, int phase) {
    if (!renderer) return;
    menu_render_startup_splash(renderer, phase);
    SDL_RenderPresent(renderer);
}

static void present_loading_splash_for(SDL_Renderer* renderer, int phase, uint32_t duration_ms) {
    if (!renderer) return;

    uint32_t start = SDL_GetTicks();
    SDL_Event event;
    Input input = {0};
    while (SDL_GetTicks() - start < duration_ms) {
        while (SDL_PollEvent(&event)) {
            /* Keep the window responsive during splash presentation. */
        }
        input_update(&input);
        if (input.jump || input.confirm_pressed || input.start_pressed) {
            break;
        }
        present_loading_splash(renderer, phase);
        SDL_Delay(16);
    }
}

static void present_startup_menu_intro(SDL_Renderer* renderer, MenuMainState* menu_main) {
    uint32_t start = SDL_GetTicks();
    const uint32_t duration_ms = 22u * 50u;
    SDL_Event event;
    Input input = {0};

    if (!renderer || !menu_main) return;

    while (SDL_GetTicks() - start < duration_ms) {
        uint32_t elapsed = SDL_GetTicks() - start;
        int ticks_elapsed = (int)(elapsed / 50u);
        int ticks_remaining = 22 - ticks_elapsed;
        while (SDL_PollEvent(&event)) {
            /* Keep the window responsive during startup intro. */
        }
        input_update(&input);
        if (input.jump || input.confirm_pressed || input.start_pressed) {
            break;
        }

        if (ticks_remaining < 0) ticks_remaining = 0;
        menu_render_startup_menu_intro(renderer, menu_main, ticks_remaining);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

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
                              ExitDoorState* door) {
    Level* new_level = level_load("res/lf", new_level_index);
    if (!new_level) return 0;

    ForegroundPass new_fg = (ForegroundPass){0};
    if (foreground_pass_build(new_level, &new_fg) != 0) {
        level_free(new_level);
        return 0;
    }

    char theme_path[32];
    snprintf(theme_path, sizeof(theme_path), "res/if%d", (int)new_level->theme_id);
    Tileset* new_tileset = tileset_load(sdl_renderer, "res/if0", theme_path);
    if (!new_tileset) {
        free(new_fg.front_tiles);
        level_free(new_level);
        return 0;
    }

    char bg_theme_path[32];
    snprintf(bg_theme_path, sizeof(bg_theme_path), "res/ib%d", (int)new_level->theme_id);
    BgLayer* new_bg = bg_layer_load(sdl_renderer, "res/bg", "res/ib0", bg_theme_path);
    if (!new_bg) {
        tileset_free(new_tileset);
        free(new_fg.front_tiles);
        level_free(new_level);
        return 0;
    }

    LevelRenderer* new_renderer = renderer_create(new_level, tile_meta, tile_anim, new_tileset);
    if (!new_renderer) {
        bg_layer_free(new_bg);
        tileset_free(new_tileset);
        free(new_fg.front_tiles);
        level_free(new_level);
        return 0;
    }

    Player* new_player = player_create(sdl_renderer, new_level->spawn_x, new_level->spawn_y, new_level->ball_type != 0);
    if (!new_player) {
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
    sound_stop_all();

    return 1;
}

static int restore_continue_state(SDL_Renderer* renderer,
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
                                  int* total_score,
                                  int* levels_done,
                                  bool* one_go_run,
                                  uint32_t* level_start_ticks) {
    SaveContinueState state = save_get_continue_state();
    uint32_t now = SDL_GetTicks();

    if (state == SAVE_CONTINUE_NONE) {
        return 0;
    }

    if (state == SAVE_CONTINUE_LEVEL_COMPLETE) {
        int next_level_index = get_next_campaign_level_index(save_get_continue_level_index(),
                                                             save_get_continue_one_go());
        if (next_level_index < 0) {
            return 0;
        }
        if (!reload_level_state(renderer, next_level_index,
                                level_index, level, bg_layer, fg,
                                tile_meta, tile_anim, tileset,
                                level_renderer, player, camera, door)) {
            return 0;
        }
        (*player)->lives = save_get_continue_lives();
        *total_score = save_get_continue_total_score();
        *levels_done = save_get_continue_levels_done();
        if (one_go_run) *one_go_run = save_get_continue_one_go();
        *level_start_ticks = now;
        return 1;
    }

    if (state == SAVE_CONTINUE_GAME) {
        int saved_level_index = save_get_continue_level_index();
        if (!reload_level_state(renderer, saved_level_index,
                                level_index, level, bg_layer, fg,
                                tile_meta, tile_anim, tileset,
                                level_renderer, player, camera, door)) {
            return 0;
        }
        if (!save_restore_game(*level, *player, door)) {
            return 0;
        }
        *total_score = save_get_continue_total_score();
        *levels_done = save_get_continue_levels_done();
        if (one_go_run) *one_go_run = save_get_continue_one_go();
        {
            uint32_t elapsed = save_get_continue_level_elapsed_ms();
            *level_start_ticks = (elapsed <= now) ? (now - elapsed) : 0;
        }
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    int exit_code = 1;
    bool bootstrap_complete = false;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    Level* level = NULL;
    ForegroundPass fg = (ForegroundPass){0};
    TileMetadata* tile_meta = NULL;
    TileAnimation* tile_anim = NULL;
    Tileset* tileset = NULL;
    BgLayer* bg_layer = NULL;
    LevelRenderer* level_renderer = NULL;
    Player* player = NULL;
    GameAssets assets = {0};

    int level_index = DEFAULT_LEVEL_INDEX;
    SDL_Event event;
    int running = 1;
    Input input = (Input){0};
    Camera camera = (Camera){0};
    ExitDoorState door = (ExitDoorState){0};
    int total_score = 0;
    int levels_done = 0;
    bool one_go_run = false;
    uint32_t level_start_ticks = 0;
    AppState app_state = APP_STATE_MENU;
    MenuMainState menu_main = { .selection = 1, .has_save = false };
    MenuLevelSelectState level_select = { .selection = 0, .top_index = 0, .level_count = 1 };
    MenuOverlayData overlay = {0};

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) < 0) {
        goto cleanup;
    }

    sound_init();

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        goto cleanup;
    }

    window = SDL_CreateWindow("Bounce Back", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) goto cleanup;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) goto cleanup;

    input_init();
    menu_init(renderer);
    (void)save_init();
    present_loading_splash_for(renderer, 0, SPLASH_PHASE_MS);

    uint32_t splash_phase2_start = SDL_GetTicks();
    sound_play(SND_MENU_SPLASH);
    present_loading_splash(renderer, 1);

    if (game_assets_init(&assets, renderer) != 0) goto cleanup;

    present_loading_splash(renderer, 1);
    level = level_load("res/lf", level_index);
    if (!level) goto cleanup;

    present_loading_splash(renderer, 1);
    if (foreground_pass_build(level, &fg) != 0) goto cleanup;

    present_loading_splash(renderer, 1);
    tile_meta = tilemetadata_load("res/tf");
    if (!tile_meta) goto cleanup;

    present_loading_splash(renderer, 1);
    tile_anim = animation_load("res/tf");
    if (!tile_anim) goto cleanup;

    char theme_path[32];
    snprintf(theme_path, sizeof(theme_path), "res/if%d", (int)level->theme_id);

    present_loading_splash(renderer, 1);
    tileset = tileset_load(renderer, "res/if0", theme_path);
    if (!tileset) goto cleanup;

    char bg_theme_path[32];
    snprintf(bg_theme_path, sizeof(bg_theme_path), "res/ib%d", (int)level->theme_id);
    present_loading_splash(renderer, 1);
    bg_layer = bg_layer_load(renderer, "res/bg", "res/ib0", bg_theme_path);
    if (!bg_layer) goto cleanup;

    present_loading_splash(renderer, 1);
    level_renderer = renderer_create(level, tile_meta, tile_anim, tileset);
    if (!level_renderer) goto cleanup;

    present_loading_splash(renderer, 1);
    player = player_create(renderer, level->spawn_x, level->spawn_y, level->ball_type != 0);
    if (!player) goto cleanup;

    present_loading_splash(renderer, 1);
    if (hud_init(renderer, &assets.ic) != 0) goto cleanup;

    present_loading_splash(renderer, 1);
    if (object_renderer_init(&assets.ic) != 0) goto cleanup;

    present_loading_splash(renderer, 1);
    if (exit_door_renderer_init(&assets.ic) != 0) goto cleanup;

    present_loading_splash(renderer, 1);
    if (foreground_pass_init(&assets.ic) != 0) goto cleanup;

    Input splash_input = {0};
    while (SDL_GetTicks() - splash_phase2_start < SPLASH_PHASE_MS) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            /* Keep the window responsive during splash presentation. */
        }
        input_update(&splash_input);
        if (splash_input.jump || splash_input.confirm_pressed || splash_input.start_pressed) {
            break;
        }
        present_loading_splash(renderer, 1);
        SDL_Delay(16);
    }
    present_startup_menu_intro(renderer, &menu_main);
    input_sync(&input);
    camera_init(&camera);
    camera_reset(&camera, level->width, level->height);
    door.I = 0;
    door.open = false;
    level_start_ticks = SDL_GetTicks();
    menu_main.has_save = save_has_continue();
    bootstrap_complete = true;

    while (running) {
        uint32_t tick_start = SDL_GetTicks();

        /* ── SDL event pump ──────────────────────────────────── */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        input_update(&input);

        /* ════════════════════════════════════════════════════
           STATE: MENU
           ════════════════════════════════════════════════════ */
        if (app_state == APP_STATE_MENU) {
            AppState next = menu_update_main(&menu_main, &input);

            if (next == APP_STATE_QUIT) {
                running = 0;
                continue;
            }

            if (next == APP_STATE_LEVEL_SELECT) {
                level_select.selection = DEFAULT_LEVEL_INDEX;
                level_select.top_index = 0;
                level_select.level_count = clamp_campaign_level_count(save_get_unlocked_level());
                app_state = APP_STATE_LEVEL_SELECT;
                continue;
            }

            if (next == APP_STATE_GAME) {
                if (!restore_continue_state(renderer, &level_index, &level, &bg_layer, &fg,
                                            tile_meta, tile_anim, &tileset,
                                            &level_renderer, &player, &camera, &door,
                                            &total_score, &levels_done, &one_go_run, &level_start_ticks)) {
                    menu_main.has_save = save_has_continue();
                    continue;
                }
                app_state = APP_STATE_GAME;
                continue;
            }

            /* render game world behind dim menu overlay */
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            animation_tick(tile_anim);
            bg_layer_draw(bg_layer, renderer, camera.x >> 1, camera.y >> 1, SCREEN_WIDTH, SCREEN_HEIGHT);
            renderer_draw(level_renderer, renderer, camera.x, camera.y);
            menu_render_main(renderer, &menu_main);
            SDL_RenderPresent(renderer);
            { uint32_t el = SDL_GetTicks() - tick_start; if (el < 50) SDL_Delay(50 - el); }
            continue;
        }

        if (app_state == APP_STATE_LEVEL_SELECT) {
            AppState next = menu_update_level_select(&level_select, &input);

            if (next == APP_STATE_MENU) {
                app_state = APP_STATE_MENU;
                continue;
            }

            if (next == APP_STATE_GAME) {
                total_score = 0;
                levels_done = 0;
                one_go_run = (level_select.selection == 0);
                save_clear_continue();
                save_flush();
                menu_main.has_save = save_has_continue();
                if (reload_level_state(renderer, level_select.selection,
                                       &level_index, &level, &bg_layer, &fg,
                                       tile_meta, tile_anim, &tileset,
                                       &level_renderer, &player, &camera, &door)) {
                    player->lives = 3;
                    level_start_ticks = SDL_GetTicks();
                }
                app_state = APP_STATE_GAME;
                continue;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            animation_tick(tile_anim);
            bg_layer_draw(bg_layer, renderer, camera.x >> 1, camera.y >> 1, SCREEN_WIDTH, SCREEN_HEIGHT);
            renderer_draw(level_renderer, renderer, camera.x, camera.y);
            menu_render_level_select(renderer, &level_select);
            SDL_RenderPresent(renderer);
            { uint32_t el = SDL_GetTicks() - tick_start; if (el < 50) SDL_Delay(50 - el); }
            continue;
        }

        /* ════════════════════════════════════════════════════
           STATE: LEVEL_COMPLETE overlay
           ════════════════════════════════════════════════════ */
        if (app_state == APP_STATE_LEVEL_COMPLETE) {
            AppState next = menu_update_level_complete(&overlay, &input);
            if (next == APP_STATE_GAME) {
                int next_level_index = get_next_campaign_level_index(level_index, one_go_run);
                int preserved_lives = player->lives;
                if (next_level_index >= 0 &&
                    reload_level_state(renderer, next_level_index,
                                       &level_index, &level, &bg_layer, &fg,
                                       tile_meta, tile_anim, &tileset,
                                       &level_renderer, &player, &camera, &door)) {
                    player->lives = preserved_lives;
                    level_start_ticks = SDL_GetTicks();
                }
                app_state = APP_STATE_GAME;
            }

            /* render game world behind overlay */
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            animation_tick(tile_anim);
            bg_layer_draw(bg_layer, renderer, camera.x >> 1, camera.y >> 1, SCREEN_WIDTH, SCREEN_HEIGHT);
            renderer_draw(level_renderer, renderer, camera.x, camera.y);
            menu_render_level_complete(renderer, &overlay);
            SDL_RenderPresent(renderer);
            { uint32_t el = SDL_GetTicks() - tick_start; if (el < 50) SDL_Delay(50 - el); }
            continue;
        }

        /* ════════════════════════════════════════════════════
           STATE: GAME_OVER overlay
           ════════════════════════════════════════════════════ */
        if (app_state == APP_STATE_GAME_OVER) {
            AppState next = menu_update_game_over(&overlay, &input);
            if (next == APP_STATE_MENU) {
                app_state = APP_STATE_MENU;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            animation_tick(tile_anim);
            bg_layer_draw(bg_layer, renderer, camera.x >> 1, camera.y >> 1, SCREEN_WIDTH, SCREEN_HEIGHT);
            renderer_draw(level_renderer, renderer, camera.x, camera.y);
            menu_render_game_over(renderer, &overlay);
            SDL_RenderPresent(renderer);
            { uint32_t el = SDL_GetTicks() - tick_start; if (el < 50) SDL_Delay(50 - el); }
            continue;
        }

        /* ════════════════════════════════════════════════════
           STATE: CONGRATULATIONS overlay
           ════════════════════════════════════════════════════ */
        if (app_state == APP_STATE_CONGRATULATIONS) {
            AppState next = menu_update_congratulations(&overlay, &input);
            if (next == APP_STATE_GAME) {
                int next_level_index = get_next_campaign_level_index(level_index, one_go_run);
                int preserved_lives = player->lives;
                if (next_level_index >= 0 &&
                    reload_level_state(renderer, next_level_index,
                                       &level_index, &level, &bg_layer, &fg,
                                       tile_meta, tile_anim, &tileset,
                                       &level_renderer, &player, &camera, &door)) {
                    player->lives = preserved_lives;
                    level_start_ticks = SDL_GetTicks();
                }
                save_clear_continue();
                save_flush();
                menu_main.has_save = save_has_continue();
                app_state = APP_STATE_GAME;
            } else if (next == APP_STATE_MENU) {
                app_state = APP_STATE_MENU;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            animation_tick(tile_anim);
            bg_layer_draw(bg_layer, renderer, camera.x >> 1, camera.y >> 1, SCREEN_WIDTH, SCREEN_HEIGHT);
            renderer_draw(level_renderer, renderer, camera.x, camera.y);
            menu_render_congratulations(renderer, &overlay);
            SDL_RenderPresent(renderer);
            { uint32_t el = SDL_GetTicks() - tick_start; if (el < 50) SDL_Delay(50 - el); }
            continue;
        }

        /* ════════════════════════════════════════════════════
           STATE: GAME — main gameplay tick
           ════════════════════════════════════════════════════ */
        /* Start → pause to menu */
        if (input.start_pressed) {
            save_capture_game(level_index, total_score, levels_done,
                              one_go_run,
                              SDL_GetTicks() - level_start_ticks, level, player, &door);
            save_flush();
            menu_main.has_save = save_has_continue();
            app_state = APP_STATE_MENU;
            continue;
        }

        level_objects_tick(level,
                           player->x_pos - player->half_width - 1,
                           player->y_pos - player->half_height - 1,
                           player->x_pos + player->half_width + 1,
                           player->y_pos + player->half_height + 1,
                           player->is_stone);

        /* ── Exit door ───────────────────────────────────── */
        exit_door_tick(&door, level->hoops_remaining);
        if (exit_door_test_complete(&door, level, player)) {
            sound_play(SND_EXIT);
            uint32_t level_timer_ms = SDL_GetTicks() - level_start_ticks;
            int level_seconds = (int)(level_timer_ms / 1000U);
            int time_bonus = (1200 - level_seconds) * (level_index + 1);
            if (time_bonus < 0) time_bonus = 0;
            int level_points_bonus = player->score / 10;
            int stage_bonus = time_bonus + level_points_bonus;
            total_score += stage_bonus;
            levels_done++;

            overlay = (MenuOverlayData){
                .level_index   = level_index,
                .level_points  = player->score,
                .time_bonus    = time_bonus,
                .stage_bonus   = stage_bonus,
                .total_score   = total_score,
                .lives_done    = levels_done,
            };

            save_update_unlocked_level(level_index + 2);

            if (level_index == REGULAR_LEVEL_COUNT - 1) {
                if (one_go_run) {
                    save_capture_level_complete(level_index, player->lives, levels_done, total_score, true,
                                                level, player, &door);
                    save_flush();
                } else {
                    save_insert_record(total_score);
                    save_clear_continue();
                    save_flush();
                }
                menu_main.has_save = save_has_continue();
                overlay.continue_to_game = one_go_run;
                overlay.full_run_message = one_go_run;
                app_state = APP_STATE_CONGRATULATIONS;
                continue;
            }

            if (level_index == HIDDEN_LEVEL_INDEX) {
                save_insert_record(total_score);
                save_clear_continue();
                save_flush();
                menu_main.has_save = save_has_continue();
                overlay.continue_to_game = false;
                overlay.full_run_message = false;
                app_state = APP_STATE_CONGRATULATIONS;
                continue;
            }

            save_capture_level_complete(level_index, player->lives, levels_done, total_score, one_go_run,
                                        level, player, &door);
            save_flush();
            menu_main.has_save = save_has_continue();
            app_state = APP_STATE_LEVEL_COMPLETE;
            continue;
        }

        player_update(player, level, &input);
        level_breaking_tiles_tick(level);

        /* ── Game Over: lives exhausted ──────────────────── */
        if (player->lives <= 0) {
            overlay = (MenuOverlayData){
                .level_index = level_index,
                .lives_done  = levels_done,
                .total_score = total_score,
            };
            save_insert_record(total_score);
            save_clear_continue();
            save_flush();
            menu_main.has_save = save_has_continue();
            total_score = 0;
            levels_done = 0;
            one_go_run = false;
            /* reset world to level 0 so background is valid */
            if (reload_level_state(renderer, DEFAULT_LEVEL_INDEX,
                                   &level_index, &level, &bg_layer, &fg,
                                   tile_meta, tile_anim, &tileset,
                                   &level_renderer, &player, &camera, &door)) {
                player->lives = 3;
                level_start_ticks = SDL_GetTicks();
            }
            app_state = APP_STATE_GAME_OVER;
            continue;
        }

        /* ── Render game frame ───────────────────────────── */
        camera_update(&camera, player->x_pos, player->y_pos, level->width, level->height);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        animation_tick(tile_anim);
        bg_layer_draw(bg_layer, renderer, camera.x >> 1, camera.y >> 1, SCREEN_WIDTH, SCREEN_HEIGHT);
        renderer_draw(level_renderer, renderer, camera.x, camera.y);
        exit_door_render(renderer, &door, level, camera.x, camera.y);
        object_renderer_draw(renderer, level, camera.x, camera.y);
        player_render(player, renderer, camera.x, camera.y);
        foreground_pass_draw(renderer, level, tile_meta, tile_anim, tileset, &fg, camera.x, camera.y);
        int bonus_counter = 0;
        if (player->has_speed_bonus || player->gravity_down || player->has_jump_bonus) {
            bonus_counter = player->timer_b;
        }
        if (player->is_stone) {
            bonus_counter = player->stone_timer;
        }

        HudState hud = {
            .score = player->score,
            .num_lives = player->lives,
            .total_rings = level->hoops_total,
            .num_rings = level->hoops_total - level->hoops_remaining,
            .bonus_counter = bonus_counter,
        };
        SDL_Texture* hud_life_ball_icon = (player && player->ball_sprites && player->sprite_count > 0)
                                        ? player->ball_sprites[0]
                                        : NULL;
        hud_render(renderer, tileset, hud_life_ball_icon, &hud);
        SDL_RenderPresent(renderer);

        { uint32_t el = SDL_GetTicks() - tick_start; if (el < 50) SDL_Delay(50 - el); }
    }

    exit_code = 0;

cleanup:
    if (bootstrap_complete && app_state == APP_STATE_GAME) {
        save_capture_game(level_index, total_score, levels_done,
                          one_go_run,
                          SDL_GetTicks() - level_start_ticks, level, player, &door);
    } else if (bootstrap_complete && app_state == APP_STATE_LEVEL_COMPLETE) {
        save_capture_level_complete(level_index, player->lives, levels_done, total_score, one_go_run,
                                    level, player, &door);
    }
    save_shutdown();

    menu_shutdown();
    input_cleanup();
    foreground_pass_shutdown();
    exit_door_renderer_shutdown();
    object_renderer_shutdown();
    hud_shutdown();
    game_assets_shutdown(&assets);
    player_free(player);
    bg_layer_free(bg_layer);
    renderer_free(level_renderer);
    animation_free(tile_anim);
    tilemetadata_free(tile_meta);
    free(fg.front_tiles);
    level_free(level);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    sound_shutdown();
    SDL_Quit();

    return exit_code;
}
