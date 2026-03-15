/**
 * Level loader + runtime collision/object API.
 */

#include "level_loader.h"
#include "resource_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int require_size_at_least(const char* ctx, size_t size, size_t need) {
    if (size < need) {
        fprintf(stderr, "%s: buffer too small: have=%zu need=%zu\n", ctx, size, need);
        return 0;
    }
    return 1;
}

static int require_bytes(const char* ctx, const uint8_t* p, const uint8_t* end, size_t need) {
    size_t have = (size_t)(end - p);
    if (have < need) {
        fprintf(stderr, "%s: need %zu bytes, have %zu\n", ctx, need, have);
        return 0;
    }
    return 1;
}

static uint32_t read_be32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int clamp_0_to_max(int value, int max_exclusive) {
    if (value < 0) return 0;
    if (value >= max_exclusive) return max_exclusive - 1;
    return value;
}

static int ptr_seen(bool*** seen, size_t seen_count, bool** ptr) {
    for (size_t i = 0; i < seen_count; i++) {
        if (seen[i] == ptr) return 1;
    }
    return 0;
}

static void level_objects_free(Level* level) {
    if (!level) return;
    free(level->objects.ao);
    free(level->objects.f);
    free(level->objects.k);
    free(level->objects.ag);
    free(level->objects.s);
    memset(&level->objects, 0, sizeof(level->objects));
}

static void level_runtime_masks_free(Level* level) {
    if (!level) return;
    if (level->masks) {
        bool*** seen = NULL;
        size_t seen_count = 0;
        if (level->tile_count > 0) {
            seen = (bool***)calloc(level->tile_count, sizeof(bool**));
        }
        for (uint8_t tid = 0; tid < level->tile_count; tid++) {
            bool** tile_mask = level->masks[tid];
            if (!tile_mask) continue;
            if (seen && ptr_seen(seen, seen_count, tile_mask)) continue;
            if (seen) seen[seen_count++] = tile_mask;
            for (uint8_t x = 0; x < level->tile_w; x++) {
                free(tile_mask[x]);
            }
            free(tile_mask);
        }
        free(seen);
        free(level->masks);
        level->masks = NULL;
    }
    free(level->collision_type);
    level->collision_type = NULL;
    free(level->transform);
    level->transform = NULL;
}

