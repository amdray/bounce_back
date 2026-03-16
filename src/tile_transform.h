#ifndef TILE_TRANSFORM_H
#define TILE_TRANSFORM_H

#include <SDL2/SDL.h>
#include <stdint.h>

void draw_tile_with_transform(SDL_Renderer* renderer,
                              SDL_Texture* texture,
                              const SDL_Rect* dest,
                              uint8_t transform);
void transform_point_16(uint8_t transform, int* x, int* y);

#endif // TILE_TRANSFORM_H
