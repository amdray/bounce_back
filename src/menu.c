/**
 * menu.c — Texture-based main menu + modal overlay screens
 *
 * Main menu background from res/im container (J2ME class i.java):
 *   im[0] 176x208 — full-screen transition/splash  (Q)
 *   im[1] 145x26  — logo image                     (b)
 *   im[2] 176x21  — header bar                     (C)  drawImage(C, 0, 0)
 *   im[3] 28x187  — side column decoration         (M)  drawImage(M,0,y) + H-flip
 *   im[4] 16x16   — small arrow decoration         (E)
 *
 * Menu items match CrystalMidlet.java r[]:
 *   { "Continue", "New Game", "Options", "Records", "Help", "Exit" }
 *
 * Background: black letterbox outside the 176x208 active area; inside uses
 * texture-matched dark blue so the menu no longer appears cyan on PSP output.
 */

#include "menu.h"
#include "resource_loader.h"
#include "sound.h"
#include "hud_font.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

#include "../artifacts/second_splash_gradient.inc"

/* ── screen dimensions ───────────────────────────────────────── */
#define SCREEN_W 480
#define SCREEN_H 272

/* ── im[] indices (loading order from i.java c(boolean)) ──────── */
#define IM_SPLASH  0   /* 176x208  Q — transition/splash only, not in menu bg */
#define IM_BLOB    1   /* 145x26   b — splash logo only, not in menu bg       */
#define IM_HEADER  2   /* 176x21   C — drawImage(C, 0, 0)                     */
#define IM_COLUMN  3   /* 28x187   M — drawImage(M,0,i) + H-flip at (148,i)   */
#define IM_ARROW   4   /* 16x16    E — drawImage(E,160,21) + V-flip (160,172) */
#define IM_COUNT   5

/* ── active-area origin: keep X centered, pin menu Y to the top edge ─────── */
#define MENU_OX  ((SCREEN_W - 176) / 2)
#define MENU_OY  0
#define SPLASH_OY ((SCREEN_H - 208) / 2)

/* ── item layout from i.java: Y=30, step=18, centre x=88 in 176px ─────────  */
/*   drawString(z[i], 88, 30+i*18, TOP|HCENTER) → cx = MENU_OX+88 = 240      */
#define ITEM_COUNT 6
#define LEVEL_SELECT_VISIBLE_ROWS 12
/* selection bar: fillRect(18, k[i]-3, 139, 18)                               */
#define SEL_X_OFF  18
#define SEL_W      139
#define SEL_H      18

/* ── colours ─────────────────────────────────────────────────── */
/* Darker base under C/M/E to avoid cyan cast on PSP output */
#define COL_BG_R   0
#define COL_BG_G   57
#define COL_BG_B   119

/* panel overlay */
#define COL_OVERLAY_R 0
#define COL_OVERLAY_G 0
#define COL_OVERLAY_B 0
#define COL_OVERLAY_A 160

/* white panel card */
#define COL_PANEL_R 255
#define COL_PANEL_G 255
#define COL_PANEL_B 255
#define COL_PANEL_A 255

/* dark text */
#define COL_TEXT_R 30
#define COL_TEXT_G 30
#define COL_TEXT_B 30
#define COL_TEXT_A 255

/* accent red */
#define COL_ACCENT_R 220
#define COL_ACCENT_G 30
#define COL_ACCENT_B 30
#define COL_ACCENT_A 255

/* selection bar: J2ME color 187 = 0x0000BB */
#define COL_SEL_R   0
#define COL_SEL_G   0
#define COL_SEL_B   187
#define COL_SEL_A   210

/* ── font sizes ──────────────────────────────────────────────── */
#define FONT_TITLE  23
#define FONT_BODY   12
#define FONT_SMALL   9

/* ── menu items (CrystalMidlet.java r[]) ─────────────────────── */
static const char* k_items[6] = {
    "Continue", "New Game", "Options", "Records", "Help", "Exit"
};

/* sub_screen values */
#define SUB_NONE     0
#define SUB_OPTIONS  1
#define SUB_RECORDS  2
#define SUB_HELP     3

/* ── static module state ─────────────────────────────────────── */
static SDL_Texture*       s_tex[IM_COUNT];
static ResourceContainer* s_rc;

static int menu_first_enabled_selection(bool has_save) {
    return has_save ? 0 : 1;
}

