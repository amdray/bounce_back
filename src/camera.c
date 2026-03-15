/**
 * Camera implementation (bounce_zero/src/game.c logic) - Step 06.
 */

#include "camera.h"

// Gameplay viewport excludes HUD strip at bottom.
#define HUD_HEIGHT 21
#define CAMERA_VIEW_HEIGHT (SCREEN_HEIGHT - HUD_HEIGHT)

static inline bool is_level_small(int level_height) {
    return (level_height * TILE_SIZE) < CAMERA_VIEW_HEIGHT;
}

static inline int get_center_offset_y(int level_height) {
    int levelPixelHeight = level_height * TILE_SIZE;
    return -(CAMERA_VIEW_HEIGHT - levelPixelHeight) / 2;
}

void camera_init(Camera* cam) {
    if (!cam) return;
    cam->x = 0;
    cam->y = 0;
    cam->initialized = false;
}

void camera_reset(Camera* cam, int level_width, int level_height) {
    (void)level_width;
    if (!cam) return;

    if (is_level_small(level_height)) {
        cam->y = get_center_offset_y(level_height);
        cam->initialized = true;
    } else {
        cam->y = 0;
        cam->initialized = false;
    }
    cam->x = 0;
}

void camera_update(Camera* cam, int player_x, int player_y, int level_width, int level_height) {
    if (!cam) return;

    int level_px_w = level_width * TILE_SIZE;
    int level_px_h = level_height * TILE_SIZE;

    // Horizontal: center on player + clamp
    int cameraX = player_x - SCREEN_WIDTH / 2;
    int maxCameraX = level_px_w - SCREEN_WIDTH;
    if (maxCameraX <= 0) {
        cameraX = 0;
    } else {
        if (cameraX < 0) cameraX = 0;
        if (cameraX > maxCameraX) cameraX = maxCameraX;
    }
    cam->x = cameraX;

    // Vertical: small levels centered (no movement)
    if (is_level_small(level_height)) {
        cam->y = get_center_offset_y(level_height);
        cam->initialized = true;
        return;
    }

    if (!cam->initialized) {
        cam->y = player_y - CAMERA_VIEW_HEIGHT / 2;
        cam->initialized = true;
    }

    int deadZoneTop = (CAMERA_VIEW_HEIGHT * CAMERA_DEADZONE_PERCENT) / 100;
    int deadZoneBottom = CAMERA_VIEW_HEIGHT - deadZoneTop;

    int tempPlayerScreenY = player_y - cam->y;
    if (tempPlayerScreenY < deadZoneTop) {
        cam->y = player_y - deadZoneTop;
    } else if (tempPlayerScreenY > deadZoneBottom) {
        cam->y = player_y - deadZoneBottom;
    }

    int maxCameraY = level_px_h - CAMERA_VIEW_HEIGHT;
    if (cam->y < 0) cam->y = 0;
    if (cam->y > maxCameraY && maxCameraY > 0) cam->y = maxCameraY;
}
