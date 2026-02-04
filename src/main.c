/**
 * Bounce Back - PSP Port
 * Minimal bring-up: load first ball sprite from /res/b
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "resource_loader.h"

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    FILE* log = fopen("debug.log", "w");
    if (log) fprintf(log, "=== Bounce Back Debug ===\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        if (log) { fprintf(log, "SDL_Init failed: %s\n", SDL_GetError()); fclose(log); }
        return 1;
    }
    if (log) fprintf(log, "SDL2 OK\n");

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        if (log) { fprintf(log, "IMG_Init failed: %s\n", IMG_GetError()); fclose(log); }
        SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "SDL2_image OK\n");

    SDL_Window* window = SDL_CreateWindow("Bounce Back", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        if (log) { fprintf(log, "CreateWindow failed: %s\n", SDL_GetError()); fclose(log); }
        IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Window OK\n");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        if (log) { fprintf(log, "CreateRenderer failed: %s\n", SDL_GetError()); fclose(log); }
        SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Renderer OK\n");

    ResourceContainer* ball_res = resource_load("res/b");
    if (!ball_res) {
        if (log) { fprintf(log, "Failed to load res/b\n"); fclose(log); }
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "res/b loaded: %u elements\n", ball_res->count);

    // Элемент 0 в /res/b - кастомный формат (02 10 10 ...), не PNG
    // Элементы 1-25 - нормальные PNG спрайты
    size_t png_size = 0;
    const uint8_t* png_data = resource_get_element(ball_res, 1, &png_size);
    if (!png_data) {
        if (log) { fprintf(log, "Failed to get sprite 0\n"); fclose(log); }
        resource_free(ball_res); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) {
        fprintf(log, "Sprite 0: %zu bytes\n", png_size);
        fprintf(log, "First 16 bytes: ");
        for (int i = 0; i < 16 && i < (int)png_size; i++) {
            fprintf(log, "%02X ", png_data[i]);
        }
        fprintf(log, "\n");
    }

    SDL_RWops* rw = SDL_RWFromConstMem(png_data, png_size);
    if (!rw) {
        if (log) { fprintf(log, "RWFromConstMem failed\n"); fclose(log); }
        resource_free(ball_res); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }

    SDL_Surface* ball_surface = IMG_LoadTyped_RW(rw, 1, "PNG");
    if (!ball_surface) {
        if (log) { fprintf(log, "IMG_Load_RW failed: %s\n", IMG_GetError()); fclose(log); }
        resource_free(ball_res); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }
    if (log) fprintf(log, "Sprite loaded: %dx%d\n", ball_surface->w, ball_surface->h);

    SDL_Texture* ball_texture = SDL_CreateTextureFromSurface(renderer, ball_surface);
    SDL_FreeSurface(ball_surface);
    if (!ball_texture) {
        if (log) { fprintf(log, "CreateTexture failed\n"); fclose(log); }
        resource_free(ball_res); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit();
        return 1;
    }

    int ball_w, ball_h;
    SDL_QueryTexture(ball_texture, NULL, NULL, &ball_w, &ball_h);
    if (log) fprintf(log, "Starting main loop...\n");

    SDL_GameController* controller = NULL;
    if (SDL_NumJoysticks() > 0) {
        controller = SDL_GameControllerOpen(0);
        if (log) fprintf(log, "Controller: %s\n", controller ? "OK" : "FAIL");
    }

    SDL_Event event;
    int running = 1;
    Uint32 last_time = SDL_GetTicks();
    int frame_count = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_CONTROLLERDEVICEADDED && !controller) {
                controller = SDL_GameControllerOpen(event.cdevice.which);
            }
            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A || event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    running = 0;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect dest_rect = { (SCREEN_WIDTH - ball_w) / 2, (SCREEN_HEIGHT - ball_h) / 2, ball_w, ball_h };
        SDL_RenderCopy(renderer, ball_texture, NULL, &dest_rect);
        SDL_RenderPresent(renderer);

        frame_count++;
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_time >= 1000) {
            if (log) { fprintf(log, "FPS: %d\n", frame_count); fflush(log); }
            frame_count = 0;
            last_time = current_time;
        }

        SDL_Delay(16);
    }

    if (log) { fprintf(log, "Shutdown OK\n"); fclose(log); }
    if (controller) SDL_GameControllerClose(controller);
    SDL_DestroyTexture(ball_texture);
    resource_free(ball_res);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