static void menu_normalize_main_selection(MenuMainState* ms) {
    if (!ms) return;
    if (ms->selection < 0 || ms->selection >= ITEM_COUNT) {
        ms->selection = menu_first_enabled_selection(ms->has_save);
        return;
    }
    if (ms->selection == 0 && !ms->has_save) {
        ms->selection = menu_first_enabled_selection(ms->has_save);
    }
}

static void draw_second_splash_gradient(SDL_Renderer* renderer) {
    /* Central 208 rows are exact 1:1 from source; outside rows use edge colors. */
    const int y0 = SPLASH_OY;
    const int y1 = SPLASH_OY + 208;

    SDL_SetRenderDrawColor(renderer,
                           k_second_splash_grad_208[0][0],
                           k_second_splash_grad_208[0][1],
                           k_second_splash_grad_208[0][2],
                           255);
    if (y0 > 0) {
        SDL_RenderFillRect(renderer, &(SDL_Rect){ 0, 0, SCREEN_W, y0 });
    }

    for (int sy = 0; sy < 208; sy++) {
        int y = y0 + sy;
        SDL_SetRenderDrawColor(renderer,
                               k_second_splash_grad_208[sy][0],
                               k_second_splash_grad_208[sy][1],
                               k_second_splash_grad_208[sy][2],
                               255);
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_W - 1, y);
    }

    SDL_SetRenderDrawColor(renderer,
                           k_second_splash_grad_208[207][0],
                           k_second_splash_grad_208[207][1],
                           k_second_splash_grad_208[207][2],
                           255);
    if (y1 < SCREEN_H) {
        SDL_RenderFillRect(renderer, &(SDL_Rect){ 0, y1, SCREEN_W, SCREEN_H - y1 });
    }
}

/* ── colour helpers ──────────────────────────────────────────── */
static SDL_Color s_col_text   = { COL_TEXT_R,   COL_TEXT_G,   COL_TEXT_B,   COL_TEXT_A };
static SDL_Color s_col_accent = { COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B, COL_ACCENT_A };
static SDL_Color s_col_white  = { 255, 255, 255, 255 };
static SDL_Color s_col_hint   = { 255, 255, 255, 255 };  /* original menu text is white */
static SDL_Color s_col_gray   = { 100, 120, 150, 255 };  /* disabled */

static SDL_Color menu_pulse_color(void) {
    int phase = (int)((SDL_GetTicks() / 50u) % 19u);
    switch (phase) {
        case 0:
        case 1:
        case 17:
        case 18:
            return (SDL_Color){ 0xFF, 0xDD, 0x00, 0xFF };
        case 2:
        case 3:
        case 15:
        case 16:
            return (SDL_Color){ 0xFF, 0xCC, 0x00, 0xFF };
        case 4:
        case 5:
        case 13:
        case 14:
            return (SDL_Color){ 0xFF, 0xBB, 0x00, 0xFF };
        case 6:
        case 7:
        case 11:
        case 12:
            return (SDL_Color){ 0xFF, 0x99, 0x00, 0xFF };
        default:
            return (SDL_Color){ 0xFF, 0x88, 0x00, 0xFF };
    }
}

static void draw_selected_item_frame(SDL_Renderer* renderer, int y) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, COL_SEL_R, COL_SEL_G, COL_SEL_B, COL_SEL_A);
    SDL_RenderFillRect(renderer, &(SDL_Rect){
        MENU_OX + SEL_X_OFF, y - 3, SEL_W, SEL_H
    });
    SDL_SetRenderDrawColor(renderer, 102, 102, 102, 200);
    SDL_RenderDrawRect(renderer, &(SDL_Rect){
        MENU_OX + SEL_X_OFF, y - 3, SEL_W, SEL_H
    });
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

/* Draw a centred string, returns y-bottom */
static int draw_centered(SDL_Renderer* r, int cx, int y, const char* text,
                         SDL_Color col, int font) {
    int w = hud_font_measure_text(text, font);
    hud_font_draw_text(r, cx - w / 2, y, text, col, font);
    return y + font + 2;
}

