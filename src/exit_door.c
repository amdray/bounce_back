/**
 * Exit door implementation (a.java:607-612, h.java:664-672) - Step 08.
 */

#include "exit_door.h"

#include "level_loader.h"
#include "player.h"
#include "game_constants.h"

/* ic[7] is 32x96: two halves of 32x24 each, closed and open states */
#define DOOR_TEX_W   32
#define DOOR_HALF_H  24

/* g_door_tex borrowed from IcAssets — NOT owned by exit_door. */
static SDL_Texture* g_door_tex = NULL;

void exit_door_tick(ExitDoorState* door, int objective_remaining) {
    if (!door) return;

    if (!door->open && objective_remaining == 0) {
        door->I++;
        if (door->I == 48) {
            door->open = true;
        }
        return;
    }

    if (door->open) {
        door->I++;
        if (door->I == 72) {
            door->I = 48;
        }
    }
}

bool exit_door_test_complete(ExitDoorState* door, Level* level, Player* p) {
    if (!door || !level || !p) return false;
    if (!door->open) return false;

    int door_px_x = (int)level->exit_x * 16;
    int door_px_y = (int)level->exit_y * 16;

    return (door_px_x < p->x_pos && p->x_pos < door_px_x + 32 &&
            door_px_y < p->y_pos && p->y_pos < door_px_y + 48);
}

/* ── Renderer (h.java::c(Graphics)) ─────────────────────────────────────── */

int exit_door_renderer_init(const IcAssets* ic) {
    if (!ic || !ic->door) return -1;
    g_door_tex = ic->door; /* borrowed, not owned */
    return 0;
}

void exit_door_renderer_shutdown(void) {
    /* Texture owned by GameAssets — only null out the borrowed pointer. */
    g_door_tex = NULL;
}

/*
 * h.java::c(Graphics) — clip-based two-half animation.
 *
 * Texture layout (ic[7], 32×96):
 *   rows   0-23: upper panel, closed state
 *   rows  24-47: upper panel, open state
 *   rows  48-71: lower panel, open state
 *   rows  72-95: lower panel, closed state
 *
 * I cycles 0→48 (opening), then 48→72→48 (cycling open).
 * i6  = I >> 1  (0..24 while opening, 24..36 while cycling)
 *
 * Upper half: src row = i6         →  slides from row 0 (closed) to row 24 (open)
 * Lower half: src row = 72 - i6    →  slides from row 72 (closed) to row 48 (open)
 */
void exit_door_render(SDL_Renderer* renderer, const ExitDoorState* door,
                      const Level* level, int camera_x, int camera_y) {
    if (!renderer || !door || !level || !g_door_tex) return;

    int screen_x = (int)level->exit_x * 16 - camera_x;
    int screen_y = (int)level->exit_y * 16 - camera_y;

    /* visibility cull — door occupies 32×48 on screen */
    if (screen_x + DOOR_TEX_W <= 0 || screen_x >= SCREEN_WIDTH ||
        screen_y + DOOR_HALF_H * 2 <= 0 || screen_y >= SCREEN_HEIGHT) {
        return;
    }

    int i6 = door->I >> 1;

    /* upper half */
    SDL_Rect src_top = { 0, i6,        DOOR_TEX_W, DOOR_HALF_H };
    SDL_Rect dst_top = { screen_x, screen_y,               DOOR_TEX_W, DOOR_HALF_H };
    SDL_RenderCopy(renderer, g_door_tex, &src_top, &dst_top);

    /* lower half */
    SDL_Rect src_bot = { 0, 72 - i6,   DOOR_TEX_W, DOOR_HALF_H };
    SDL_Rect dst_bot = { screen_x, screen_y + DOOR_HALF_H, DOOR_TEX_W, DOOR_HALF_H };
    SDL_RenderCopy(renderer, g_door_tex, &src_bot, &dst_bot);
}