static int level_runtime_load_tf(Level* level) {
    ResourceContainer* tf = resource_load("res/tf");
    if (!tf || tf->count < 2) {
        if (tf) resource_free(tf);
        return 0;
    }

    size_t obj0_size = 0, obj1_size = 0;
    const uint8_t* obj0 = resource_get_element(tf, 0, &obj0_size);
    const uint8_t* obj1 = resource_get_element(tf, 1, &obj1_size);
    if (!obj0 || !obj1) {
        resource_free(tf);
        return 0;
    }

    const uint8_t* p0 = obj0;
    const uint8_t* end0 = obj0 + obj0_size;
    const uint8_t* p1 = obj1;
    const uint8_t* end1 = obj1 + obj1_size;

    if (!require_bytes("level_runtime_load_tf(header)", p0, end0, 14)) {
        resource_free(tf);
        return 0;
    }
    uint8_t tile_w = p0[4];
    uint8_t tile_h = p0[5];
    uint8_t tile_count = p0[6];
    uint8_t split_index = p0[7];
    level->tile_id_mask = p0[8];
    level->tile_flag_mask = p0[9];
    level->tile_w = (tile_w == 12) ? 16 : tile_w;
    level->tile_h = (tile_h == 12) ? 16 : tile_h;
    level->tile_count = tile_count;
    p0 += 14;

    if (!require_bytes("level_runtime_load_tf(anim hdr)", p0, end0, 1)) {
        resource_free(tf);
        return 0;
    }
    uint8_t anim_count = p0[0];
    p0 += 1;
    for (uint8_t i = 0; i < anim_count; i++) {
        if (!require_bytes("level_runtime_load_tf(anim entry)", p0, end0, 3)) {
            resource_free(tf);
            return 0;
        }
        uint8_t frame_count = p0[2];
        p0 += 3;
        if (!require_bytes("level_runtime_load_tf(anim frames)", p0, end0, frame_count)) {
            resource_free(tf);
            return 0;
        }
        p0 += frame_count;
    }

    level->collision_type = (uint8_t*)calloc(tile_count, sizeof(uint8_t));
    level->transform = (uint8_t*)calloc(tile_count, sizeof(uint8_t));
    level->masks = (bool***)calloc(tile_count, sizeof(bool**));
    if (!level->collision_type || !level->transform || !level->masks) {
        resource_free(tf);
        return 0;
    }

    for (uint8_t tile_id = 0; tile_id < tile_count; tile_id++) {
        const uint8_t** p = (tile_id < split_index) ? &p0 : &p1;
        const uint8_t* end = (tile_id < split_index) ? end0 : end1;
        if (!require_bytes("level_runtime_load_tf(tile fixed)", *p, end, 5)) {
            resource_free(tf);
            return 0;
        }

        uint8_t render_type = (*p)[1];
        (void)render_type;
        uint8_t image_index = (*p)[2];
        (void)image_index;
        uint8_t transform = (*p)[3];
        uint8_t collision_type = (*p)[4];
        *p += 5;

        if (collision_type == 1) {
            bool** tile_mask = (bool**)calloc(level->tile_w, sizeof(bool*));
            if (!tile_mask) {
                resource_free(tf);
                return 0;
            }
            for (uint8_t x = 0; x < level->tile_w; x++) {
                tile_mask[x] = (bool*)calloc(level->tile_h, sizeof(bool));
                if (!tile_mask[x]) {
                    for (uint8_t i = 0; i < x; i++) free(tile_mask[i]);
                    free(tile_mask);
                    resource_free(tf);
                    return 0;
                }
            }
            size_t mask_bytes = (size_t)level->tile_w * (size_t)level->tile_h;
            if (!require_bytes("level_runtime_load_tf(mask)", *p, end, mask_bytes)) {
                for (uint8_t x = 0; x < level->tile_w; x++) free(tile_mask[x]);
                free(tile_mask);
                resource_free(tf);
                return 0;
            }
            // Keep the original Java storage order: bytes are read row-by-row,
            // but stored as tile_mask[x][y]. Collision intentionally samples the
            // same data as tile_mask[y][x]; resource data in /res/tf depends on it.
            for (uint8_t y = 0; y < level->tile_h; y++) {
                for (uint8_t x = 0; x < level->tile_w; x++) {
                    tile_mask[x][y] = ((*p)[0] != 0);
                    (*p)++;
                }
            }
            level->masks[tile_id] = tile_mask;
        }

        if (!require_bytes("level_runtime_load_tf(aux)", *p, end, 4)) {
            resource_free(tf);
            return 0;
        }
        uint32_t aux = read_be32(*p);
        *p += 4;

        if (collision_type == 3) {
            uint8_t base = (uint8_t)(aux & 0xFF);
            if (base >= tile_count) {
                resource_free(tf);
                return 0;
            }
            level->masks[tile_id] = level->masks[base];
        }

        level->collision_type[tile_id] = collision_type;
        level->transform[tile_id] = transform;
    }

    resource_free(tf);
    return 1;
}

static void apply_transform_16(uint8_t transform, int* x, int* y) {
    int k = 15;
    int m = 15;
    int i10 = *x;
    int i11 = *y;
    if (transform & 0x8) i10 = k - i10;
    if (transform & 0x4) i11 = m - i11;
    switch (transform & 0x3) {
        case 0:
            break;
        case 1: {
            int t = i10;
            i10 = i11;
            i11 = m - t;
            break;
        }
        case 2:
            i10 = k - i10;
            i11 = m - i11;
            break;
        case 3: {
            int t = i11;
            i11 = i10;
            i10 = m - t;
            break;
        }
    }
    *x = i10;
    *y = i11;
}