/* Modal panel used by overlay screens (level complete, game over, …) */
static void draw_panel(SDL_Renderer* r, int* px, int* py, int* pw, int* ph) {
    const int W = 260, H = 150;
    *px = (SCREEN_W - W) / 2;
    *py = (SCREEN_H - H) / 2;
    *pw = W;
    *ph = H;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, COL_OVERLAY_R, COL_OVERLAY_G, COL_OVERLAY_B, COL_OVERLAY_A);
    SDL_RenderFillRect(r, &(SDL_Rect){ 0, 0, SCREEN_W, SCREEN_H });
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_SetRenderDrawColor(r, COL_PANEL_R, COL_PANEL_G, COL_PANEL_B, COL_PANEL_A);
    SDL_RenderFillRect(r, &(SDL_Rect){ *px, *py, *pw, *ph });

    SDL_SetRenderDrawColor(r, COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B, 255);
    SDL_RenderDrawRect(r, &(SDL_Rect){ *px, *py, *pw, *ph });
}

static void draw_hint(SDL_Renderer* r, int panel_x, int panel_y, int panel_h,
                      const char* text) {
    int y = panel_y + panel_h - FONT_SMALL - 6;
    int cx = panel_x + 260 / 2;
    draw_centered(r, cx, y, text, s_col_text, FONT_SMALL);
}

/* ═══════════════════════════════════════════════════════════════
   INIT / SHUTDOWN
   Load all 5 PNG images from res/im container via resource_loader.
   ═══════════════════════════════════════════════════════════════ */

void menu_init(SDL_Renderer* renderer) {
    s_rc = resource_load("res/im");
    if (!s_rc) {
        fprintf(stderr, "menu_init: failed to load res/im\n");
        return;
    }

    int load_count = (int)s_rc->count < IM_COUNT ? (int)s_rc->count : IM_COUNT;
    for (int i = 0; i < load_count; i++) {
        size_t sz = 0;
        const uint8_t* data = resource_get_element(s_rc, i, &sz);
        if (!data || sz == 0) continue;

        SDL_RWops* rw = SDL_RWFromConstMem(data, (int)sz);
        if (!rw) {
            fprintf(stderr, "menu_init: SDL_RWFromConstMem failed for im[%d]\n", i);
            continue;
        }
        s_tex[i] = IMG_LoadTexture_RW(renderer, rw, 1); /* rw freed by SDL */
        if (!s_tex[i]) {
            fprintf(stderr, "menu_init: IMG_LoadTexture_RW failed im[%d]: %s\n",
                    i, IMG_GetError());
        }
    }
}

void menu_shutdown(void) {
    for (int i = 0; i < IM_COUNT; i++) {
        if (s_tex[i]) { SDL_DestroyTexture(s_tex[i]); s_tex[i] = NULL; }
    }
    resource_free(s_rc);
    s_rc = NULL;
}

void menu_render_startup_splash(SDL_Renderer* renderer, int phase) {
    if (!renderer) return;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (phase <= 0) {
        if (s_tex[IM_BLOB]) {
            int w = 0;
            int h = 0;
            SDL_QueryTexture(s_tex[IM_BLOB], NULL, NULL, &w, &h);
            SDL_Rect dst = {
                MENU_OX + 88 - w / 2,
                SPLASH_OY + 104 - h / 2,
                w,
                h
            };
            SDL_RenderCopy(renderer, s_tex[IM_BLOB], NULL, &dst);
        }
        return;
    }

    draw_second_splash_gradient(renderer);

    if (s_tex[IM_SPLASH]) {
        SDL_Rect dst = { MENU_OX, SPLASH_OY, 176, 208 };
        SDL_RenderCopy(renderer, s_tex[IM_SPLASH], NULL, &dst);
    }
}

/* ═══════════════════════════════════════════════════════════════
   MAIN MENU — background
   i.java b(Graphics) — pixel-perfect 1:1, no scaling:
     fillRect(0,0,176,208) #0077EE → full PSP screen fill
     drawImage(C, 0, 0)            → im[2] 176×21 at (MENU_OX, MENU_OY)
     drawImage(M, 0, i)            → im[3] 28×187 at (MENU_OX, MENU_OY+21)
     drawImage(M, 148, i, HFLIP)   → im[3] 28×187 at (MENU_OX+148, MENU_OY+21)
     drawImage(E, 160, 21)         → im[4] 16×16  at (MENU_OX+160, MENU_OY+21)
     drawImage(E, 160, 172, VFLIP) → im[4] 16×16  at (MENU_OX+160, MENU_OY+172)
   im[0] (Q) and im[1] (b) are NOT part of the normal menu background.
   ═══════════════════════════════════════════════════════════════ */

