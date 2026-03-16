#include "hud.h"

#include "hud_font.h"
#include "game_constants.h"

#include <stdio.h>
#define SCORE_DIGITS  8

#define HUD_COLOUR_R 0x29
#define HUD_COLOUR_G 0x31
#define HUD_COLOUR_B 0x94

#define COLOR_WHITE_R 0xFF
#define COLOR_WHITE_G 0xFF
#define COLOR_WHITE_B 0xFF

#define COLOR_BONUS_BAR_R 0xFF
#define COLOR_BONUS_BAR_G 0xD3
#define COLOR_BONUS_BAR_B 0x31

/* Textures borrowed from IcAssets — NOT owned by hud. */
static SDL_Texture* g_hud_lives_strip = NULL; /* ic[1] — lives strip  */
static SDL_Texture* g_hud_ring_icon   = NULL; /* ic[2] — ring icon    */
static int g_hud_lives_w = 0;
static int g_hud_lives_h = 0;
static int g_hud_ring_w = 0;
static int g_hud_ring_h = 0;

static void format_score_string(int score, char* buffer) {
    if (score < 0) score = 0;
    if (score > 99999999) score = 99999999;
    snprintf(buffer, SCORE_DIGITS + 1, "%0*d", SCORE_DIGITS, score);
}

static int normalize_bonus_bar_units(int bonus_counter) {
    if (bonus_counter <= 0) return 0;
    bonus_counter /= BONUS_BAR_TICK_DIVISOR;
    if (bonus_counter > BONUS_BAR_MAX_UNITS) bonus_counter = BONUS_BAR_MAX_UNITS;
    return bonus_counter;
}

// 1:1 from bounce_zero/src/game.c (with SDL rect primitives)
static void draw_bonus_bar(SDL_Renderer* renderer, int x, int y, int bonus_value) {
    const int frame_width = 62;
    const int frame_height = 10;
    const int bar_width = 60;
    const int bar_height = 8;

    SDL_SetRenderDrawColor(renderer, COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, 255);
    SDL_RenderDrawLine(renderer, x, y, x + frame_width - 1, y);
    SDL_RenderDrawLine(renderer, x, y + frame_height - 1, x + frame_width - 1, y + frame_height - 1);
    SDL_RenderDrawLine(renderer, x, y, x, y + frame_height - 1);
    SDL_RenderDrawLine(renderer, x + frame_width - 1, y, x + frame_width - 1, y + frame_height - 1);

    if (bonus_value > 0) {
        if (bonus_value > BONUS_BAR_MAX_UNITS) bonus_value = BONUS_BAR_MAX_UNITS;
        int current_bar_width = (bar_width * bonus_value) / BONUS_BAR_MAX_UNITS;
        if (current_bar_width > 0) {
            SDL_Rect bar = { x + 1, y + 1, current_bar_width, bar_height };
            SDL_SetRenderDrawColor(renderer, COLOR_BONUS_BAR_R, COLOR_BONUS_BAR_G, COLOR_BONUS_BAR_B, 255);
            SDL_RenderFillRect(renderer, &bar);
        }
    }
}

int hud_init(SDL_Renderer* renderer, const IcAssets* ic) {
    if (hud_font_init(renderer) != 0) return -1;

    if (!ic) return 0;

    /* Borrow pre-decoded textures — hud does not own them. */
    g_hud_lives_strip = ic->lives_strip;
    g_hud_ring_icon   = ic->ring_icon;

    if (g_hud_lives_strip)
        SDL_QueryTexture(g_hud_lives_strip, NULL, NULL, &g_hud_lives_w, &g_hud_lives_h);
    if (g_hud_ring_icon)
        SDL_QueryTexture(g_hud_ring_icon,   NULL, NULL, &g_hud_ring_w,  &g_hud_ring_h);

    return 0;
}

void hud_shutdown(void) {
    /* Textures are owned by GameAssets — only null out the borrowed pointers. */
    g_hud_ring_icon   = NULL;
    g_hud_ring_w      = 0;
    g_hud_ring_h      = 0;
    g_hud_lives_strip = NULL;
    g_hud_lives_w     = 0;
    g_hud_lives_h     = 0;
    hud_font_shutdown();
}

