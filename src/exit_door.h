/**
 * Exit door state (I/o) - Step 08.
 */

#ifndef EXIT_DOOR_H
#define EXIT_DOOR_H

#include <stdbool.h>

#include "level_loader.h"
#include "player.h"

typedef struct {
    bool open; // o
    int I;     // I
} ExitDoorState;

void exit_door_tick(ExitDoorState* door, int objective_remaining);
bool exit_door_test_complete(ExitDoorState* door, Level* level, Player* p);

#endif // EXIT_DOOR_H
