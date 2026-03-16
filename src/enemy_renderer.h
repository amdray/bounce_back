#ifndef ENEMY_RENDERER_H
#define ENEMY_RENDERER_H

#include <SDL2/SDL.h>

#include "game_assets.h"
#include "level_loader.h"

int object_renderer_init(const IcAssets* ic);
void object_renderer_shutdown(void);
void object_renderer_draw(SDL_Renderer* renderer, const Level* level, int camera_x, int camera_y);

#endif // ENEMY_RENDERER_H
