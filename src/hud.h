#ifndef HUD_H
#define HUD_H

#include <SDL2/SDL.h>

#include "tileset_loader.h"

typedef struct {
    int score;
    int num_lives;
    int total_rings;
    int num_rings;
    int speed_bonus_counter;
    int grav_bonus_counter;
    int jump_bonus_counter;
} HudState;

int hud_init(SDL_Renderer* renderer);
void hud_shutdown(void);
void hud_render(SDL_Renderer* renderer, const Tileset* tileset, SDL_Texture* life_ball_icon_tex, const HudState* state);

#endif // HUD_H
