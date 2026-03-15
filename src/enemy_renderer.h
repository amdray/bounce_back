#ifndef ENEMY_RENDERER_H
#define ENEMY_RENDERER_H

#include <SDL2/SDL.h>

#include "level_loader.h"

int enemy_renderer_init(SDL_Renderer* renderer);
void enemy_renderer_shutdown(void);
void enemy_renderer_draw(SDL_Renderer* renderer, const Level* level, int camera_x, int camera_y);

#endif // ENEMY_RENDERER_H
