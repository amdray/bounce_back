/**
 * Exit door state (I/o) - Step 08.
 */

#ifndef EXIT_DOOR_H
#define EXIT_DOOR_H

#include <stdbool.h>

#include <SDL2/SDL.h>
#include "level_loader.h"
#include "player.h"

typedef struct {
    bool open; // o
    int I;     // I
} ExitDoorState;

void exit_door_tick(ExitDoorState* door, int objective_remaining);
bool exit_door_test_complete(ExitDoorState* door, Level* level, Player* p);

#include "game_assets.h"

/* Renderer (h.java:c(Graphics)) */
int  exit_door_renderer_init(const IcAssets* ic);
void exit_door_renderer_shutdown(void);
void exit_door_render(SDL_Renderer* renderer, const ExitDoorState* door,
                      const Level* level, int camera_x, int camera_y);

#endif // EXIT_DOOR_H