static int object_width(const Level* level, int idx, int for_tile_units) {
    (void)level;
    switch (level->objects.ao[idx]) {
        case 0: return for_tile_units ? 2 : 32;
        case 1: return for_tile_units ? 1 : 16;
        case 2: return for_tile_units ? 2 : 24;
        default: return 0;
    }
}

static int object_height(const Level* level, int idx, int for_tile_units) {
    (void)level;
    switch (level->objects.ao[idx]) {
        case 0: return for_tile_units ? 2 : 32;
        case 1: return for_tile_units ? 1 : 16;
        case 2: return for_tile_units ? 1 : 11;
        default: return 0;
    }
}

static int rects_overlap(int x1, int y1, int x2, int y2, int rx1, int ry1, int rx2, int ry2) {
    return (x2 > rx1 && y2 > ry1 && x1 < rx2 && y1 < ry2);
}

// Java h.java:356 counts initial hoops only for 93, 94, 97, 101.
static int counts_toward_initial_hoops(uint8_t tile_id) {
    return (tile_id == 93 || tile_id == 94 || tile_id == 97 || tile_id == 101);
}

Level* level_load(const char* lf_path, int level_index) {
    if (!lf_path || level_index < 0) return NULL;

    ResourceContainer* lf = resource_load(lf_path);
    if (!lf) return NULL;

    int meta_index = 2 * level_index;
    int map_index = 2 * level_index + 1;
    if (meta_index < 0 || map_index < 0 || meta_index >= lf->count || map_index >= lf->count) {
        resource_free(lf);
        return NULL;
    }

    size_t meta_size = 0;
    const uint8_t* meta = resource_get_element(lf, meta_index, &meta_size);
    size_t map_size = 0;
    const uint8_t* map = resource_get_element(lf, map_index, &map_size);
    if (!meta || !map || !require_size_at_least("level_load(meta)", meta_size, 7) ||
        !require_size_at_least("level_load(map)", map_size, 2)) {
        resource_free(lf);
        return NULL;
    }

    Level* level = (Level*)calloc(1, sizeof(Level));
    if (!level) {
        resource_free(lf);
        return NULL;
    }

    level->theme_id = meta[0];
    level->spawn_y = meta[1];
    level->spawn_x = meta[2];
    level->ball_type = meta[3];
    level->exit_y = meta[4];
    level->exit_x = meta[5];
    uint8_t object_count = meta[6];

    size_t meta_need = 7 + (size_t)object_count * 9;
    if (!require_size_at_least("level_load(meta objects)", meta_size, meta_need)) {
        level_free(level);
        resource_free(lf);
        return NULL;
    }

    level->height = map[0];
    level->width = map[1];
    size_t tiles_count = (size_t)level->height * (size_t)level->width;
    if (!require_size_at_least("level_load(map bytes)", map_size, 2 + tiles_count)) {
        level_free(level);
        resource_free(lf);
        return NULL;
    }

    level->tile_map = (uint8_t*)malloc(tiles_count);
    if (!level->tile_map) {
        level_free(level);
        resource_free(lf);
        return NULL;
    }
    memcpy(level->tile_map, map + 2, tiles_count);

    // Count initial hoops exactly as Java h.java:356 (E.W analog).
    level->hoops_remaining = 0;
    for (size_t i = 0; i < tiles_count; i++) {
        uint8_t tile_id = level->tile_map[i] & 0x7F;
        if (counts_toward_initial_hoops(tile_id)) {
            level->hoops_remaining++;
        }
    }
    level->hoops_total = level->hoops_remaining;

    if (object_count > 0) {
        level->objects.count = object_count;
        level->objects.ao = (uint8_t*)calloc(object_count, sizeof(uint8_t));
        level->objects.f = (int(*)[2])calloc(object_count, sizeof(int[2]));
        level->objects.k = (int(*)[2])calloc(object_count, sizeof(int[2]));
        level->objects.ag = (int(*)[2])calloc(object_count, sizeof(int[2]));
        level->objects.s = (int8_t(*)[2])calloc(object_count, sizeof(int8_t[2]));
        if (!level->objects.ao || !level->objects.f || !level->objects.k ||
            !level->objects.ag || !level->objects.s) {
            level_free(level);
            resource_free(lf);
            return NULL;
        }

        size_t off = 7;
        for (uint8_t i = 0; i < object_count; i++) {
            uint8_t ao = meta[off + 0];
            uint8_t b8 = meta[off + 1];
            uint8_t b9 = meta[off + 2];
            uint8_t b10 = meta[off + 3];
            uint8_t b11 = meta[off + 4];
            int m = (int)meta[off + 5] * 16;
            int n = (int)meta[off + 6] * 16;
            int8_t s1 = (int8_t)meta[off + 7];
            int8_t s0 = (int8_t)meta[off + 8];
            off += 9;

            if (b8 > b10 || b9 > b11) {
                uint8_t t = b8; b8 = b10; b10 = t;
                t = b9; b9 = b11; b11 = t;
                m = (int)(b10 - b8) * 16;
                n = (int)(b11 - b9) * 16;
                if (s0 > 0 || s1 > 0) {
                    s0 = (int8_t)-s0;
                    s1 = (int8_t)-s1;
                }
            }

            level->objects.ao[i] = ao;
            level->objects.f[i][0] = b8;
            level->objects.f[i][1] = b9;
            level->objects.k[i][0] = b10;
            level->objects.k[i][1] = b11;
            level->objects.ag[i][1] = m;
            level->objects.ag[i][0] = n;
            level->objects.s[i][1] = s1;
            level->objects.s[i][0] = s0;
        }
    }

    for (int i = 0; i < 5; i++) {
        level->hit_cols[i] = -1;
        level->hit_rows[i] = -1;
    }
    level->hits_overflow = false;
    level->active_object_index = -1;

    if (!level_runtime_load_tf(level)) {
        level_free(level);
        resource_free(lf);
        return NULL;
    }

    resource_free(lf);
    return level;
}

