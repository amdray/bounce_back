/**
 * Camera with deadzone - Step 06.
 *
 * Spec: docs/STEP_06_CAMERA.md
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define TILE_SIZE 16
#define CAMERA_DEADZONE_PERCENT 30

typedef struct {
    int x;
    int y;
    bool initialized;
} Camera;

void camera_init(Camera* cam);
void camera_reset(Camera* cam, int level_width, int level_height);
void camera_update(Camera* cam, int player_x, int player_y, int level_width, int level_height);

#endif // CAMERA_H