static void draw_menu_bg(SDL_Renderer* r) {
    /* black full screen base */
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderFillRect(r, &(SDL_Rect){ 0, 0, SCREEN_W, SCREEN_H });

    /* Extend the menu background color to the bottom of the screen. */
    SDL_SetRenderDrawColor(r, 0, 114, 238, 255);
    SDL_RenderFillRect(r, &(SDL_Rect){ 0, MENU_OY, SCREEN_W, SCREEN_H - MENU_OY });

    /* im[2] = C header bar (176x21): split into halves and bridge them with a
       1px center slice stretched across the middle, preserving the white bottom line. */
    if (s_tex[IM_HEADER]) {
        SDL_Rect src_l = { 0, 0, 88, 21 };
        SDL_Rect dst_l = { 0, MENU_OY, 88, 21 };
        SDL_RenderCopy(r, s_tex[IM_HEADER], &src_l, &dst_l);

        if (SCREEN_W > 176) {
            SDL_Rect src_mid = { 88, 0, 1, 21 };
            SDL_Rect dst_mid = { 88, MENU_OY, SCREEN_W - 176, 21 };
            SDL_RenderCopy(r, s_tex[IM_HEADER], &src_mid, &dst_mid);
        }

        SDL_Rect src_r = { 88, 0, 88, 21 };
        SDL_Rect dst_r = { SCREEN_W - 88, MENU_OY, 88, 21 };
        SDL_RenderCopy(r, s_tex[IM_HEADER], &src_r, &dst_r);
    }

    /* im[3] = M (28×187): left column at screen left edge, right H-flipped at screen right edge */
    if (s_tex[IM_COLUMN]) {
        SDL_Rect dst_l = { 0,            MENU_OY + 21, 28, 187 };
        SDL_Rect dst_r = { SCREEN_W - 28, MENU_OY + 21, 28, 187 };
        SDL_RenderCopy(r, s_tex[IM_COLUMN], NULL, &dst_l);
        SDL_RenderCopyEx(r, s_tex[IM_COLUMN], NULL, &dst_r,
                         0.0, NULL, SDL_FLIP_HORIZONTAL);
    }

}

static void draw_scroll_arrows(SDL_Renderer* r) {
    if (!r || !s_tex[IM_ARROW]) return;

    SDL_Rect dst_t = { MENU_OX + 160, MENU_OY + 21, 16, 16 };
    SDL_Rect dst_b = { MENU_OX + 160, MENU_OY + 172, 16, 16 };
    SDL_RenderCopy(r, s_tex[IM_ARROW], NULL, &dst_t);
    SDL_RenderCopyEx(r, s_tex[IM_ARROW], NULL, &dst_b,
                     0.0, NULL, SDL_FLIP_VERTICAL);
}

static int menu_startup_text_offset_for_tick(int tick) {
    int offset = -150;
    int h = 0;

    if (tick > 22) tick = 22;
    if (tick < 0) tick = 0;

    for (int param = 22; param > tick; param--) {
        if (param == 12 || param == 22) {
            h = 0;
        }
        if (param > 12) {
            offset += 11 + h;
            if (((22 - param) % 2) == 1) h += 2;
        } else if (param > 6) {
            offset -= 3 - h;
            if ((12 - param) == 0 || (12 - param) == 2) h++;
        } else {
            offset += 3 - h;
            if ((6 - param) == 2 || (6 - param) == 4) h--;
        }
    }

    return offset;
}

void menu_render_startup_menu_intro(SDL_Renderer* renderer,
                                    const MenuMainState* ms,
                                    int tick_remaining) {
    menu_render_main_with_intro(renderer, ms, menu_startup_text_offset_for_tick(tick_remaining));
}

/* ═══════════════════════════════════════════════════════════════
   MAIN MENU — update
   6 items: Continue(0), New Game(1), Options(2), Records(3), Help(4), Exit(5)
   sub_screen ≠ 0  →  Options / Records / Help stub panel
   ═══════════════════════════════════════════════════════════════ */

