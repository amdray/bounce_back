#include "tile_transform.h"

void transform_point_16(uint8_t transform, int* x, int* y) {
    int max_x = 15;
    int max_y = 15;
    int tx = *x;
    int ty = *y;

    if (transform & 0x8) tx = max_x - tx;
    if (transform & 0x4) ty = max_y - ty;

    switch (transform & 0x3) {
        case 0:
            break;
        case 1: {
            int t = tx;
            tx = ty;
            ty = max_y - t;
            break;
        }
        case 2:
            tx = max_x - tx;
            ty = max_y - ty;
            break;
        case 3: {
            int t = ty;
            ty = tx;
            tx = max_y - t;
            break;
        }
    }

    *x = tx;
    *y = ty;
}

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