void level_free(Level* level) {
    if (!level) return;
    free(level->tile_map);
    level_objects_free(level);
    level_runtime_masks_free(level);
    free(level);
}

uint8_t level_get_tile(const Level* level, int tile_x, int tile_y) {
    if (!level || !level->tile_map) return 0;
    if (tile_x < 0 || tile_y < 0) return 0;
    if (tile_x >= (int)level->width || tile_y >= (int)level->height) return 0;
    return level->tile_map[(size_t)tile_y * (size_t)level->width + (size_t)tile_x];
}

void level_set_tile(Level* level, int tile_x, int tile_y, uint8_t tile_byte) {
    if (!level || !level->tile_map) return;
    if (tile_x < 0 || tile_y < 0) return;
    if (tile_x >= (int)level->width || tile_y >= (int)level->height) return;
    level->tile_map[(size_t)tile_y * (size_t)level->width + (size_t)tile_x] = tile_byte;
}

void level_objects_tick(Level* level,
                        int player_left,
                        int player_top,
                        int player_right,
                        int player_bottom,
                        bool player_is_popped) {
    if (!level || level->objects.count == 0) return;
    for (uint8_t idx = 0; idx < level->objects.count; idx++) {
        uint8_t type = level->objects.ao[idx];
        int n = level->objects.f[idx][0];
        int i1 = level->objects.f[idx][1];
        int i2 = level->objects.k[idx][0];
        int i3 = level->objects.k[idx][1];
        int i4 = level->objects.ag[idx][0];
        int i5 = level->objects.ag[idx][1];
        int b2 = level->objects.s[idx][0];
        int b3 = level->objects.s[idx][1];

        if (type == 0 || type == 2) {
            int i6 = object_width(level, idx, 0);
            int i7 = object_height(level, idx, 0);
            int b4 = (b2 > 0) ? 1 : -1;
            int i8 = abs(b2);
            int i9 = i1 * 16 + i4;
            int i10 = n * 16 + i5;
            int i11 = i9 + i6;
            int i12 = i10 + i7;
            int i13 = (i3 - i1) * 16;
            int i14 = (i2 - n) * 16;

            for (int c = 0; c < i8; c++) {
                if (player_is_popped && type == 0) {
                    if (!rects_overlap(i9, i10, i11, i12,
                                       player_left, player_top, player_right, player_bottom)) {
                        i4 += b4;
                    }
                } else {
                    i4 += b4;
                }
                if (i4 == 0 || i4 == i13) {
                    b2 = -b2;
                    b4 = -b4;
                }
            }
            i4 = clamp_0_to_max(i4, i13 + 1);

            b4 = (b3 > 0) ? 1 : -1;
            i8 = abs(b3);
            for (int c = 0; c < i8; c++) {
                if (player_is_popped && type == 0) {
                    if (!rects_overlap(i9, i10, i11, i12,
                                       player_left, player_top, player_right, player_bottom)) {
                        i5 += b4;
                    }
                } else {
                    i5 += b4;
                }
                if (i5 == 0 || i5 == i14) {
                    b3 = -b3;
                }
            }
            i5 = clamp_0_to_max(i5, i14 + 1);
        } else {
            int i6 = (i2 - n) * 16;
            if (i5 == 0) b3 = 30;
            if (i5 == i6) b3 = -40;
            if (++b3 > 80) b3 = 80;

            int i7 = object_width(level, idx, 0);
            int i8 = object_height(level, idx, 0);
            int b4 = (b3 > 0) ? 1 : -1;
            int i9 = abs(b3 / 10);
            if (i9 > 3) i9 = 3;

            for (int c = 0; c < i9; c++) {
                if (player_is_popped) {
                    int i10 = i1 * 16;
                    int i11 = n * 16 + i5;
                    int i12 = i10 + i7;
                    int i13 = i11 + i8;
                    if (!rects_overlap(i10, i11, i12, i13,
                                       player_left, player_top, player_right, player_bottom)) {
                        i5 += b4;
                    }
                } else {
                    i5 += b4;
                }
            }
            i5 = clamp_0_to_max(i5, i6 + 1);
        }

        level->objects.ag[idx][0] = i4;
        level->objects.ag[idx][1] = i5;
        level->objects.s[idx][0] = (int8_t)b2;
        level->objects.s[idx][1] = (int8_t)b3;
    }
}