AppState menu_update_main(MenuMainState* ms, const Input* inp) {
    if (!ms || !inp) return APP_STATE_MENU;
    menu_normalize_main_selection(ms);

    /* ── sub-screen: O = back ────────────────────────────────── */
    if (ms->sub_screen != SUB_NONE) {
        if (inp->cancel_pressed) {
            sound_play(SND_MENU_BACK);
            ms->sub_screen = SUB_NONE;
        }
        return APP_STATE_MENU;
    }

    /* ── main list navigation ────────────────────────────────── */
    if (inp->up_pressed) {
        ms->selection = (ms->selection - 1 + ITEM_COUNT) % ITEM_COUNT;
        if (ms->selection == 0 && !ms->has_save)
            ms->selection = ITEM_COUNT - 1;
    }
    if (inp->down_pressed) {
        ms->selection = (ms->selection + 1) % ITEM_COUNT;
        if (ms->selection == 0 && !ms->has_save)
            ms->selection = 1;
    }

    if (inp->confirm_pressed) {
        sound_play(SND_MENU_SELECT);
        switch (ms->selection) {
            case 0: /* Continue */
                if (ms->has_save) {
                    return APP_STATE_GAME;
                }
                break;
            case 1: /* New Game */
                return APP_STATE_LEVEL_SELECT;
            case 2: ms->sub_screen = SUB_OPTIONS; break;
            case 3: ms->sub_screen = SUB_RECORDS; break;
            case 4: ms->sub_screen = SUB_HELP;    break;
            case 5: return APP_STATE_QUIT;
        }
    }

    return APP_STATE_MENU;
}

/* ═══════════════════════════════════════════════════════════════
   MAIN MENU — render
   ═══════════════════════════════════════════════════════════════ */

void menu_render_main_with_intro(SDL_Renderer* renderer,
                                 const MenuMainState* ms,
                                 int text_y_offset) {
    MenuMainState normalized;

    if (!renderer || !ms) return;
    normalized = *ms;
    menu_normalize_main_selection(&normalized);

    draw_menu_bg(renderer);

    /* ── sub-screen overlay (Options / Records / Help) ──────── */
    if (normalized.sub_screen != SUB_NONE) {
        const char* titles[] = { "", "Options", "Records", "Help" };
        int sub = normalized.sub_screen;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, &(SDL_Rect){ 0, 0, SCREEN_W, SCREEN_H });
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        const int pw = 300, ph = 130;
        int px = (SCREEN_W - pw) / 2;
        int py = (SCREEN_H - ph) / 2;
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){ px, py, pw, ph });
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &(SDL_Rect){ px, py, pw, ph });

        int cx = px + pw / 2;
        int y  = py + 14;
        draw_centered(renderer, cx, y, titles[sub], s_col_white, FONT_TITLE);
        y += FONT_TITLE + 14;

        if (sub == SUB_HELP) {
            draw_centered(renderer, cx, y, "Left / Right  =  Move",   s_col_hint, FONT_BODY);
            y += FONT_BODY + 5;
            draw_centered(renderer, cx, y, "Cross (X)     =  Jump",   s_col_hint, FONT_BODY);
            y += FONT_BODY + 5;
            draw_centered(renderer, cx, y, "Start         =  Pause",  s_col_hint, FONT_BODY);
        } else {
            const char* bodies[] = { "", "Coming soon.", "No records yet.", "" };
            draw_centered(renderer, cx, y, bodies[sub], s_col_hint, FONT_BODY);
        }

        draw_centered(renderer, cx, py + ph - FONT_SMALL - 6,
                      "O = Back", s_col_hint, FONT_SMALL);
        return;
    }

    /* ── item list ───────────────────────────────────────────── */
    /* i.java: centre x=88 in 176px → cx = MENU_OX+88 = 240 */
    /* i.java: y[i] = Y + i*18 = 30 + i*18 → MENU_OY+30+i*18 */
    const int cx = MENU_OX + 88;

    for (int i = 0; i < ITEM_COUNT; i++) {
        int iy = MENU_OY + 30 + i * 18 + text_y_offset;
        bool disabled = (i == 0 && !normalized.has_save);

        if (i == normalized.selection && !disabled) {
            draw_selected_item_frame(renderer, iy);
        }

        SDL_Color col;
        if      (disabled)           col = s_col_gray;
        else if (i == normalized.selection) col = menu_pulse_color();
        else                         col = s_col_hint;

        draw_centered(renderer, cx, iy, k_items[i], col, FONT_BODY);
    }

    /* ── bottom hint ─────────────────────────────────────────── */
    draw_centered(renderer, cx, SCREEN_H - FONT_SMALL - 4 + text_y_offset,
                  "Up/Down = Navigate    X = Select",
                  s_col_hint, FONT_SMALL);
}

