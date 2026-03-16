#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdint.h>

SDL_Texture* texture_loader_png_from_memory(SDL_Renderer* renderer, const uint8_t* data, size_t size);

#endif // TEXTURE_LOADER_H
