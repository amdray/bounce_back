#ifndef BG_LAYER_H
#define BG_LAYER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t render_type;
    uint8_t image_index;
    uint8_t transform;
    int32_t aux;
} BgTileMeta;

typedef struct {
    int tile_w;
    int tile_h;
    int tile_count;
    int width_tiles;
    int height_tiles;
    bool clamp_x;
    bool clamp_y;
    uint8_t tile_id_mask;
    uint8_t tile_flag_mask;

    BgTileMeta* meta;      // [tile_count]
    uint8_t* tile_map;     // [height_tiles * width_tiles]

    SDL_Texture** textures;
    int texture_count;
} BgLayer;

BgLayer* bg_layer_load(SDL_Renderer* renderer,
                       const char* bg_path,
                       const char* ib0_path,
                       const char* ib_theme_path);
void bg_layer_free(BgLayer* bg);

void bg_layer_draw(const BgLayer* bg,
                   SDL_Renderer* renderer,
                   int camera_x,
                   int camera_y,
                   int screen_w,
                   int screen_h);

#endif // BG_LAYER_H
