#include "tile_transform.h"

void draw_tile_with_transform(SDL_Renderer* renderer,
                              SDL_Texture* texture,
                              const SDL_Rect* dest,
                              uint8_t transform) {
    if (!renderer || !texture || !dest) return;

    double angle = 0.0;
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (transform & 0x8) {
        flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
    }
    if (transform & 0x4) {
        flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);
    }

    switch (transform & 0x3) {
        case 1:
            angle = 270.0;
            break;
        case 2:
            angle = 180.0;
            break;
        case 3:
            angle = 90.0;
            break;
        default:
            angle = 0.0;
            break;
    }

    SDL_RenderCopyEx(renderer, texture, NULL, dest, angle, NULL, flip);
}