void menu_render_main(SDL_Renderer* renderer, const MenuMainState* ms) {
    menu_render_main_with_intro(renderer, ms, 0);
}

AppState menu_update_level_select(MenuLevelSelectState* ls, const Input* inp) {
    if (!ls || ls->level_count <= 0) return APP_STATE_MENU;

    if (inp->cancel_pressed) {
        sound_play(SND_MENU_BACK);
        return APP_STATE_MENU;
    }

    if (inp->up_pressed) {
        ls->selection--;
        if (ls->selection < 0) ls->selection = ls->level_count - 1;
    }
    if (inp->down_pressed) {
        ls->selection++;
        if (ls->selection >= ls->level_count) ls->selection = 0;
    }

    if (ls->selection < ls->top_index) {
        ls->top_index = ls->selection;
    } else if (ls->selection >= ls->top_index + LEVEL_SELECT_VISIBLE_ROWS) {
        ls->top_index = ls->selection - (LEVEL_SELECT_VISIBLE_ROWS - 1);
    }

    if (ls->top_index < 0) ls->top_index = 0;
    if (ls->top_index > ls->level_count - LEVEL_SELECT_VISIBLE_ROWS) {
        ls->top_index = (ls->level_count > LEVEL_SELECT_VISIBLE_ROWS)
                      ? (ls->level_count - LEVEL_SELECT_VISIBLE_ROWS)
                      : 0;
    }

    if (inp->confirm_pressed) {
        sound_play(SND_MENU_SELECT);
        return APP_STATE_GAME;
    }
    return APP_STATE_LEVEL_SELECT;
}

void menu_render_level_select(SDL_Renderer* renderer, const MenuLevelSelectState* ls) {
    if (!renderer || !ls || ls->level_count <= 0) return;

    draw_menu_bg(renderer);
    if (ls->level_count > LEVEL_SELECT_VISIBLE_ROWS) {
        draw_scroll_arrows(renderer);
    }

    draw_centered(renderer, MENU_OX + 88, MENU_OY + 2, "New Game", s_col_white, FONT_BODY);

    for (int row = 0; row < LEVEL_SELECT_VISIBLE_ROWS; row++) {
        int index = ls->top_index + row;
        if (index >= ls->level_count) break;

        int iy = MENU_OY + 30 + row * 18;
        char label[32];
        snprintf(label, sizeof(label), "Level %d", index + 1);

        if (index == ls->selection) {
            draw_selected_item_frame(renderer, iy);
            draw_centered(renderer, MENU_OX + 88, iy, label, menu_pulse_color(), FONT_BODY);
        } else {
            draw_centered(renderer, MENU_OX + 88, iy, label, s_col_hint, FONT_BODY);
        }
    }

    draw_centered(renderer, MENU_OX + 88, SCREEN_H - FONT_SMALL - 4,
                  "Up/Down = Navigate    X = Select    O = Back",
                  s_col_hint, FONT_SMALL);
}

/* ═══════════════════════════════════════════════════════════════
   LEVEL COMPLETE
   Java CrystalMidlet.c():
     "Points: {0}" = player score
     "Bonus: {0}"  = time_bonus + level_points/10
     "Total: {0}"  = total_score
   ═══════════════════════════════════════════════════════════════ */

AppState menu_update_level_complete(const MenuOverlayData* d, const Input* inp) {
    (void)d;
    if (inp->confirm_pressed) {
        sound_play(SND_MENU_SELECT);
        return APP_STATE_GAME; /* proceed to next level */
    }
    return APP_STATE_LEVEL_COMPLETE;
}

void menu_render_level_complete(SDL_Renderer* renderer, const MenuOverlayData* d) {
    int px, py, pw, ph;
    draw_panel(renderer, &px, &py, &pw, &ph);

    int cx = px + pw / 2;
    int y = py + 8;

    /* Title */
    y = draw_centered(renderer, cx, y, "Level complete!", s_col_accent, FONT_TITLE);
    y += 4;

    /* "Points: NNN" */
    char buf[64];
    snprintf(buf, sizeof(buf), "Points: %d", d->level_points);
    y = draw_centered(renderer, cx, y, buf, s_col_text, FONT_BODY);
    y += 2;

    /* "Bonus: NNN" */
    snprintf(buf, sizeof(buf), "Bonus: %d", d->stage_bonus);
    y = draw_centered(renderer, cx, y, buf, s_col_text, FONT_BODY);
    y += 2;

    /* "Total: NNN" — accented */
    snprintf(buf, sizeof(buf), "Total: %d", d->total_score);
    y = draw_centered(renderer, cx, y, buf, s_col_accent, FONT_BODY);

    draw_hint(renderer, px, py, ph, "A = OK");
}