bool level_test_collision_collect(Level* level,
                                  int rect_x, int rect_y, int rect_w, int rect_h,
                                  const bool* player_mask,
                                  int* out_hit_x, int* out_hit_y, int max_hits,
                                  bool* out_overflow,
                                  int* out_active_object_index) {
    if (!level || !level->tile_map || !level->collision_type || !level->transform || !level->masks) return false;
    if (max_hits <= 0) return false;

    for (int i = 0; i < max_hits; i++) {
        out_hit_x[i] = -1;
        out_hit_y[i] = -1;
    }
    for (int i = 0; i < 5; i++) {
        level->hit_cols[i] = -1;
        level->hit_rows[i] = -1;
    }
    level->hits_overflow = false;
    level->active_object_index = -1;
    if (out_overflow) *out_overflow = false;
    if (out_active_object_index) *out_active_object_index = -1;

    int hit_count = 0;
    int start_tile_x = rect_x / level->tile_w;
    int end_tile_x = (rect_x + rect_w) / level->tile_w;
    int start_tile_y = rect_y / level->tile_h;
    int end_tile_y = (rect_y + rect_h) / level->tile_h;
    bool collided = false;

    for (int tx = start_tile_x; tx <= end_tile_x; tx++) {
        for (int ty = start_tile_y; ty <= end_tile_y; ty++) {
            if (tx < 0 || ty < 0 || tx >= (int)level->width || ty >= (int)level->height) {
                if (out_overflow) *out_overflow = true;
                level->hits_overflow = true;
                return true;
            }

            uint8_t tile_byte = level_get_tile(level, tx, ty);
            uint8_t tile_id = (uint8_t)(tile_byte & level->tile_id_mask);
            if (tile_id >= level->tile_count) continue;
            uint8_t collision_type = level->collision_type[tile_id];
            if (collision_type == 0) continue;

            int tile_px_x = tx * level->tile_w;
            int tile_px_y = ty * level->tile_h;
            int overlap_x1 = (rect_x > tile_px_x) ? rect_x : tile_px_x;
            int overlap_y1 = (rect_y > tile_px_y) ? rect_y : tile_px_y;
            int overlap_x2 = ((rect_x + rect_w) < (tile_px_x + level->tile_w)) ? (rect_x + rect_w) : (tile_px_x + level->tile_w);
            int overlap_y2 = ((rect_y + rect_h) < (tile_px_y + level->tile_h)) ? (rect_y + rect_h) : (tile_px_y + level->tile_h);
            if (overlap_x1 >= overlap_x2 || overlap_y1 >= overlap_y2) continue;

            bool tile_collided = false;
            if (collision_type == 2) {
                for (int py = overlap_y1; py < overlap_y2 && !tile_collided; py++) {
                    for (int px = overlap_x1; px < overlap_x2; px++) {
                        if (player_mask) {
                            int local_x = px - rect_x;
                            int local_y = py - rect_y;
                            if (!player_mask[local_y * rect_w + local_x]) continue;
                        }
                        tile_collided = true;
                        break;
                    }
                }
            } else {
                bool** tile_mask = level->masks[tile_id];
                if (!tile_mask) continue;
                for (int py = overlap_y1; py < overlap_y2 && !tile_collided; py++) {
                    for (int px = overlap_x1; px < overlap_x2; px++) {
                        int mask_x = px - tile_px_x;
                        int mask_y = py - tile_px_y;
                        if (collision_type == 3) {
                            apply_transform_16(level->transform[tile_id], &mask_x, &mask_y);
                        }
                        if (mask_x < 0 || mask_y < 0 || mask_x >= level->tile_w || mask_y >= level->tile_h) continue;
                        // Do not "fix" this to [mask_x][mask_y]: Java collision reads
                        // the inline mask with transposed indices, and shipped /res/tf
                        // masks are authored for that runtime behavior.
                        if (!tile_mask[mask_y][mask_x]) continue;
                        if (player_mask) {
                            int local_x = px - rect_x;
                            int local_y = py - rect_y;
                            if (!player_mask[local_y * rect_w + local_x]) continue;
                        }
                        tile_collided = true;
                        break;
                    }
                }
            }

            if (tile_collided) {
                collided = true;
                if (hit_count < max_hits) {
                    out_hit_x[hit_count] = tx;
                    out_hit_y[hit_count] = ty;
                    if (hit_count < 5) {
                        level->hit_cols[hit_count] = tx;
                        level->hit_rows[hit_count] = ty;
                    }
                    hit_count++;
                } else {
                    level->hits_overflow = true;
                    if (out_overflow) *out_overflow = true;
                    return true;
                }
            }
        }
    }

    if (level->objects.count > 0) {
        int rx1 = rect_x;
        int ry1 = rect_y;
        int rx2 = rect_x + rect_w;
        int ry2 = rect_y + rect_h;
        for (uint8_t i = 0; i < level->objects.count; i++) {
            int top = level->objects.f[i][0] * 16 + level->objects.ag[i][1];
            int left = level->objects.f[i][1] * 16 + level->objects.ag[i][0];
            int w = object_width(level, i, 0);
            int h = object_height(level, i, 0);
            if (rects_overlap(left, top, left + w, top + h, rx1, ry1, rx2, ry2)) {
                level->active_object_index = (int)i;
                if (out_active_object_index) *out_active_object_index = (int)i;
                // Java a(..., ..., ..., ..., true/false) treats object overlap as collision too.
                collided = true;
                break;
            }
        }
    }

    return collided;
}

bool level_test_collision(Level* level,
                          int rect_x, int rect_y, int rect_w, int rect_h,
                          const bool* player_mask) {
    int hx[1] = { -1 };
    int hy[1] = { -1 };
    bool overflow = false;
    int obj = -1;
    return level_test_collision_collect(level, rect_x, rect_y, rect_w, rect_h,
                                        player_mask, hx, hy, 1, &overflow, &obj);
}
