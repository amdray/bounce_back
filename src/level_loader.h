/**
 * Level loader for Bounce Back (/res/lf)
 *
 * Parses a single level's metadata + tileMap from the J2ME container resource.
 *
 * Spec: docs/STEP_02_TILE_ENGINE.md
 * Reference: docs/DEOBFUSCATION.md ("Формат данных уровня (/res/lf)")
 */

#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <stdbool.h>
#include <stdint.h>

// Runtime objects from level metadata (h.java:127/133/203/206/207)
typedef struct {
    uint8_t count;     // this.j
    uint8_t* ao;       // this.ao[]
    int (*f)[2];       // this.f[][2]   (tile bounds start)
    int (*k)[2];       // this.k[][2]   (tile bounds end)
    int (*ag)[2];      // this.ag[][2]  (pixel offsets)
    int8_t (*s)[2];    // this.s[][2]   (speeds)
} LevelObjects;

typedef struct {
    uint8_t theme_id;
    uint8_t spawn_x;
    uint8_t spawn_y;
    uint8_t ball_type;
    uint8_t exit_x;
    uint8_t exit_y;
    int hoops_remaining;   // E.W analog - rings left to collect
    int hoops_total;       // Initial ring count on level load
    uint8_t width;
    uint8_t height;
    uint8_t* tile_map; // g.R flattened [height * width], row-major

    // g.java runtime analogs needed by player d()
    uint8_t tile_id_mask;   // g.z
    uint8_t tile_flag_mask; // g.q
    uint8_t tile_w;         // g.f (normalized: 12->16)
    uint8_t tile_h;         // g.A (normalized: 12->16)
    uint8_t tile_count;     // g.h
    uint8_t* collision_type;// g.l[tileId]
    uint8_t* transform;     // g.b[tileId]
    bool** masks;           // g.s[tileId] -> flattened mask storage
    int hit_cols[5];        // g.Y
    int hit_rows[5];        // g.P
    bool hits_overflow;
    int active_object_index; // analog this.k binding in player flow

    LevelObjects objects;   // E.f/ag/ao/s analog
} Level;

Level* level_load(const char* lf_path, int level_index);
void level_free(Level* level);
uint8_t level_get_tile(const Level* level, int tile_x, int tile_y);
void level_set_tile(Level* level, int tile_x, int tile_y, uint8_t tile_byte);
void level_objects_tick(Level* level,
                        int player_left,
                        int player_top,
                        int player_right,
                        int player_bottom,
                        bool player_is_stone);

bool level_test_collision_collect(Level* level,
                                  int rect_x, int rect_y, int rect_w, int rect_h,
                                  const bool* player_mask,
                                  int* out_hit_x, int* out_hit_y, int max_hits,
                                  bool* out_overflow,
                                  int* out_active_object_index);
bool level_test_collision(Level* level,
                          int rect_x, int rect_y, int rect_w, int rect_h,
                          const bool* player_mask);

#endif // LEVEL_LOADER_H