/* ═══════════════════════════════════════════════════════════════
   GAME OVER
   Java CrystalMidlet.f():
     "Levels Completed: {0}"
     "Score: {0}"
   ═══════════════════════════════════════════════════════════════ */

AppState menu_update_game_over(const MenuOverlayData* d, const Input* inp) {
    (void)d;
    if (inp->confirm_pressed) {
        sound_play(SND_MENU_SELECT);
        return APP_STATE_MENU;
    }
    return APP_STATE_GAME_OVER;
}

void menu_render_game_over(SDL_Renderer* renderer, const MenuOverlayData* d) {
    int px, py, pw, ph;
    draw_panel(renderer, &px, &py, &pw, &ph);

    int cx = px + pw / 2;
    int y = py + 8;

    y = draw_centered(renderer, cx, y, "Game Over!", s_col_accent, FONT_TITLE);
    y += 8;

    char buf[64];
    snprintf(buf, sizeof(buf), "Levels Completed: %d", d->lives_done);
    y = draw_centered(renderer, cx, y, buf, s_col_text, FONT_BODY);
    y += 2;

    snprintf(buf, sizeof(buf), "Score: %d", d->total_score);
    draw_centered(renderer, cx, y, buf, s_col_accent, FONT_BODY);

    draw_hint(renderer, px, py, ph, "A = OK");
}

/* ═══════════════════════════════════════════════════════════════
   CONGRATULATIONS
   Java CrystalMidlet string d[]:
     "Congratulations! You have completed the last level of Bounce Back!"
   ═══════════════════════════════════════════════════════════════ */

AppState menu_update_congratulations(const MenuOverlayData* d, const Input* inp) {
    (void)d;
    if (inp->confirm_pressed) {
        sound_play(SND_MENU_SELECT);
        if (d && d->continue_to_game) {
            return APP_STATE_GAME;
        }
        return APP_STATE_MENU;
    }
    return APP_STATE_CONGRATULATIONS;
}

void menu_render_congratulations(SDL_Renderer* renderer, const MenuOverlayData* d) {
    int px, py, pw, ph;
    draw_panel(renderer, &px, &py, &pw, &ph);

    int cx = px + pw / 2;
    int y = py + 12;

    y = draw_centered(renderer, cx, y, "Congratulations!", s_col_accent, FONT_TITLE);
    y += 6;

    if (d && d->continue_to_game) {
        if (d->full_run_message) {
            draw_centered(renderer, cx, y, "You have completed", s_col_text, FONT_BODY);
            y += FONT_BODY + 2;
            draw_centered(renderer, cx, y, "all the levels of", s_col_text, FONT_BODY);
            y += FONT_BODY + 2;
            draw_centered(renderer, cx, y, "Bounce Back in one go!", s_col_text, FONT_BODY);
            y += FONT_BODY + 4;
        } else {
            draw_centered(renderer, cx, y, "You have completed", s_col_text, FONT_BODY);
            y += FONT_BODY + 2;
            draw_centered(renderer, cx, y, "the last level of", s_col_text, FONT_BODY);
            y += FONT_BODY + 2;
            draw_centered(renderer, cx, y, "Bounce Back!", s_col_text, FONT_BODY);
            y += FONT_BODY + 4;
        }
    } else {
        /* long string — split manually to fit 260px panel */
        draw_centered(renderer, cx, y,      "You have completed", s_col_text, FONT_BODY);
        y += FONT_BODY + 2;
        draw_centered(renderer, cx, y,      "the last level of", s_col_text, FONT_BODY);
        y += FONT_BODY + 2;
        draw_centered(renderer, cx, y,      "Bounce Back!", s_col_text, FONT_BODY);
        y += FONT_BODY + 4;
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "Final Score: %d", d->total_score);
    draw_centered(renderer, cx, y, buf, s_col_accent, FONT_BODY);

    draw_hint(renderer, px, py, ph, d && d->continue_to_game ? "A = Next" : "A = OK");
}
