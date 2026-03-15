#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <stdbool.h>

/* ── App-level state machine ────────────────────────────────── */
typedef enum {
    APP_STATE_MENU,
    APP_STATE_LEVEL_SELECT,
    APP_STATE_GAME,
    APP_STATE_LEVEL_COMPLETE,
    APP_STATE_GAME_OVER,
    APP_STATE_CONGRATULATIONS,
    APP_STATE_QUIT              /* Exit selected in main menu    */
} AppState;

/* ── Data passed to overlay screens (filled by game loop) ────── */
typedef struct {
    int level_index;       /* 0-based level that was just completed     */
    int level_points;      /* player->score at completion               */
    int time_bonus;        /* (1200 - secs) * (level_index+1), clamped */
    int stage_bonus;       /* time_bonus + level_points/10              */
    int total_score;       /* running total after this level            */
    int lives_done;        /* levels completed counter (= level_index+1)*/
    bool continue_to_game; /* OK should continue into special next level */
    bool full_run_message; /* Use the "all levels in one go" variant     */
} MenuOverlayData;

/* ── Menu selection state (main menu) ───────────────────────── */
typedef struct {
    int  selection;        /* 0=Continue,1=New Game,2=Options,3=Records,4=Help,5=Exit */
    bool has_save;         /* whether Continue is available                            */
    int  sub_screen;       /* 0=main list, 1=options, 2=records, 3=help               */
} MenuMainState;

typedef struct {
    int selection;         /* 0-based selected level */
    int top_index;         /* first visible row, max 9 visible */
    int level_count;       /* total entries in the list */
} MenuLevelSelectState;

/* ── Init/shutdown ──────────────────────────────────────────── */
void menu_init(SDL_Renderer* renderer);
void menu_shutdown(void);
void menu_render_startup_splash(SDL_Renderer* renderer, int phase);

/* ── Render functions (called each frame in their state) ─────── */
/* Returns new AppState so the caller can react:
   - menu_render_main          → APP_STATE_GAME (new game), APP_STATE_MENU (still open)
   - menu_render_level_complete→ APP_STATE_GAME (next level), APP_STATE_MENU (back)
   - menu_render_game_over     → APP_STATE_MENU (back)
   - menu_render_congratulations→APP_STATE_MENU (back)
   All take the Input to check confirm/cancel presses.                  */

#include "input.h"

AppState menu_update_main(MenuMainState* ms, const Input* inp);
void     menu_render_main(SDL_Renderer* renderer, const MenuMainState* ms);

AppState menu_update_level_select(MenuLevelSelectState* ls, const Input* inp);
void     menu_render_level_select(SDL_Renderer* renderer, const MenuLevelSelectState* ls);

/* Returns APP_STATE_GAME (continue to next level) or APP_STATE_MENU */
AppState menu_update_level_complete(const MenuOverlayData* d, const Input* inp);
void     menu_render_level_complete(SDL_Renderer* renderer, const MenuOverlayData* d);

/* Returns APP_STATE_MENU always */
AppState menu_update_game_over(const MenuOverlayData* d, const Input* inp);
void     menu_render_game_over(SDL_Renderer* renderer, const MenuOverlayData* d);

/* Returns APP_STATE_MENU always */
AppState menu_update_congratulations(const MenuOverlayData* d, const Input* inp);
void     menu_render_congratulations(SDL_Renderer* renderer, const MenuOverlayData* d);

#endif /* MENU_H */