void hud_render(SDL_Renderer* renderer, const Tileset* tileset, SDL_Texture* life_ball_icon_tex, const HudState* state) {
    if (!renderer || !state) return;

    const int hudStartY = SCREEN_HEIGHT - HUD_HEIGHT;
    int separator_y = hudStartY;
    int hud_blue_y = separator_y + 1;

    SDL_SetRenderDrawColor(renderer, COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, 255);
    SDL_RenderDrawLine(renderer, 0, separator_y, SCREEN_WIDTH - 1, separator_y);

    SDL_Rect blue = { 0, hud_blue_y, SCREEN_WIDTH, HUD_HEIGHT - 1 };
    SDL_SetRenderDrawColor(renderer, HUD_COLOUR_R, HUD_COLOUR_G, HUD_COLOUR_B, 255);
    SDL_RenderFillRect(renderer, &blue);

    int max_bonus = 0;
    int speed_bonus = normalize_bonus_bar_units(state->speed_bonus_counter);
    int grav_bonus = normalize_bonus_bar_units(state->grav_bonus_counter);
    int jump_bonus = normalize_bonus_bar_units(state->jump_bonus_counter);
    int stone_bonus = normalize_bonus_bar_units(state->stone_bonus_counter);
    if (speed_bonus > max_bonus) max_bonus = speed_bonus;
    if (grav_bonus > max_bonus) max_bonus = grav_bonus;
    if (jump_bonus > max_bonus) max_bonus = jump_bonus;
    if (stone_bonus > max_bonus) max_bonus = stone_bonus;

    if (tileset) {
        int remainingRings = state->total_rings - state->num_rings;
        if (remainingRings < 0) remainingRings = 0;

        if (g_hud_ring_icon && g_hud_ring_w > 0 && g_hud_ring_h > 0) {
            // Use original ic_2 ring icon, but without overlap.
            for (int i = 0; i < remainingRings; i++) {
                int x = 12 + i * g_hud_ring_w;
                int y = hudStartY + 3;
                SDL_Rect dst = { x, y, g_hud_ring_w, g_hud_ring_h };
                SDL_RenderCopy(renderer, g_hud_ring_icon, NULL, &dst);
            }
        }

    }

    if (g_hud_lives_strip && g_hud_lives_w > 0 && g_hud_lives_h >= 16) {
        int frames = g_hud_lives_h / 16;
        int lives = state->num_lives;
        if (lives < 0) lives = 0;
        if (frames > 0 && lives >= frames) lives = frames - 1;

        int digit_x = SCREEN_WIDTH - 5 - g_hud_lives_w;
        int digit_y = hudStartY + 2;

        if (life_ball_icon_tex) {
            SDL_Rect icon_dst = { digit_x - 18, digit_y + 1, 16, 16 };
            SDL_RenderCopy(renderer, life_ball_icon_tex, NULL, &icon_dst);
        }

        SDL_Rect src = { 0, lives * 16, g_hud_lives_w, 16 };
        SDL_Rect dst = { digit_x, digit_y, g_hud_lives_w, 16 };
        SDL_RenderCopy(renderer, g_hud_lives_strip, &src, &dst);
    }

    char score_buffer[SCORE_DIGITS + 1];
    format_score_string(state->score, score_buffer);

    SDL_Color white = { COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, 255 };
    int score_width = hud_font_measure_text(score_buffer, 9);
    int score_x = (SCREEN_WIDTH - score_width) / 2;
    int score_y = hudStartY + 5;
    hud_font_draw_text(renderer, score_x, score_y, score_buffer, white, 9);

    {
        int text_width = hud_font_measure_text(score_buffer, 9);
        int bonus_x = score_x + text_width + 10 + 30;
        int bonus_y = hudStartY + 4;
        draw_bonus_bar(renderer, bonus_x, bonus_y, max_bonus);
    }
}
