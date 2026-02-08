#ifndef TILE_TRANSFORM_H
#define TILE_TRANSFORM_H

#include <SDL2/SDL.h>
#include <stdint.h>

void draw_tile_with_transform(SDL_Renderer* renderer,
                              SDL_Texture* texture,
                              const SDL_Rect* dest,
                              uint8_t transform);

#endif // TILE_TRANSFORM_H
