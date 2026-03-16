#include "texture_loader.h"

#include <SDL2/SDL_image.h>

SDL_Texture* texture_loader_png_from_memory(SDL_Renderer* renderer, const uint8_t* data, size_t size) {
    if (!renderer || !data || size == 0) return NULL;

    SDL_RWops* rw = SDL_RWFromConstMem(data, (int)size);
    if (!rw) return NULL;

    SDL_Surface* surface = IMG_LoadTyped_RW(rw, 1, "PNG");
    if (!surface) return NULL;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}
