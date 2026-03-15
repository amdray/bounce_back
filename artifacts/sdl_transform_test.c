/*
 * Renders all 12 transform values of ic_05.png using real SDL_RenderCopyEx.
 * Each row: LEFT=transform(t), RIGHT=rot270(t=0x01)
 * Gap of 4px between left and right.
 * Saves result to sdl_transform_out.bmp
 *
 * Compile:
 *   gcc sdl_transform_test.c -o sdl_transform_test $(sdl2-config --cflags --libs) -lSDL2_image
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdint.h>

static void draw_tile_with_transform(SDL_Renderer* renderer, SDL_Texture* tex,
                                     const SDL_Rect* dest, uint8_t t) {
    double angle = 0.0;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (t & 0x8) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
    if (t & 0x4) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);
    switch (t & 0x3) {
        case 1: angle = 270.0; break;
        case 2: angle = 180.0; break;
        case 3: angle =  90.0; break;
    }
    SDL_RenderCopyEx(renderer, tex, NULL, dest, angle, NULL, flip);
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    const int SCALE = 8;
    const int TILE  = 16;
    const int GAP   = 0; /* no gap: Java places right tile at x + 16 exactly */
    const int ROWS  = 12;
    const int ROW_H = TILE * SCALE + 2 * SCALE; /* 2px row separator */
    const int WIN_W = (TILE * 2) * SCALE;
    const int WIN_H = ROWS * ROW_H;

    SDL_Window*   win = SDL_CreateWindow("transform test",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            WIN_W, WIN_H, SDL_WINDOW_HIDDEN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);

    SDL_Texture* tex = IMG_LoadTexture(ren,
        "/mnt/d/OneDrive/VSCodeProject/psp/bounce_back/artifacts/ic_dump/ic_05.png");
    if (!tex) { printf("Load failed: %s\n", IMG_GetError()); return 1; }

    SDL_SetRenderDrawColor(ren, 40, 40, 40, 255);
    SDL_RenderClear(ren);

    for (int t = 0; t < 12; t++) {
        int y = t * ROW_H;

        /* left tile: transform t */
        SDL_Rect left = { 0, y, TILE * SCALE, TILE * SCALE };
        draw_tile_with_transform(ren, tex, &left, (uint8_t)t);

        /* right tile: always rot270 (t=0x01) — at x+16, no gap */
        SDL_Rect right = { TILE * SCALE, y, TILE * SCALE, TILE * SCALE };
        draw_tile_with_transform(ren, tex, &right, 0x01);
    }

    SDL_RenderPresent(ren);

    /* save to BMP */
    SDL_Surface* surf = SDL_CreateRGBSurface(0, WIN_W, WIN_H, 32,
                            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_RenderReadPixels(ren, NULL, surf->format->format, surf->pixels, surf->pitch);
    SDL_SaveBMP(surf,
        "/mnt/d/OneDrive/VSCodeProject/psp/bounce_back/artifacts/sdl_transform_out.bmp");
    SDL_FreeSurface(surf);

    printf("Saved sdl_transform_out.bmp\n");
    printf("Row 0 = t=0, Row 1 = t=1, ... Row 11 = t=11\n");
    printf("LEFT = that transform, RIGHT = rot270 (always)\n");

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
