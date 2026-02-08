/**
 * Exit door implementation (a.java:607-612, h.java:664-672) - Step 08.
 */

#include "exit_door.h"

#include "level_loader.h"
#include "player.h"

void exit_door_tick(ExitDoorState* door, int objective_remaining) {
    if (!door) return;

    if (!door->open && objective_remaining == 0) {
        door->I++;
        if (door->I == 48) {
            door->open = true;
        }
        return;
    }

    if (door->open) {
        door->I++;
        if (door->I == 72) {
            door->I = 48;
        }
    }
}

bool exit_door_test_complete(ExitDoorState* door, Level* level, Player* p) {
    if (!door || !level || !p) return false;
    if (!door->open) return false;

    int door_px_x = (int)level->exit_x * 16;
    int door_px_y = (int)level->exit_y * 16;

    return (door_px_x < p->x_pos && p->x_pos < door_px_x + 32 &&
            door_px_y < p->y_pos && p->y_pos < door_px_y + 48);
}
