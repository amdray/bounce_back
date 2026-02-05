/**
 * Pixel-perfect collision implementation - Step 4.
 */

#include "collision.h"

static int max_i(int a, int b) { return (a > b) ? a : b; }
static int min_i(int a, int b) { return (a < b) ? a : b; }

void apply_transform(uint8_t transform, int* x, int* y) {
    int k = 15;
    int m = 15;
    int i10 = *x;
    int i11 = *y;

    if (transform & 0x8) {
        i10 = k - i10;
    }
    if (transform & 0x4) {
        i11 = m - i11;
    }

    int rotate = transform & 0x3;
    switch (rotate) {
        case 0:
            break;
        case 1:
            {
                int temp = i10;
                i10 = i11;
                i11 = m - temp;
            }
            break;
        case 2:
            i10 = k - i10;
            i11 = m - i11;
            break;
        case 3:
            {
                int temp = i11;
                i11 = i10;
                i10 = m - temp;
            }
            break;
    }

    *x = i10;
    *y = i11;
}

bool collision_test(Level* level, TileMetadata* tile_meta,
                    CollisionMasks* masks,
                    int rect_x, int rect_y, int rect_w, int rect_h,
                    bool* player_mask) {
    if (!level || !tile_meta || !masks) return false;

    int start_tile_x = rect_x / 16;
    int end_tile_x = (rect_x + rect_w - 1) / 16;
    int start_tile_y = rect_y / 16;
    int end_tile_y = (rect_y + rect_h - 1) / 16;

    for (int ty = start_tile_y; ty <= end_tile_y; ty++) {
        for (int tx = start_tile_x; tx <= end_tile_x; tx++) {
            if (tx < 0 || tx >= (int)level->width || ty < 0 || ty >= (int)level->height) {
                continue;
            }

            uint8_t tile_byte = level_get_tile(level, tx, ty);
            uint8_t tile_id = (uint8_t)(tile_byte & 0x7F);
            if (tile_id == 0) continue;

            TileMetadata* tm = &tile_meta[tile_id];
            if (tm->collision_type == 0) continue;
            if (tm->collision_type == 2) return true;

            int tile_px_x = tx * 16;
            int tile_px_y = ty * 16;

            int overlap_x1 = max_i(rect_x, tile_px_x);
            int overlap_y1 = max_i(rect_y, tile_px_y);
            int overlap_x2 = min_i(rect_x + rect_w, tile_px_x + 16);
            int overlap_y2 = min_i(rect_y + rect_h, tile_px_y + 16);

            bool** tile_mask = masks->masks[tile_id];
            if (!tile_mask) continue;

            for (int py = overlap_y1; py < overlap_y2; py++) {
                for (int px = overlap_x1; px < overlap_x2; px++) {
                    int tile_local_x = px - tile_px_x;
                    int tile_local_y = py - tile_px_y;

                    int mask_x = tile_local_x;
                    int mask_y = tile_local_y;
                    if (tm->collision_type == 3) {
                        apply_transform(tm->transform, &mask_x, &mask_y);
                    }

                    if (!tile_mask[mask_x][mask_y]) {
                        continue;
                    }

                    if (player_mask) {
                        int player_local_x = px - rect_x;
                        int player_local_y = py - rect_y;
                        if (!player_mask[player_local_y * rect_w + player_local_x]) {
                            continue;
                        }
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

